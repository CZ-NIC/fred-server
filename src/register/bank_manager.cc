#include <string>
#include <iostream>
#include <algorithm>
// #include <magic.h>

#include "bank_payment_list_impl.h"
#include "bank_statement_list_impl.h"
#include "bank_manager.h"
#include "bank_common.h"
#include "invoice_manager.h"
#include "types/stringify.h"


std::string magic_string_to_mime_type(const std::string &_magic_str)
{
    std::string::size_type l = _magic_str.length();
    std::string::size_type scolon = _magic_str.find_first_of(";");
    if (scolon >= l) {
        return _magic_str;
    }
    else {
        return std::string(_magic_str, 0, scolon);
    }
}

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
        uquery.buffer() << "UPDATE bank_item SET statement_id = ";
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
                boost::format("Pair payment id=%1% with statement id=%2%: "
                              "success!") % payment % statement);
    }


    void processPayment(PaymentImpl *_payment)
    {
        if (_payment->getId() == Database::ID(0)) {
            throw std::runtime_error("cannot process payment which was is "
                    "saved");
        }

        try {
            Database::Query query;
            query.buffer()
                << "SELECT bi.id, ba.zone, rr.id, rr.handle, bi.price, bi.account_date"
                << " FROM bank_item bi"
                << " JOIN bank_account ba ON bi.account_id=ba.id"
                << " JOIN registrar rr ON bi.varsymb=rr.varsymb"
                << " OR (length(trim(rr.regex)) > 0 and bi.account_memo ~* trim(rr.regex))"
                << " WHERE bi.invoice_id IS NULL AND bi.code=2 AND bi.type=1"
                << " AND bi.id = " << Database::Value(_payment->getId());

            Database::Connection conn = Database::Manager::acquire();
            Database::Result result = conn.exec(query);
            if (result.size() != 1) {
                /* already processed of not suitable registrar found
                 * or more than one registrar found */
                return;
            }

            unsigned long long zone_id = result[0][1];
            unsigned long long registrar_id = result[0][2];
            std::string registrar_handle = result[0][3];
            Database::Money price;
            price.format(result[0][4]);
            Database::Date account_date = result[0][5];

            std::auto_ptr<Register::Invoicing::Manager>
                    invoice_manager(Register::Invoicing::Manager::create());

            int invoice_id = invoice_manager->createDepositInvoice(
                    account_date, (int)zone_id, (int)registrar_id, (long)price);
            if (invoice_id > 0) {
                _payment->setInvoiceId(invoice_id);
                _payment->save();
                LOGGER(PACKAGE).info(boost::format(
                            "Bank payment paired with registrar %1% (id=%2%) "
                            "created deposit invoice (id=%3% price=%4%)")
                            % registrar_handle % registrar_id
                            % invoice_id % stringify(price));
            }
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

    bool importStatementXml(std::istream &_in,
                            const std::string &_file_path,
                            const std::string &_file_mime,
                            const bool &_generate_invoices = false)
    {
        TRACE("[CALL] Register::Banking::Manager::importStatementXml(...)");
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
                            "root element name is not `<%1%>'")
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
                    Database::ID conflict_sid(0);
                    if ((conflict_sid = statement->getConflictId()) == 0) {
                        statement_conflict = false;
                        statement->save();
                        LOGGER(PACKAGE).info(boost::format(
                                "Bank statement XML import: "
                                "new statement imported (id=%1% account_id=%2% "
                                "number=%3%)")
                                % statement->getId()
                                % statement->getAccountId()
                                % statement->getNum());
                    }
                    else {
                        statement_conflict = true;
                        statement->setId(conflict_sid);
                        LOGGER(PACKAGE).info(boost::format(
                                "Bank statement XML import: "
                                "conflict statement found (id=%1% account_id=%2% "
                                "number=%3%) -- checking payments")
                                % statement->getId()
                                % statement->getAccountId()
                                % statement->getNum());
                    }
                }
                else {
                    statement->setId(0);
                    LOGGER(PACKAGE).info("Bank statement XML import: "
                            "no statement -- importing only payments");

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
                                "Bank statement XML import: "
                                "payment imported (id=%1% account=%2%/%3% "
                                "evid=%4%)")
                                % payment->getId()
                                % payment->getAccountNumber()
                                % payment->getBankCode()
                                % payment->getAccountEvid());

                        /* payment processing */
                        processPayment(payment.get());
                    }
                    else {
                        payment->setId(conflict_pid);
                        LOGGER(PACKAGE).info(boost::format(
                                "Bank statement XML import: "
                                "conflict payment found "
                                "(id=%1% account=%2%/%3% evid=%4%)")
                                % payment->getId()
                                % payment->getAccountNumber()
                                % payment->getBankCode()
                                % payment->getAccountEvid());
                        /* XXX don't process statements so far
                        if (statement_valid) {
                            if (!statement_conflict) {
                                // payment exists and statement was saved
                                // need to pair payment with statement
                                _pairPaymentWithStatement(payment->getId(), statement->getId());
                            }
                            else {
                                // payment exists and statement was existing also
                                // is it paired already?
                            }
                        }
                        */
                    }
                }

                /* upload file via file manager and update statement */
                if (file_manager_ && statement_valid && !statement_conflict) {
                    /* get mime type 
                    magic_t magic = magic_open(MAGIC_MIME);
                    magic_load(magic, 0);
                    std::string magic_str = magic_file(magic, _file_path.c_str());
                    std::string mime_type = magic_string_to_mime_type(magic_str);
                    */

                    unsigned long long id = file_manager_->upload(_file_path, _file_mime, 4);
                    statement->setFileId(id);
                    statement->save();
                    LOGGER(PACKAGE).info(boost::format("Bank statement XML import: "
                                         "statement file succesfully uploaded to "
                                         "server (id=%1%)") % id);
                }

            }


            tx.commit();
            return true;
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("Bank statement XML import: %1%")
                                    % ex.what());
            return false;
        }
        catch (...) {
            LOGGER(PACKAGE).error("Bank statement XML import: an error occured");
            return false;
        }
    }

    bool insertBankAccount(const std::string &zone,
                           const std::string &account_number,
                           const std::string &account_name,
                           const std::string &bank_code)
    {
        TRACE("[CALL] Register::Banking::Manager::insertBankAccount(zone_fqdn, ...)");
        Database::Query query;
        query.buffer()
            << "SELECT id FROM zone WHERE fqdn=" << Database::Value(zone);
        try {
            Database::Connection conn = Database::Manager::acquire();
            Database::Result res = conn.exec(query);
            if (res.size() == 0) {
                LOGGER(PACKAGE).error(boost::format("Unknown zone (%1%)") % zone);
                return false;
            }
            Database::ID zoneId = res[0][0];
            return insertBankAccount(zoneId, account_number, account_name, bank_code);
        } catch (...) {
            LOGGER(PACKAGE).error("An exception was catched");
            return false;
        }
        return false;
    }

    bool insertBankAccount(const Database::ID &zone,
                           const std::string &account_number,
                           const std::string &account_name,
                           const std::string &bank_code)
    {
        TRACE("[CALL] Register::Banking::Manager::insertBankAccount(zone_id, ...)");
        Database::InsertQuery insertAccount("bank_account");
        insertAccount.add("zone", zone);
        insertAccount.add("account_number", account_number);
        insertAccount.add("account_name", account_name);
        insertAccount.add("bank_code", bank_code);
        Database::Connection conn = Database::Manager::acquire();
        try {
            conn.exec(insertAccount);
        } catch (...) {
            LOGGER(PACKAGE).error("Cannot insert new account into the ``bank_account'' table");
            return false;
        }
        return true;
    }

    bool moveItemToPayment(
        const Database::ID &payment,
        const Database::ID &statement,
        bool force = false)
    {
        return pairPaymentWithStatement(payment, statement, force);
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
            pquery.buffer() << "SELECT id, statement_id FROM bank_item WHERE id = "
                            << Database::Value(payment);

            Database::Result presult = conn.exec(pquery);
            if (presult.size() == 0) {
                throw std::runtime_error("payment doesn't exist");
            }

            if (statement > 0) {
                Database::Query squery;
                squery.buffer() << "SELECT id FROM bank_head WHERE id = "
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
            << "UPDATE bank_item SET invoice_id="
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

    bool processPayments(bool report) {
        TRACE("[CALL] Register::Invoicing::Manager::pairInvoices()");

        std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create());

        Database::Query query;
        query.buffer()
            << "SELECT bi.id, ba.zone, rr.id, bi.price, bi.account_date"
            << " FROM bank_item bi"
    //        << " JOIN bank_head bh ON bi.statement_id=bh.id"
            << " JOIN bank_account ba ON bi.account_id=ba.id"
            << " JOIN registrar rr ON bi.varsymb=rr.varsymb"
            << " OR (length(trim(rr.regex)) > 0 and bi.account_memo ~* trim(rr.regex))"
            << " WHERE bi.invoice_id IS NULL AND bi.code=2 AND bi.type=1;";
        Database::Connection conn = Database::Manager::acquire();
        // Database::Transaction transaction(conn);
        Database::Result res = conn.exec(query);
        Database::Result::Iterator it = res.begin();
        for (; it != res.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            Database::ID statementId = *(col);
            int zoneId = *(++col);
            int registrarId = *(++col);
            long price = *(++col);
            Database::Date date = *(++col);
                    
            int invoiceId = invMan->createDepositInvoice(date, (int)zoneId, (int)registrarId, (long)price);
            if (invoiceId == 0) {
                LOGGER(PACKAGE).warning("Failed to save credit invoice");
                continue;
            }
            int retval = setInvoiceToStatementItem(statementId, invoiceId);
            if (!retval) {
                LOGGER(PACKAGE).error("Unable to update bank_item table");
                return false;
            }
        }

        return true;
    }

    bool
    hasStatementAnInvoice(const Database::ID &statementId)
    {
        TRACE("[CALL] Register::Invoicing::Manager::hasStatementAnInvoice()");
        Database::Query query;
        query.buffer()
            << "SELECT invoice_id FROM bank_item WHERE id="
            << Database::Value(statementId);
        Database::Connection conn = Database::Manager::acquire();
        try {
            Database::Result res = conn.exec(query);
            if(res.size() == 0) {
                    LOGGER(PACKAGE).error(boost::format(
                        "Payment (id: %1%) not found in database")
                            % statementId);
                    return true;        
            }
            Database::ID invoiceId = res[0][0];
            if (invoiceId != Database::ID()) {
                LOGGER(PACKAGE).error(boost::format(
                            "This payment (id: %1%) already has invoice (id: %2%)")
                        % statementId % invoiceId);
                return true;
            }
        } catch (...) {
            LOGGER(PACKAGE).error("An error has occured");
            return true;
        }
        return false;
    }

  // version using dbsql in invoicing - creates its own DB connection, so it shouldn't be called inside of a transaction
  // manual creation of deposit (advance) invoice
    virtual bool manualCreateInvoice(
            const Database::ID &paymentId,
            const Database::ID &registrar) {    

    std::auto_ptr<Register::Invoicing::Manager>
    invMan(Register::Invoicing::Manager::create());
      
    TRACE("[CALL] Register::Invoicing::Manager::manualCreateInvoice(Database::ID, Database::ID)");

    if (hasStatementAnInvoice(paymentId)) {
        LOGGER(PACKAGE).error(boost::format("Payment with ID %1% already has an invoice assigned.") % paymentId);
        return false;
    }

    Database::Query query;
    query.buffer()
        << "SELECT ba.zone, bi.price, bi.account_date"
        << " FROM bank_item bi"
//        << " JOIN bank_head bh ON bi.statement_id=bh.id"
        << " JOIN bank_account ba ON bi.account_id=ba.id"
        << " WHERE bi.id="
        << Database::ID(paymentId);
    Database::ID zoneId;
    Database::Money price;
    Database::Date date;
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction transaction(conn);
    try {
        Database::Result res = transaction.exec(query);
        if (res.size() == 0) {
            LOGGER(PACKAGE).error("No result");
            return false;
        }
        zoneId = res[0][0];
        price = res[0][1];
        date = res[0][2];
    } catch (...) {
        LOGGER(PACKAGE).error("An error has occured");
        return false;
    }

    // if the call failed, it logged an error message
    return (0 != invMan->createDepositInvoice(date, (int)zoneId, (int)registrar, (long)price)); 
  };

  virtual bool manualCreateInvoice(
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
    } catch (...) {
        LOGGER(PACKAGE).error("An error has occured");
        return false;
    }
    return manualCreateInvoice(paymentId, registrarId);
  }

};

Manager* Manager::create(File::Manager *_file_manager)
{
    return new ManagerImpl(_file_manager);
}

} // namespace Banking
} // namespace Register

