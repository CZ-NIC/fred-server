#include <string>
#include <iostream>
#include <algorithm>

#include "src/libfred/banking/bank_common.hh"
#include "src/libfred/banking/bank_manager.hh"
#include "src/libfred/banking/exceptions.hh"
#include "src/libfred/banking/payment_data.hh"
#include "src/libfred/credit.hh"
#include "src/libfred/invoicing/invoice.hh"
#include "src/libfred/registrar.hh"
#include "src/util/types/money.hh"
#include "src/util/types/stringify.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <utility>


namespace LibFred {
namespace Banking {

namespace {

unsigned long long get_registrar_id_by_handle(
        const std::string& _registrar_handle)
{
    const std::string registrar_handle = boost::trim_copy(_registrar_handle);

    Database::Query query;
    query.buffer() <<
            // clang-format off
            "SELECT id "
              "FROM registrar "
             "WHERE handle = " << Database::Value(registrar_handle);
            // clang-format on
    Database::Connection conn = Database::Manager::acquire();
    const Database::Result res = conn.exec(query);
    if (res.size() == 0) {
            LOGGER(PACKAGE).error(boost::str(boost::format(
                    "Registrar with handle '%1%' not found in database.") % registrar_handle));
            throw RegistrarNotFound();
    }
    const auto registrar_id = static_cast<unsigned long long>(res[0][0]);
    return registrar_id;
}

unsigned long long get_registrar_id_by_payment(
        const PaymentData& _payment)
{
    Database::Query query;
    query.buffer() <<
            // clang-format off
            "SELECT id "
              "FROM registrar "
             "WHERE varsymb = " << Database::Value(_payment.variable_symbol) << " "
                "OR (length(trim(regex)) > 0 "
                    "AND " << Database::Value(_payment.memo) << " ~* trim(regex))";
            // clang-format on
    Database::Connection conn = Database::Manager::acquire();
    const Database::Result result = conn.exec(query);
    if (result.size() != 1) {
        LOGGER(PACKAGE).warning(boost::format(
                    "couldn't find suitable registrar for payment id=%1% "
                    "=> processing canceled") % _payment.uuid);
        throw RegistrarNotFound();
    }
    const auto registrar_id = static_cast<unsigned long long>(result[0][0]);
    return registrar_id;
}

unsigned long long get_account_id_by_account_number_and_bank_code(
        const std::string& _account_number,
        const boost::optional<std::string>& _account_bank_code)
{
    if (_account_number.empty() || _account_bank_code.get_value_or("").empty())
    {
        LOGGER(PACKAGE).error(boost::str(boost::format(
                "invalid account_number/account_bank_code (%1%/%2%)")
                % _account_number % _account_bank_code.get_value_or("")));
        throw InvalidAccountData();
    }

    Database::Query query;
    query.buffer() <<
            // clang-format off
            "SELECT id "
              "FROM bank_account "
             "WHERE TRIM(LEADING '0' FROM account_number) = "
                   "TRIM(LEADING '0' FROM " << Database::Value(_account_number) << ") "
               "AND bank_code = " << Database::Value(_account_bank_code.get_value_or(""));
            // clang-format on
    Database::Connection conn = Database::Manager::acquire();
    const Database::Result result = conn.exec(query);
    if (result.size() == 0) {
        LOGGER(PACKAGE).error(boost::str(boost::format(
                "not valid record found in database for account=%1% bankcode=%2%")
                % _account_number % _account_bank_code.get_value_or("")));
        throw InvalidAccountData();
    }
    const auto account_id = static_cast<unsigned long long>(result[0][0]);
    return account_id;
}

std::string get_invoice_number_by_id(
        unsigned long long _invoice_id)
{
    Database::Query query;
    query.buffer() <<
            // clang-format off
            "SELECT prefix "
              "FROM invoice "
             "WHERE id = " << Database::Value(_invoice_id);
            // clang-format on
    Database::Connection conn = Database::Manager::acquire();
    const Database::Result res = conn.exec(query);
    if (res.size() == 0) {
            LOGGER(PACKAGE).error(boost::str(boost::format(
                    "Invoice with id '%1%' not found in database.") % _invoice_id));
            throw std::runtime_error("invoice not found");
    }
    const auto invoice_number = static_cast<std::string>(res[0][0]);
    return invoice_number;
}

} // LibFred::Banking::{anonymous}

class ManagerImpl : virtual public Manager
{
private:
    File::Manager *file_manager_;


