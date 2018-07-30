#include <string>
#include <iostream>
#include <algorithm>
// #include <magic.h>

#include "src/libfred/banking/bank_statement_impl.hh"
#include "src/libfred/banking/bank_payment_list_impl.hh"
#include "src/libfred/banking/bank_statement_list_impl.hh"
#include "src/libfred/banking/bank_common.hh"
#include "src/libfred/banking/bank_manager.hh"
#include "src/libfred/credit.hh"
#include "src/libfred/invoicing/invoice.hh"
#include "src/libfred/registrar.hh"
#include "src/util/types/stringify.hh"
#include "src/util/types/money.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <utility>


namespace LibFred {
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

    void pay_invoice(unsigned long long registrar_id
            , unsigned long long zone_id
            , unsigned long long bank_payment_id
            , Money price
            , unsigned long long invoice_id)
    {
        Logging::Context ctx("pay_invoice");
        Database::Connection conn = Database::Manager::acquire();

        //add credit
        unsigned long long registrar_credit_transaction_id =
        LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, price, invoice_id);

        //insert_bank_payment_registrar_credit_transaction_map
        conn.exec_params(
        "INSERT INTO bank_payment_registrar_credit_transaction_map "
                " (bank_payment_id, registrar_credit_transaction_id) "
                " VALUES ($1::bigint, $2::bigint) "
        ,Database::query_param_list(bank_payment_id)
        (registrar_credit_transaction_id));


    }

    Money processPayment(PaymentImpl *_payment,
                        unsigned long long _registrar_id = 0)
    {
        Logging::Context ctx("payment processing");
        try
        {
            /* is payment in db? */
            if (_payment->getId() == Database::ID(0))
            {
                throw std::runtime_error("cannot process payment which was not"
                        "saved");
            }
            _payment->reload();
            if (_payment->getPrice() <= Money("0")) return Money("0");

            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction transaction(conn);

            /* we process only payment with code = 1 (normal transaction)
             * and status = 1 (realized transfer) and type = 1 (not assigned) */
            if(!_payment->is_eligible_to_process())
            {
                LOGGER(PACKAGE).info(boost::format(
                            "payment id=%1% not eligible -- status: %2% != 1 "
                            "code: %3% != 1 type: %4% != 1 => processing canceled")
                            % _payment->getId()
                            % _payment->getStatus()
                            % _payment->getCode()
                            % _payment->getType());
                return Money("0");
            }

            DBSharedPtr nodb;
            /* automatic pair payment with registrar */
            LibFred::Registrar::Manager::AutoPtr rmanager(LibFred::Registrar::Manager::create(nodb));
            if (_registrar_id == 0) {
                _registrar_id = rmanager->getRegistrarByPayment(_payment->getVarSymb(),
                                                                _payment->getAccountMemo());
                /* did we find suitable registrar? */
                if (_registrar_id == 0) {
                    LOGGER(PACKAGE).warning(boost::format(
                                "couldn't find suitable registrar for payment id=%1% "
                                "=> processing canceled") % _payment->getId());
                    return Money("0");
                }
            }

            unsigned long long zone_id = getZoneByAccountId(_payment->getAccountId());


            std::unique_ptr<LibFred::Invoicing::Manager>
                    invoice_manager(LibFred::Invoicing::Manager::create());

            //find_unpaid_account_invoices

            std::vector<LibFred::Invoicing::unpaid_account_invoice> uai_vect
                = invoice_manager->find_unpaid_account_invoices(
                     _registrar_id, zone_id);

            Money payment_price_rest = _payment->getPrice();

            //for unpaid_account_invoices
            for(unsigned i = 0 ; i < uai_vect.size(); ++i)
            {
                unsigned long long unpaid_account_invoice_id = uai_vect[i].id;
                Money uaci_balance = uai_vect[i].balance;//w/o vat
                Decimal uaci_vat = uai_vect[i].vat;
                Money unpaid_price_with_vat = uaci_balance
                        + uaci_balance * uaci_vat / Decimal("100");

                //if payment is not big enough to pay this account invoice
                bool is_partial_payment = (payment_price_rest <= unpaid_price_with_vat);

                Money partial_price
                    = is_partial_payment
                        ? payment_price_rest : unpaid_price_with_vat;
                if(partial_price > Money("0"))//don't do zero transactions
                {
                    payment_price_rest -= partial_price;
                    if(is_partial_payment)
                    {
                        Money balance_change //is partial_price without vat
                            = invoice_manager->lower_account_invoice_balance_by_paid_amount(
                                partial_price, uaci_vat , unpaid_account_invoice_id);
                        pay_invoice(_registrar_id , zone_id, _payment->getId()
                            , balance_change, unpaid_account_invoice_id);
                    }
                    else
                    {
                        Money balance_change //is rest of invoice balance without vat
                            = invoice_manager->zero_account_invoice_balance(
                                unpaid_account_invoice_id);
                        pay_invoice(_registrar_id , zone_id, _payment->getId()
                            , balance_change, unpaid_account_invoice_id);
                    }
                }

            }

            /* zone access check */
            if (!rmanager->hasRegistrarZoneAccess(_registrar_id, zone_id)) {
                // no acces to zone and no debt payed - invalid payment
                if(uai_vect.size() == 0) {
                    LOGGER(PACKAGE).warning(boost::format(
                            "registrar (id=%1%) has not access to zone (id=%2%)"
                            " => processing canceled (payment id=%3%)")
                            % _registrar_id
                            % zone_id
                            % _payment->getId());
                    return Money("0");
                }

                // amount larger than registrar debt
                if(payment_price_rest > Money("0")) {
                    LOGGER(PACKAGE).warning(boost::format(
                            "registrar (id=%1%) who has no longer access to zone (id=%2%)"
                            " sent amount larger than debt (payment id=%3%) it will have to be resolved manually")
                            % _registrar_id
                            % zone_id
                            % _payment->getId());

                }
            }

            Money remaining_credit = Money("0");

            // create advance invoice for rest amount after paying possible debt (account invoice)
            if (payment_price_rest > Money("0"))
            {
                //Database::Date account_date = _payment->getAccountDate();

                boost::posix_time::ptime local_current_timestamp
                    = boost::posix_time::microsec_clock::local_time();


                boost::gregorian::date tax_date = local_current_timestamp.date();


                Money out_credit;

                unsigned long long advance_invoice_id
                    = invoice_manager->createDepositInvoice(
                        tax_date, zone_id, _registrar_id
                        , payment_price_rest
                        , local_current_timestamp, out_credit);

                pay_invoice(_registrar_id , zone_id, _payment->getId()
                        , out_credit, advance_invoice_id);

                remaining_credit = out_credit;
            }
            //set_payment_as_processed
            _payment->setType(2);
            _payment->save();
            LOGGER(PACKAGE).info(boost::format(
                        "payment paired with registrar (id=%1%) "
                        ) % _registrar_id );

            transaction.commit();

            return remaining_credit;
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

    ManagerImpl()
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
         //throw (std::runtime_error)
    {
        TRACE("[CALL] LibFred::Banking::Manager::importStatementXml(...)");
        Logging::Context ctx("bank xml import");



        try {
            /* load stream to string */
            _in.seekg(0, std::ios::beg);
            std::ostringstream stream;
            stream << _in.rdbuf();
            std::string xml = stream.str();

            LOGGER(PACKAGE).debug(boost::format(
                    "stream loaded into string : %1%")
                    % xml );

            /* parse */
            XMLparser parser;
            if (!parser.parse(xml)) {
                throw std::runtime_error("parser error");
            }

            LOGGER(PACKAGE).debug("string parsed");

            XMLnode root = parser.getRootNode();
            if (root.getName().compare(STATEMENTS_ROOT) != 0) {
                throw std::runtime_error(str(boost::format(
                            "root xml element name is not `<%1%>'")
                            % STATEMENTS_ROOT));
            }

            /* start transaction for saving */
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            LOGGER(PACKAGE).debug(boost::format(
                    "saving transaction for : %1%")
                    % root.getChildrenSize() );

            for (int i = 0; i < root.getChildrenSize(); i++) {

                LOGGER(PACKAGE).debug(boost::format(
                        "saving node : %1%")
                        % i );

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
                        if (payment->getAccountNumber() != cpayment->getAccountNumber()
                                || payment->getBankCode() != cpayment->getBankCode()
                                || payment->getCode() != cpayment->getCode()
                                || payment->getKonstSym() != cpayment->getKonstSym()
                                || payment->getVarSymb() != cpayment->getVarSymb()
                                || payment->getSpecSymb() != cpayment->getSpecSymb()
                                || payment->getAccountMemo() != cpayment->getAccountMemo()) {

                            LOGGER(PACKAGE).debug(boost::format("imported payment: %1%")
                                                                % payment->toString());
                            LOGGER(PACKAGE).debug(boost::format("conflict payment: %1%")
                                                                % cpayment->toString());

                            LOGGER(PACKAGE).error(boost::format(
                                            "oops! found conflict payment with "
                                            "INCONSISTENT DATA (id=%1% account_evid=%2%) "
                                            "importing file=%3% -- please make manual checking"
                                            "; skipping payment")
                                            % cpayment->getId()
                                            % cpayment->getAccountEvid()
                                            % _file_path);
                            /* lets do another payment */
                            continue;
                        }
                        /* compare changable attributes for futher processing */
                        else if (payment->getStatus() != cpayment->getStatus()) {
                            LOGGER(PACKAGE).info(boost::format(
                                    "already imported payment -- status changed "
                                    "%1% => %2% (price %3% => %4%; account_date %5% => %6%)")
                                    % cpayment->getStatus()
                                    % payment->getStatus()
                                    % cpayment->getPrice()
                                    % payment->getPrice()
                                    % cpayment->getAccountDate()
                                    % payment->getAccountDate());

                            cpayment->setStatus(payment->getStatus());
                            cpayment->setAccountDate(payment->getAccountDate());
                            cpayment->setPrice(payment->getPrice());
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

    Money importPayment(
            const std::string& _uuid,
            const std::string& _bank_payment_ident,
            const std::string& _account_number,
            const std::string& _account_bank_code,
            const std::string& _counter_account_number,
            const std::string& _counter_account_bank_code,
            const std::string& _counter_account_name,
            const std::string& _constant_symbol,
            const std::string& _variable_symbol,
            const std::string& _specific_symbol,
            const Money& _price,
            const boost::gregorian::date _date,
            const std::string& _memo,
            const boost::posix_time::ptime& _creation_time,
            const boost::optional<std::string>& _registrar_handle)
    {
        TRACE("[CALL] LibFred::Banking::Manager::importPayment(...)");
        Logging::Context ctx("bank payment import");

        try {
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            LOGGER(PACKAGE).debug("saving transaction for single payment");

            // TODO check duplicity (_uuid)

            StatementImplPtr statement(statement_from_params(_account_number, _account_bank_code));
            statement->setId(0);

            LOGGER(PACKAGE).info("no statement -- importing only payments");

            PaymentImplPtr payment(payment_from_params(
                    _bank_payment_ident,
                    _counter_account_number,
                    _counter_account_bank_code,
                    _counter_account_name,
                    _constant_symbol,
                    _variable_symbol,
                    _specific_symbol,
                    _price,
                    _date,
                    _memo,
                    _creation_time));

            payment->setAccountId(statement->getAccountId());

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

            const Money remaining_credit =
                _registrar_handle != boost::none
                    ? pairPaymentWithRegistrar(payment->getId(), *_registrar_handle)
                          // note: \ also calls processPeyment
                    : processPayment(payment.get());

            tx.commit();

            return remaining_credit;
        }
        catch (const std::exception& e) {
            throw std::runtime_error(str(boost::format(
                            "bank import payment: %1%") % e.what()));
        }
        catch (...) {
            throw std::runtime_error("bank import payment: an error occured");
        }
    }

    void addBankAccount(const std::string &_account_number,
                            const std::string &_bank_code,
                            const std::string &_zone,
                            const std::string &_account_name)
        //throw (std::runtime_error)
    {
        TRACE("[CALL] LibFred::Banking::Manager::insertBankAccount(zone_fqdn, ...)");

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
                                    "account '%1%/%2%' already exists")
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
        TRACE("[CALL] LibFred::Banking::Manager::pairPaymentWithStatement(...)");
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

    virtual Money pairPaymentWithRegistrar(
                const Database::ID &paymentId,
                const std::string &_registrarHandle) {

        TRACE("[CALL] LibFred::Invoicing::Manager::manualCreateInvoice(Database::ID, std::string)");

        //trim spaces
        std::string registrarHandle = boost::trim_copy(_registrarHandle);

        Database::Query query;
        query.buffer()
            << "SELECT id FROM registrar WHERE handle="
            << Database::Value(registrarHandle);
        Database::Connection conn = Database::Manager::acquire();
        Database::ID registrarId;
        Money remaining_credit = Money("0");
        try {
            Database::Result res = conn.exec(query);
            if(res.size() == 0) {
                    LOGGER(PACKAGE).error(boost::format(
                    "Registrar with handle '%1%' not found in database.") %
                            registrarHandle);
                    throw std::runtime_error("registrar not found");
            }
            registrarId = res[0][0];

            PaymentImpl pi;
            pi.setId(paymentId);
            pi.reload();
            remaining_credit = processPayment(&pi, registrarId);

        } catch (std::exception &e) {
            LOGGER(PACKAGE).error(boost::format("An error has occured: %1% ") % e.what());
            throw;
        }

        return remaining_credit;
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

            if (old_type == 2 || old_type == 5) {
                throw std::runtime_error(str(boost::format(
                                "payment (id=%1% type=%2%) was already "
                                "processed => type cannot be changed")
                                % payment_id
                                % old_type));
            }

            if (type == 2) {
                throw std::runtime_error(str(boost::format(
                                "payment (id=%1%) need to be processed "
                                "to set type=2 (need registrar info) use "
                                "pairPaymentWithRegistrar(...) instead")
                                % payment_id));
            }
            else {
                payment.setType(type);
                payment.save();
                LOGGER(PACKAGE).info(boost::format(
                            "successfully changed payment id=%1% type "
                            "%2% => %3%") % payment_id % old_type % type);

            }
        }
        catch (NOT_FOUND) {
            throw std::runtime_error(str(boost::format("set payment type: "
                            "payment id=%1% not found") % payment_id));
        }
        catch (std::exception &ex) {
            throw std::runtime_error(str(boost::format(
                            "set payment type: %1%") % ex.what()));
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

Manager* Manager::create()
{
    return new ManagerImpl();
}

} // namespace Banking
} // namespace LibFred

