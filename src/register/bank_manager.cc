#include <string>
#include <iostream>
#include <algorithm>
// #include <magic.h>

#include "bank_payment_list_impl.h"
#include "bank_statement_list_impl.h"
#include "bank_common.h"
#include "bank_manager.h"
#include "invoice_manager.h"
#include "registrar.h"
#include "types/stringify.h"


// std::string magic_string_to_mime_type(const std::string &_magic_str)
// {
//     std::string::size_type l = _magic_str.length();
//     std::string::size_type scolon = _magic_str.find_first_of(";");
//     if (scolon >= l) {
//         return _magic_str;
//     }
//     else {
//         return std::string(_magic_str, 0, scolon);
//     }
// }

namespace Register {
namespace Banking {


class ManagerImpl : virtual public Manager
{
private:
    File::Manager *file_manager_;


    /**
     *  core method for pairing payment and statement - no error checking
     *  just statement update
     */
    void _pairPaymentWithStatement(const Database::ID &payment,
                                   const Database::ID &statement)
    {
        if (payment == 0) {
            throw std::runtime_error("payment id not valid");
        }

        Database::Query uquery;
        uquery.buffer() << "UPDATE bank_payment SET statement_id = ";
        if (statement == 0) {
            uquery.buffer() << "NULL";
        }
        else {
            uquery.buffer() << Database::Value(statement);
        }
        uquery.buffer() << " WHERE id = " << Database::Value(payment);

        Database::Connection conn = Database::Manager::acquire();
        Database::Result uresult = conn.exec(uquery);

        LOGGER(PACKAGE).info(
                boost::format("pair payment id=%1% with statement id=%2%: "
                              "success!") % payment % statement);
    }

    unsigned long long getZoneByAccountId(const unsigned long long &_account_id) {
        Database::Query query;
        query.buffer() << "SELECT zone FROM bank_account WHERE "
                       << "id = " << Database::Value(_account_id);

        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec(query);

        if (result.size() != 0) {
            return static_cast<unsigned long long>(result[0][0]);
        }
        else {
            return 0;
        }
    }

    void processPayment(PaymentImpl *_payment,
                        unsigned long long _registrar_id = 0)
    {
        Logging::Context ctx("payment processing");
        try {
            /* is payment in db? */
            if (_payment->getId() == Database::ID(0)) {
                throw std::runtime_error("cannot process payment which was not"
                        "saved");
            }

            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction transaction(conn);

            _payment->reload();
            if (_payment->getInvoiceId() != Database::ID(0)) {
                return;
            }
            if (_payment->getPrice() <= Database::Money(0)) {
                return;
            }

            /* process only payemts from specific accounts
             * XXX: this should be in database not hardcoded */
            std::vector<std::string> allowed;
            allowed.push_back("188208275/0300");
            allowed.push_back("756/2400");
            allowed.push_back("210345314/0300");
            allowed.push_back("617/2400");
            allowed.push_back("756/5500");
            allowed.push_back("617/5500");

            std::stringstream account_query;
            account_query << "SELECT account_number || '/' || bank_code FROM "
                          << "bank_account WHERE id = "
                          << Database::Value(_payment->getAccountId());
            Database::Result result = conn.exec(account_query.str());
            if (result.size() != 1) {
                throw std::runtime_error("oops! payment has no record in "
                        "bank_account table");
            }
            std::string test = result[0][0];

            std::vector<std::string>::const_iterator it = std::find(allowed.begin(), allowed.end(), test);
            if (it == allowed.end()) {
                LOGGER(PACKAGE).debug(boost::format(
                            "account %1% is excluded from processing "
                            "-> processing canceled")
                            % test);
                return;
            }

            /* we process only payment with code = 1 (normal transaction)
             * and status = 1 (realized transfer) and type = 1 (not assigned) */
            if (_payment->getStatus() != 1 || _payment->getCode() != 1
                    || _payment->getType() != 1) {
                LOGGER(PACKAGE).info(boost::format(
                            "payment id=%1% not eligible -- status: %2% != 1 "
                            "code: %3% != 1 type: %4% != 1 => processing canceled")
                            % _payment->getId()
                            % _payment->getStatus()
                            % _payment->getCode()
                            % _payment->getType());
                return;
            }

            /* automatic pair payment with registrar */
            if (_registrar_id == 0) {
                Register::Registrar::Manager::AutoPtr rmanager(Register::Registrar::Manager::create(0));
                _registrar_id = rmanager->getRegistrarByPayment(_payment->getVarSymb(),
                                                                _payment->getAccountMemo());
                /* did we find suitable registrar? */
                if (_registrar_id == 0) {
                    LOGGER(PACKAGE).warning(boost::format(
                                "couldn't find suitable registrar for payment id=%1% "
                                "=> processing canceled") % _payment->getId());
                    return;
                }
            }

            Database::Money price = _payment->getPrice();
            Database::Date account_date = _payment->getAccountDate();
            unsigned long long zone_id = getZoneByAccountId(_payment->getAccountId());

            std::auto_ptr<Register::Invoicing::Manager>
                    invoice_manager(Register::Invoicing::Manager::create());

            int invoice_id = invoice_manager->createDepositInvoice(
                    account_date, (int)zone_id, (int)_registrar_id, (long)price);
            if (invoice_id > 0) {
                _payment->setInvoiceId(invoice_id);
                _payment->setType(2);
                _payment->save();
                LOGGER(PACKAGE).info(boost::format(
                            "payment paired with registrar (id=%1%) "
                            "=> deposit invoice created (id=%2% price=%3%)")
                            % _registrar_id
                            % invoice_id % stringify(price));
            }

            transaction.commit();
        }
        catch (std::exception &ex) {
            throw std::runtime_error(str(boost::format(
                            "processing payment failed: %1%") % ex.what()));
        }
    }


public:
    ManagerImpl(File::Manager *_file_manager)
              : file_manager_(_file_manager)
    {
    }