    unsigned long long getZoneByAccountId(const unsigned long long &_account_id)
    {
        Database::Query query;
        query.buffer() <<
                // clang-format off
                "SELECT zone "
                  "FROM bank_account "
                 "WHERE id = " << Database::Value(_account_id);
                // clang-format on

        Database::Connection conn = Database::Manager::acquire();
        const Database::Result result = conn.exec(query);

        if (result.size() != 0) {
            return static_cast<unsigned long long>(result[0][0]);
        }
        else {
            return 0;
        }
    }

    void pay_invoice(
            unsigned long long registrar_id,
            unsigned long long zone_id,
            const std::string& bank_payment_uuid,
            Money price,
            unsigned long long invoice_id)
    {
        Logging::Context ctx("pay_invoice");
        Database::Connection conn = Database::Manager::acquire();

        //add credit
        unsigned long long registrar_credit_transaction_id =
        LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, price, invoice_id);

        //insert_bank_payment_registrar_credit_transaction_map
        conn.exec_params(
                // clang-format off
                "INSERT INTO bank_payment_registrar_credit_transaction_map "
                "(bank_payment_uuid, registrar_credit_transaction_id) "
                "VALUES ($1::uuid, $2::bigint)",
                Database::query_param_list
                        (bank_payment_uuid)
                        (registrar_credit_transaction_id));
                // clang-format on


    }

    PaymentInvoices processPayment(
            PaymentData& _payment,
            unsigned long long _registrar_id)
    {
        Logging::Context ctx("payment processing");
        try
        {
            PaymentInvoices payment_invoices;

            if (_payment.price <= Money("0"))
            {
                throw std::runtime_error("could not process payment, price not > 0");
            }

            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction transaction(conn);

            DBSharedPtr nodb;
            LibFred::Registrar::Manager::AutoPtr rmanager(LibFred::Registrar::Manager::create(nodb));

            unsigned long long zone_id = getZoneByAccountId(_payment.account_id);


            std::unique_ptr<LibFred::Invoicing::Manager>
                    invoice_manager(LibFred::Invoicing::Manager::create());

            //find_unpaid_account_invoices

            std::vector<LibFred::Invoicing::unpaid_account_invoice> uai_vect
                = invoice_manager->find_unpaid_account_invoices(
                     _registrar_id, zone_id);

            Money payment_price_rest = _payment.price;

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
                        pay_invoice(_registrar_id , zone_id, _payment.uuid
                            , balance_change, unpaid_account_invoice_id);
                        payment_invoices.account_invoices.push_back(
                                InvoiceReference(
                                        unpaid_account_invoice_id,
                                        get_invoice_number_by_id(unpaid_account_invoice_id),
                                        InvoiceType::account,
                                        balance_change));
                    }
                    else
                    {
                        Money balance_change //is rest of invoice balance without vat
                            = invoice_manager->zero_account_invoice_balance(
                                unpaid_account_invoice_id);
                        pay_invoice(_registrar_id , zone_id, _payment.uuid
                            , balance_change, unpaid_account_invoice_id);
                        payment_invoices.account_invoices.push_back(
                                InvoiceReference(
                                        unpaid_account_invoice_id,
                                        get_invoice_number_by_id(unpaid_account_invoice_id),
                                        InvoiceType::account,
                                        balance_change));
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
                            % _payment.uuid);
                    throw std::runtime_error("could not process payment");
                }

