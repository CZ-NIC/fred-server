#include <string>
#include <iostream>
#include <algorithm>
// #include <magic.h>

#include "src/libfred/banking/bank_statement_impl.hh"
#include "src/libfred/banking/bank_statement_list_impl.hh"
#include "src/libfred/banking/bank_common.hh"
#include "src/libfred/banking/bank_manager.hh"
#include "src/libfred/banking/exceptions.hh"
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
            , const std::string& bank_payment_uuid
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
                " (bank_payment_uuid, registrar_credit_transaction_id) "
                " VALUES ($1::uuid, $2::bigint) "
        ,Database::query_param_list(bank_payment_uuid)
        (registrar_credit_transaction_id));


    }

    Money processPayment(PaymentImpl *_payment,
                        unsigned long long _registrar_id = 0)
    {
        Logging::Context ctx("payment processing");
        try
        {
            if (_payment->getPrice() <= Money("0")) return Money("0");

            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction transaction(conn);

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
                                "=> processing canceled") % _payment->getUuid());
                    throw RegistrarNotFound();
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
                        pay_invoice(_registrar_id , zone_id, _payment->getUuid()
                            , balance_change, unpaid_account_invoice_id);
                    }
                    else
                    {
                        Money balance_change //is rest of invoice balance without vat
                            = invoice_manager->zero_account_invoice_balance(
                                unpaid_account_invoice_id);
                        pay_invoice(_registrar_id , zone_id, _payment->getUuid()
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
                            % _payment->getUuid());
                    throw std::runtime_error("could not process payment");
                }

                // amount larger than registrar debt
                if(payment_price_rest > Money("0")) {
                    LOGGER(PACKAGE).warning(boost::format(
                            "registrar (id=%1%) who has no longer access to zone (id=%2%)"
                            " sent amount larger than debt (payment id=%3%) it will have to be resolved manually")
                            % _registrar_id
                            % zone_id
                            % _payment->getUuid());
                    throw std::runtime_error("could not process payment");

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

                pay_invoice(_registrar_id , zone_id, _payment->getUuid()
                        , out_credit, advance_invoice_id);

                remaining_credit = out_credit;
            }
            LOGGER(PACKAGE).info(boost::format(
                        "payment paired with registrar (id=%1%) "
                        ) % _registrar_id );

            transaction.commit();

            return remaining_credit;
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).info(boost::str(boost::format(
                            "processing payment failed: %1%") % ex.what()));
            throw;
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

    void importStatementXml(std::istream &_in,
                            const std::string &_file_path,
                            const std::string &_file_mime,
                            const bool &_generate_invoices = false)
         //throw (std::runtime_error)
    {
        TRACE("[CALL] LibFred::Banking::Manager::importStatementXml(...)");
        Logging::Context ctx("bank xml import");
        throw std::runtime_error("bank xml import: obsolete");
    }

    Money importPayment(
            const std::string& _uuid,
            const std::string& _account_number,
            const std::string& _account_bank_code,
            const std::string& _account_payment_ident,
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

            const unsigned long long account_id = statement_from_params(_account_number, _account_bank_code)->getAccountId();

            Database::Query query;
            // clang-format off
            query.buffer() << "SELECT 1 "
                                "FROM bank_payment_registrar_credit_transaction_map "
                               "WHERE bank_payment_uuid = " << Database::Value(_uuid);
            // clang-format on
            Database::Result result = conn.exec(query);
            if (result.size() != 0) {
                throw PaymentAlreadyProcessed();
            }

            PaymentImplPtr processable_payment =
                    make_importable_payment(
                            _uuid,
                            account_id,
                            _account_payment_ident,
                            _counter_account_number,
                            _counter_account_bank_code,
                            _counter_account_name,
                            _constant_symbol,
                            _variable_symbol,
                            _specific_symbol,
                            _price,
                            _date,
                            _memo,
                            _creation_time);

            LOGGER(PACKAGE).info(boost::format(
                    "payment imported (id=%1% account=%2%/%3% evid=%4% price=%5% account_date=%6%) account_id=%7%")
                    % processable_payment->getUuid()
                    % processable_payment->getAccountNumber()
                    % processable_payment->getBankCode()
                    % processable_payment->getAccountEvid()
                    % processable_payment->getPrice()
                    % processable_payment->getAccountDate()
                    % processable_payment->getAccountId());

            const unsigned long long registrar_id =
                    _registrar_handle != boost::none
                    ? pairPaymentWithRegistrar(_uuid, *_registrar_handle)
                    : 0;

            const Money remaining_credit = processPayment(processable_payment.get(), registrar_id);

            tx.commit();

            return remaining_credit;
        }
        catch (const LibFred::Banking::RegistrarNotFound& e) {
            LOGGER(PACKAGE).info(boost::str(boost::format(
                            "bank import payment: %1%") % e.what()));
            throw;
        }
        catch (const LibFred::Banking::InvalidAccountData& e) {
            LOGGER(PACKAGE).info(boost::str(boost::format(
                            "bank import payment: %1%") % e.what()));
            throw;
        }
        catch (const LibFred::Banking::PaymentAlreadyProcessed& e) {
            LOGGER(PACKAGE).info(boost::str(boost::format(
                            "bank import payment: %1%") % e.what()));
            throw;
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

    virtual unsigned long long pairPaymentWithRegistrar(
                const std::string& _payment_uuid,
                const std::string &_registrarHandle) {

        TRACE("[CALL] LibFred::Invoicing::Manager::manualCreateInvoice(Database::ID, std::string)");

        //trim spaces
        std::string registrarHandle = boost::trim_copy(_registrarHandle);

        Database::Query query;
        query.buffer()
            << "SELECT id FROM registrar WHERE handle="
            << Database::Value(registrarHandle);
        Database::Connection conn = Database::Manager::acquire();
        try {
            Database::Result res = conn.exec(query);
            if(res.size() == 0) {
                    LOGGER(PACKAGE).error(boost::format(
                    "Registrar with handle '%1%' not found in database.") %
                            registrarHandle);
                    throw RegistrarNotFound();
            }
            const unsigned long long registrarId = static_cast<unsigned long long>(res[0][0]);
            return registrarId;

        } catch (std::exception &e) {
            LOGGER(PACKAGE).error(boost::format("An error has occured: %1% ") % e.what());
            throw;
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