    StatementList* createStatementList() const
    {
        return new StatementListImpl();
    }

    PaymentList* createPaymentList() const
    {
        return new PaymentListImpl();
    }

    void importStatementXml(std::istream &_in,
                            const std::string &_file_path,
                            const std::string &_file_mime,
                            const bool &_generate_invoices = false)
         throw (std::runtime_error)
    {
        TRACE("[CALL] Register::Banking::Manager::importStatementXml(...)");
        Logging::Context ctx("bank xml import");
        try {
            /* load stream to string */
            _in.seekg(0, std::ios::beg);
            std::ostringstream stream;
            stream << _in.rdbuf();
            std::string xml = stream.str();

            /* parse */
            XMLparser parser;
            if (!parser.parse(xml)) {
                throw std::runtime_error("parser error");
            }

            XMLnode root = parser.getRootNode();
            if (root.getName().compare(STATEMENTS_ROOT) != 0) {
                throw std::runtime_error(str(boost::format(
                            "root xml element name is not `<%1%>'")
                            % STATEMENTS_ROOT));
            }

            /* start transaction for saving */
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            for (int i = 0; i < root.getChildrenSize(); i++) {
                /* xml to statement */
                XMLnode snode = root.getChild(i);
                StatementImplPtr statement(parse_xml_statement_part(snode));

                /* check if statement is valid or was processed in past */
                bool statement_conflict = false;
                bool statement_valid = statement->isValid();
                if (statement_valid) {
                    /* error when have valid statement and no original file */
                    if (_file_path.empty() || _file_mime.empty()) {
                        throw std::runtime_error("importing statement "
                                "without original file path and file mime type set");
                    }

                    Database::ID conflict_sid(0);
                    if ((conflict_sid = statement->getConflictId()) == 0) {
                        statement_conflict = false;
                        statement->save();
                        LOGGER(PACKAGE).info(boost::format(
                                "new statement imported (id=%1% account_id=%2% "
                                "number=%3% date=%4%)")
                                % statement->getId()
                                % statement->getAccountId()
                                % statement->getNum()
                                % statement->getCreateDate());
                    }
                    else {
                        statement_conflict = true;
                        statement->setId(conflict_sid);
                        LOGGER(PACKAGE).info(boost::format(
                                "conflict statement found (id=%1% account_id=%2% "
                                "number=%3%) -- checking payments")
                                % statement->getId()
                                % statement->getAccountId()
                                % statement->getNum());
                    }
                }
                else {
                    statement->setId(0);
                    LOGGER(PACKAGE).info("no statement -- importing only payments");

                }
                unsigned long long sid = statement->getId();

                XMLnode pnodes = snode.getChild(STATEMENT_ITEMS);
                for (int n = 0; n < pnodes.getChildrenSize(); n++) {
                    /* xml to payment */
                    XMLnode pnode = pnodes.getChild(n);
                    PaymentImplPtr payment(parse_xml_payment_part(pnode));
                    payment->setAccountId(statement->getAccountId());
                    if (sid != 0) {
                        payment->setStatementId(sid);
                    }

                    Database::ID conflict_pid(0);
                    if ((conflict_pid = payment->getConflictId()) == 0) {
                        payment->save();
                        LOGGER(PACKAGE).info(boost::format(
                                "payment imported (id=%1% account=%2%/%3% "
                                "evid=%4% price=%5% account_date=%6%) account_id=%7%")
                                % payment->getId()
                                % payment->getAccountNumber()
                                % payment->getBankCode()
                                % payment->getAccountEvid()
                                % payment->getPrice()
                                % payment->getAccountDate()
                                % payment->getAccountId());

                        /* payment processing */
                        processPayment(payment.get());
                    }
                    else {
                        /* load conflict payment */
                        PaymentImplPtr cpayment(new PaymentImpl());
                        cpayment->setId(conflict_pid);
                        cpayment->reload();
                        /* compare major attributes which should never change */
                        if (payment->getAccountDate() != cpayment->getAccountDate()
                                || payment->getAccountNumber() != cpayment->getAccountNumber()
                                || payment->getBankCode() != cpayment->getBankCode()
                                || payment->getCode() != cpayment->getCode()
                                || payment->getKonstSym() != cpayment->getKonstSym()
                                || payment->getVarSymb() != cpayment->getVarSymb()
                                || payment->getSpecSymb() != cpayment->getSpecSymb()
                                || payment->getPrice() != cpayment->getPrice()
                                || payment->getAccountDate() != cpayment->getAccountDate()
                                || payment->getAccountMemo() != cpayment->getAccountMemo()) {

                            LOGGER(PACKAGE).debug(boost::format("imported payment: %1%")
                                                                % payment->toString());
                            LOGGER(PACKAGE).debug(boost::format("conflict payment: %1%")
                                                                % cpayment->toString());

                            throw std::runtime_error(str(boost::format(
                                            "oops! found conflict payment with "
                                            "INCONSISTENT DATA (id=%1% account_evid=%2%) "
                                            "importing file=%3%")
                                            % cpayment->getId()
                                            % cpayment->getAccountEvid()
                                            % _file_path));
                        }
                        /* compare changable attributes for futher processing */
                        else if (payment->getStatus() != cpayment->getStatus()) {
                            LOGGER(PACKAGE).info(boost::format(
                                    "already imported payment -- status changed "
                                    "%1% => %2%")
                                    % cpayment->getStatus()
                                    % payment->getStatus());

                            // payment->setId(conflict_pid);
                            // payment->save();
                            cpayment->setStatus(payment->getStatus());
                            cpayment->save();
                            processPayment(cpayment.get());
                        }

                        /* there we should have already imported payment */
                        LOGGER(PACKAGE).info(boost::format(
                                "conflict payment found "
                                "(id=%1% account=%2%/%3% evid=%4%)")
                                % cpayment->getId()
                                % cpayment->getAccountNumber()
                                % cpayment->getBankCode()
                                % cpayment->getAccountEvid());

                        if (payment->getStatementId() != cpayment->getStatementId()
                                && cpayment->getStatementId() == 0) {

                            if (statement_valid && !statement_conflict) {
                                LOGGER(PACKAGE).info(boost::format(
                                            "conflict payment should be paired with imported "
                                            "statement (payment=%1% statement=%2%)")
                                            % cpayment->getId()
                                            % statement->getId());
                                _pairPaymentWithStatement(cpayment->getId(), statement->getId());
                            }
                       }
                       else {
                           LOGGER(PACKAGE).info(boost::format(
                                       "conflict payment is already paired with this "
                                       "statement (payment=%1% statement=%2%)")
                                       % cpayment->getId()
                                       % statement->getId());
                       }
                    }
                }

                /* upload file via file manager and update statement */
                if (file_manager_ && statement_valid && !statement_conflict) {
                    // /* get mime type */
                    // magic_t magic = magic_open(MAGIC_MIME);
                    // magic_load(magic, 0);
                    // std::string magic_str = magic_file(magic, _file_path.c_str());
                    // std::string mime_type = magic_string_to_mime_type(magic_str);

                    unsigned long long id = file_manager_->upload(_file_path, _file_mime, 4);
                    statement->setFileId(id);
                    statement->save();
                    LOGGER(PACKAGE).info(boost::format(
                                "statement file (id=%1%) succesfully uploaded") % id);
                }

            }


            tx.commit();
        }
        catch (std::exception &ex) {
            throw std::runtime_error(str(boost::format(
                            "bank xml import: %1%") % ex.what()));
        }
        catch (...) {
            throw std::runtime_error("bank xml import: an error occured");
        }
    }