                // amount larger than registrar debt
                if(payment_price_rest > Money("0")) {
                    LOGGER(PACKAGE).warning(boost::format(
                            "registrar (id=%1%) who has no longer access to zone (id=%2%)"
                            " sent amount larger than debt (payment id=%3%) it will have to be resolved manually")
                            % _registrar_id
                            % zone_id
                            % _payment.uuid);
                    throw std::runtime_error("could not process payment");

                }
            }

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

                pay_invoice(_registrar_id , zone_id, _payment.uuid
                        , out_credit, advance_invoice_id);

                payment_invoices.advance_invoice =
                        InvoiceReference(
                                advance_invoice_id,
                                get_invoice_number_by_id(advance_invoice_id),
                                InvoiceType::advance,
                                out_credit);
            }
            LOGGER(PACKAGE).info(boost::format(
                        "payment paired with registrar (id=%1%) "
                        ) % _registrar_id );

            transaction.commit();

            return payment_invoices;
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).info(boost::str(boost::format(
                            "processing payment failed: %1%") % ex.what()));
            throw;
        }
    }


public:
    explicit ManagerImpl(File::Manager* _file_manager)
        : file_manager_(_file_manager)
    {
    }

    ManagerImpl()
    {
    }

    PaymentInvoices importPayment(
            const std::string& _uuid,
            const std::string& _account_number,
            const boost::optional<std::string>& _account_bank_code,
            const std::string& _account_payment_ident,
            const std::string& _counter_account_number,
            const boost::optional<std::string>& _counter_account_bank_code,
            const std::string& _counter_account_name,
            const std::string& _constant_symbol,
            const std::string& _variable_symbol,
            const std::string& _specific_symbol,
            const Money& _price,
            const boost::gregorian::date _date,
            const std::string& _memo,
            const boost::posix_time::ptime _creation_time,
            const boost::optional<std::string>& _registrar_handle)
    {
        TRACE("[CALL] LibFred::Banking::Manager::importPayment(...)");
        Logging::Context ctx("bank payment import");

        try {
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            LOGGER(PACKAGE).debug("saving transaction for a single payment");

            Database::Query query;
            // clang-format off
            query.buffer() << "SELECT 1 "
                                "FROM bank_payment_registrar_credit_transaction_map "
                               "WHERE bank_payment_uuid = " << Database::Value(_uuid);
            // clang-format on
            const Database::Result result = conn.exec(query);
            if (result.size() != 0) {
                throw PaymentAlreadyProcessed();
            }

            PaymentData payment =
                    PaymentData(
                            _uuid,
                            get_account_id_by_account_number_and_bank_code(_account_number, _account_bank_code),
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
                    % payment.uuid
                    % payment.counter_account_number
                    % payment.counter_account_bank_code.get_value_or("")
                    % payment.account_payment_ident
                    % payment.price
                    % payment.date
                    % payment.account_id);

            const unsigned long long registrar_id =
                    _registrar_handle != boost::none
                            ? get_registrar_id_by_handle(*_registrar_handle)
                            : get_registrar_id_by_payment(payment);

            const auto invoice_references = processPayment(payment, registrar_id);

            tx.commit();

            return invoice_references;
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
            throw std::runtime_error("bank import payment: an unexpected exception");
        }
    }

    void addBankAccount(
            const std::string& _account_number,
            const std::string& _bank_code,
            const std::string& _zone,
            const std::string& _account_name)
    {
        TRACE("[CALL] LibFred::Banking::Manager::insertBankAccount(zone_fqdn, ...)");

        try {
            Database::Connection conn = Database::Manager::acquire();

            Database::ID zone_id = 0;
            if (!_zone.empty()) {
                Database::Query query;
                query.buffer() << "SELECT id FROM zone WHERE fqdn=" << Database::Value(_zone);

                const Database::Result result = conn.exec(query);
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
                const Database::Result result = conn.exec(query);
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