    void addBankAccount(const std::string &_account_number,
                            const std::string &_bank_code,
                            const std::string &_zone,
                            const std::string &_account_name)
        throw (std::runtime_error)
    {
        TRACE("[CALL] Register::Banking::Manager::insertBankAccount(zone_fqdn, ...)");

        try {
            Database::Connection conn = Database::Manager::acquire();

            Database::ID zone_id = 0;
            if (!_zone.empty()) {
                Database::Query query;
                query.buffer() << "SELECT id FROM zone WHERE fqdn=" << Database::Value(_zone);

                Database::Result result = conn.exec(query);
                if (result.size() == 0) {
                    throw std::runtime_error(str(boost::format(
                                        "zone '%1%' not found")
                                        % _zone));
                }
                 zone_id = result[0][0];
            }

            if (!_account_number.empty() && !_bank_code.empty()) {
                Database::Query query;
                query.buffer() << "SELECT id FROM bank_account WHERE "
                               << "account_number = " << Database::Value(_account_number)
                               << "AND bank_code = " << Database::Value(_bank_code);
                Database::Result result = conn.exec(query);
                if (result.size() != 0) {
                    throw std::runtime_error(str(boost::format(
                                    "account '%1%/%2%' already exist")
                                    % _account_number % _bank_code));
                }
                /* insert new account */
                Database::InsertQuery iquery("bank_account");
                iquery.add("zone", zone_id);
                iquery.add("account_number", _account_number);
                iquery.add("account_name", _account_name);
                iquery.add("bank_code", _bank_code);
                conn.exec(iquery);
            }
            else {
                throw std::runtime_error("account number and bank_code "
                        "not given");
            }
        }
        catch (std::exception &ex) {
            throw std::runtime_error(str(boost::format(
                            "add bank account: %1%") % ex.what()));
        }
        catch (...) {
            throw std::runtime_error("add bank account: an error occured");
        }
    }

    bool pairPaymentWithStatement(const Database::ID &payment,
                                  const Database::ID &statement,
                                  bool force = false)
    {
        TRACE("[CALL] Register::Banking::Manager::pairPaymentWithStatement(...)");
        if (payment == 0) {
            throw std::runtime_error("payment id not valid");
        }

        Database::Connection conn = Database::Manager::acquire();
        try {
            Database::Query pquery;
            pquery.buffer() << "SELECT id, statement_id FROM bank_payment WHERE id = "
                            << Database::Value(payment);

            Database::Result presult = conn.exec(pquery);
            if (presult.size() == 0) {
                throw std::runtime_error("payment doesn't exist");
            }

            if (statement > 0) {
                Database::Query squery;
                squery.buffer() << "SELECT id FROM bank_statement WHERE id = "
                                << Database::Value(statement);

                Database::Result sresult = conn.exec(squery);
                if (sresult.size() == 0) {
                    throw std::runtime_error("statement doesn't exist");
                }
            }

            unsigned long long payment_sid = presult[0][1];
            if (payment_sid > 0) {
                if (force) {
                    LOGGER(PACKAGE).info(
                            boost::format("Pair payment id=%1% with statement id=%2%: "
                                          "payment is listed on statement id=%3% (FORCING)")
                                           % payment % statement % payment_sid);
                }
                else {
                    throw std::runtime_error(str(boost::format(
                                    "payment is listed on statement id=%3% "
                                    "(try run with `force' parameter)")
                                    % payment % statement % payment_sid));
                }
            }

            /* update payment */
            _pairPaymentWithStatement(payment, statement);
            return true;
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).error(
                    boost::format("Cannot pair payment id=%1% with statement id=%2%: %3%")
                                  % payment % statement % ex.what());
            return false;
         }
        catch (...) {
            LOGGER(PACKAGE).error(
                    boost::format("Cannot pair payment id=%1% with statement id=%2%: "
                                  "exception caught") % payment % statement);
            return false;
        }
    }

    bool setInvoiceToStatementItem(
            Database::ID statementId,
            Database::ID invoiceId)
    {
        TRACE("[CALL] Register::Invoicing::Manager::setInvoiceToStatementItem()");
        Database::Query update;
        update.buffer()
            << "UPDATE bank_payment SET invoice_id="
            << Database::Value(invoiceId)
            << " WHERE id=" << Database::Value(statementId);
        Database::Connection conn = Database::Manager::acquire();
        try {
            conn.exec(update);
        } catch (...) {
            return false;
        }
        return true;
    }

    virtual bool pairPaymentWithRegistrar(
                const Database::ID &paymentId,
                const std::string &registrarHandle) {

        TRACE("[CALL] Register::Invoicing::Manager::manualCreateInvoice(Database::ID, std::string)");
        Database::Query query;
        query.buffer()
            << "SELECT id FROM registrar WHERE handle="
            << Database::Value(registrarHandle);
        Database::Connection conn = Database::Manager::acquire();
        Database::ID registrarId;
        try {
            Database::Result res = conn.exec(query);
            if(res.size() == 0) {
                    LOGGER(PACKAGE).error(boost::format(
                    "Registrar with handle '%1%' not found in database.") %
                            registrarHandle);
                    return false;
            }
            registrarId = res[0][0];

            PaymentImpl pi;
            pi.setId(paymentId);
            pi.reload();
            processPayment(&pi, registrarId);

        } catch (std::exception &e) {
            LOGGER(PACKAGE).error(boost::format("An error has occured: %1% ") % e.what());
            return false;
        }

        return true;
    }

    virtual void setPaymentType(
            const Database::ID &payment_id,
            const int &type)
    {
        Logging::Context ctx("set payment type");
        try {
            if (type < 1 || type > 6) {
                throw std::runtime_error("parameter error "
                        "(type should be in interval <1, 6>)");
            }

            if (payment_id == 0) {
                throw std::runtime_error("parameter error "
                        "invalid payment id");
            }

            PaymentImpl payment;
            payment.setId(payment_id);
            payment.reload();

            int old_type = payment.getType();

            payment.setType(type);
            payment.save();

            LOGGER(PACKAGE).info(boost::format(
                        "successfully changed payment id=%1% type "
                        "%2% => %3%") % payment_id % old_type % type);
        }
        catch (std::exception &ex) {
            throw std::runtime_error(str(boost::format(
                            "set payment type: %1%") % ex.what()));
        }
        catch (NOT_FOUND) {
            throw std::runtime_error(str(boost::format("set payment type: "
                            "payment id=%1% not found") % payment_id));
        }
        catch (...) {
            throw std::runtime_error("set payment type: unknown error occured");
        }
    }
};

Manager* Manager::create(File::Manager *_file_manager)
{
    return new ManagerImpl(_file_manager);
}

} // namespace Banking
} // namespace Register

