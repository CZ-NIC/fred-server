/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/accounting/impl/accounting.hh"

#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/impl/exceptions.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/payment_invoices.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/accounting/registrar_reference.hh"
#include "src/backend/credit.hh"
#include "src/deprecated/libfred/banking/bank_manager.hh"
#include "src/deprecated/libfred/banking/payment_invoices.hh"
#include "src/deprecated/libfred/banking/exceptions.hh"
#include "src/deprecated/libfred/invoicing/exceptions.hh"
#include "libfred/object/object_type.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrar/credit/create_registrar_credit_transaction.hh"
#include "libfred/registrar/credit/exceptions.hh"
#include "libfred/zone/exceptions.hh"
#include "util/log/context.hh"
#include "util/random.hh"
#include "src/util/subprocess.hh"
#include "util/util.hh"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/none.hpp>

#include <algorithm>
#include <cstring>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <boost/optional/optional.hpp>

namespace Fred {
namespace Backend {
namespace Accounting {
namespace Impl {

struct BankAccount
{
    static BankAccount from_string(const std::string& _account_number_with_bank_code)
    {
        std::vector<std::string> parts;
        boost::split(
                parts,
                _account_number_with_bank_code,
                boost::is_any_of("/"));
        if (parts.size() == 1)
        {
            const auto no_bank_code = boost::none;
            return BankAccount(parts.at(0), no_bank_code);
        }
        else if (parts.size() == 2)
        {
            return BankAccount(parts.at(0), parts.at(1));
        }
        throw InvalidAccountData();
    }

    const std::string account_number;
    const boost::optional<std::string> bank_code;

private:
    BankAccount(
            const std::string& _account_number,
            const boost::optional<std::string>& _bank_code)
        : account_number(_account_number),
          bank_code(_bank_code)
    {
    }
};

void change_zone_credit_of_registrar(
        LibFred::OperationContext& _ctx,
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_add)
{
    if (_credit_amount_to_add.value.is_special())
    {
        throw InvalidCreditValue();
    }

    try {
        LibFred::Registrar::Credit::CreateRegistrarCreditTransaction(_registrar_handle,
                    _zone,
                    _credit_amount_to_add.value)
                .exec(_ctx);
    }
    catch (const LibFred::Registrar::Credit::NonexistentRegistrar&)
    {
        throw RegistrarNotFound();
    }
    catch (const LibFred::Zone::NonExistentZone&)
    {
        throw ZoneNotFound();
    }
    catch (const LibFred::Registrar::Credit::NonexistentZoneAccess&)
    {
        throw ;
    }
}

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        LibFred::OperationContext& _ctx,
        const PaymentData& _payment_data)
{
    if (_payment_data.variable_symbol.empty())
    {
        throw InvalidPaymentData();
    }

    const Database::Result dbres = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT id, "
                   "ico, "
                   "dic, "
                   "handle, "
                   "name, "
                   "organization, "
                   "street1, "
                   "street2, "
                   "street3, "
                   "city, "
                   "stateorprovince, "
                   "postalcode, "
                   "country, "
                   "telephone, "
                   "fax, "
                   "url "
              "FROM registrar "
             "WHERE registrar.varsymb=$1::text",
            // clang-format on
            Database::query_param_list(_payment_data.variable_symbol));

    if (dbres.size() < 1)
    {
        throw RegistrarNotFound();
    }
    if (dbres.size() > 1)
    {
        throw std::runtime_error("too many registrars for given varsymb");
    }

    Registrar registrar;
    registrar.id = static_cast<unsigned long long>(dbres[0]["id"]);
    registrar.handle = static_cast<std::string>(dbres[0]["handle"]);
    registrar.name = static_cast<std::string>(dbres[0]["name"]);
    registrar.organization = static_cast<std::string>(dbres[0]["organization"]);
    registrar.cin = static_cast<std::string>(dbres[0]["ico"]);
    registrar.tin = static_cast<std::string>(dbres[0]["dic"]);
    registrar.url = static_cast<std::string>(dbres[0]["url"]);
    registrar.phone = static_cast<std::string>(dbres[0]["telephone"]);
    registrar.fax = static_cast<std::string>(dbres[0]["fax"]);
    registrar.address.street1 = static_cast<std::string>(dbres[0]["street1"]);
    registrar.address.street2 = static_cast<std::string>(dbres[0]["street2"]);
    registrar.address.street3 = static_cast<std::string>(dbres[0]["street3"]);
    registrar.address.city = static_cast<std::string>(dbres[0]["city"]);
    registrar.address.stateorprovince = static_cast<std::string>(dbres[0]["stateorprovince"]);
    registrar.address.postalcode = static_cast<std::string>(dbres[0]["postalcode"]);
    registrar.address.country_code = static_cast<std::string>(dbres[0]["country"]);

    return registrar;
}

Fred::Backend::Accounting::Registrar get_registrar_by_handle(
        LibFred::OperationContext& _ctx,
        const std::string& _handle)
{
    const Database::Result dbres = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT id, "
                   "ico, "
                   "dic, "
                   "handle, "
                   "name, "
                   "organization, "
                   "street1, "
                   "street2, "
                   "street3, "
                   "city, "
                   "stateorprovince, "
                   "postalcode, "
                   "country, "
                   "telephone, "
                   "fax, "
                   "url "
              "FROM registrar "
             "WHERE registrar.handle=$1::text",
            // clang-format on
            Database::query_param_list(_handle));

    if (dbres.size() < 1)
    {
        throw RegistrarNotFound();
    }
    if (dbres.size() > 1)
    {
        throw std::runtime_error("too many registrars for given handle");
    }

    Registrar registrar;
    registrar.id = static_cast<unsigned long long>(dbres[0]["id"]);
    registrar.handle = static_cast<std::string>(dbres[0]["handle"]);
    registrar.name = static_cast<std::string>(dbres[0]["name"]);
    registrar.organization = static_cast<std::string>(dbres[0]["organization"]);
    registrar.cin = static_cast<std::string>(dbres[0]["ico"]);
    registrar.tin = static_cast<std::string>(dbres[0]["dic"]);
    registrar.url = static_cast<std::string>(dbres[0]["url"]);
    registrar.phone = static_cast<std::string>(dbres[0]["telephone"]);
    registrar.fax = static_cast<std::string>(dbres[0]["fax"]);
    registrar.address.street1 = static_cast<std::string>(dbres[0]["street1"]);
    registrar.address.street2 = static_cast<std::string>(dbres[0]["street2"]);
    registrar.address.street3 = static_cast<std::string>(dbres[0]["street3"]);
    registrar.address.city = static_cast<std::string>(dbres[0]["city"]);
    registrar.address.stateorprovince = static_cast<std::string>(dbres[0]["stateorprovince"]);
    registrar.address.postalcode = static_cast<std::string>(dbres[0]["postalcode"]);
    registrar.address.country_code = static_cast<std::string>(dbres[0]["country"]);

    return registrar;
}

std::string get_zone_by_payment(
        LibFred::OperationContext& _ctx,
        const PaymentData& _payment_data)
{

    LOGGER.info(_payment_data.account_number);
    const BankAccount bank_account = BankAccount::from_string(_payment_data.account_number);

    const Database::Result dbres = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT zone.fqdn "
              "FROM zone "
              "JOIN bank_account "
                "ON zone.id = bank_account.zone "
             "WHERE bank_account.account_number = $1::text "
               "AND bank_account.bank_code = $2::text",
            // clang-format on
            Database::query_param_list
                    (bank_account.account_number)
                    (bank_account.bank_code.get_value_or("")));

    if (dbres.size() < 1)
    {
        throw ZoneNotFound();
    }
    if (dbres.size() > 1)
    {
        throw std::runtime_error("too many zones for given account_number/bank_code");
    }

    const std::string zone = static_cast<std::string>(dbres[0]["fqdn"]);

    return zone;
}

namespace {

InvoiceType convert_LibFred_Banking_InvoiceType_to_Fred_Backend_Accounting_InvoiceType(
        const LibFred::Banking::InvoiceType _invoice_type)
{
        switch (_invoice_type)
        {
            case LibFred::Banking::InvoiceType::advance: return InvoiceType::advance;
            case LibFred::Banking::InvoiceType::account: return InvoiceType::account;
        }
        throw std::logic_error("unexpected LibFred::Banking::InvoiceType");
}

InvoiceReference convert_LibFred_Baking_InvoiceReference_to_Fred_Backend_Accounting_InvoiceReference(
        const LibFred::Banking::InvoiceReference& _invoice_reference)
{
    return InvoiceReference(
            _invoice_reference.id,
            _invoice_reference.number,
            convert_LibFred_Banking_InvoiceType_to_Fred_Backend_Accounting_InvoiceType(_invoice_reference.type),
            Credit(_invoice_reference.credit_change.get_string()));
}

PaymentInvoices convert_vector_of_LibFred_Baking_InvoiceReference_to_Fred_Backend_Accounting_PaymentInvoices(
        const LibFred::Banking::PaymentInvoices& _payment_invoices)
{
        PaymentInvoices payment_invoices;
        if (_payment_invoices.advance_invoice != boost::none)
        {
            payment_invoices.advance_invoice =
                    convert_LibFred_Baking_InvoiceReference_to_Fred_Backend_Accounting_InvoiceReference(
                            *_payment_invoices.advance_invoice);
        }
        if (_payment_invoices.account_invoices.size())
        {
            payment_invoices.account_invoices.reserve(_payment_invoices.account_invoices.size());
            std::transform(
                    _payment_invoices.account_invoices.begin(),
                    _payment_invoices.account_invoices.end(),
                    std::back_inserter(payment_invoices.account_invoices),
                    convert_LibFred_Baking_InvoiceReference_to_Fred_Backend_Accounting_InvoiceReference);
        }
        return payment_invoices;
}

} // namespace Fred::Backend::Accounting::Impl::{anonymous}

PaymentInvoices import_payment(
        const PaymentData& _payment_data)
{

    const BankAccount bank_account = BankAccount::from_string(_payment_data.account_number);
    const BankAccount counter_account = BankAccount::from_string(_payment_data.counter_account_number);
    const auto no_registrar_handle = boost::none;

    try {
        LibFred::Banking::ManagerPtr banking_manager(LibFred::Banking::Manager::create());
        const auto payment_invoices =
                banking_manager->importPayment(
                        _payment_data.uuid,
                        bank_account.account_number,
                        bank_account.bank_code,
                        _payment_data.account_payment_ident,
                        counter_account.account_number,
                        counter_account.bank_code,
                        _payment_data.counter_account_name,
                        _payment_data.constant_symbol,
                        _payment_data.variable_symbol,
                        _payment_data.specific_symbol,
                        _payment_data.price,
                        _payment_data.date,
                        _payment_data.memo,
                        _payment_data.creation_time,
                        no_registrar_handle,
                        boost::none);

        return convert_vector_of_LibFred_Baking_InvoiceReference_to_Fred_Backend_Accounting_PaymentInvoices(
                payment_invoices);
    }
    catch (const LibFred::Banking::RegistrarNotFound&)
    {
        throw RegistrarNotFound();
    }
    catch (const LibFred::Banking::InvalidAccountData&)
    {
        throw InvalidAccountData();
    }
    catch (const LibFred::Banking::InvalidPriceValue&)
    {
        throw InvalidPaymentData();
    }
    catch (const LibFred::Banking::InvalidPaymentData&)
    {
        throw InvalidPaymentData();
    }
    catch (const LibFred::Invoicing::InvalidTaxDateFormat&)
    {
        throw InvalidPaymentData(); // tax_date was internally taken from payment_data.date which means from client's point of view, payment_data is invalid
    }
    catch (const LibFred::Invoicing::TaxDateTooOld&)
    {
        throw PaymentTooOld(); // tax_date was internally taken from payment_data.date which means from client's point of view, payment_data is invalid
    }
    catch (const LibFred::Banking::PaymentAlreadyProcessed&)
    {
        throw PaymentAlreadyProcessed();
    }
}

PaymentInvoices import_payment_by_registrar_handle(
        const PaymentData& _payment_data,
        const boost::optional<boost::gregorian::date>& _tax_date,
        const std::string& _registrar_handle)
{
    const BankAccount bank_account = BankAccount::from_string(_payment_data.account_number);
    const BankAccount counter_account = BankAccount::from_string(_payment_data.counter_account_number);

    try {
        LibFred::Banking::ManagerPtr banking_manager(LibFred::Banking::Manager::create());
        const auto payment_invoices =
                banking_manager->importPayment(
                        _payment_data.uuid,
                        bank_account.account_number,
                        bank_account.bank_code,
                        _payment_data.account_payment_ident,
                        counter_account.account_number,
                        counter_account.bank_code,
                        _payment_data.counter_account_name,
                        _payment_data.constant_symbol,
                        _payment_data.variable_symbol,
                        _payment_data.specific_symbol,
                        _payment_data.price,
                        _payment_data.date,
                        _payment_data.memo,
                        _payment_data.creation_time,
                        _registrar_handle,
                        _tax_date);

        return convert_vector_of_LibFred_Baking_InvoiceReference_to_Fred_Backend_Accounting_PaymentInvoices(
                payment_invoices);
    }
    catch (const LibFred::Banking::RegistrarNotFound&)
    {
        throw RegistrarNotFound();
    }
    catch (const LibFred::Banking::InvalidAccountData&)
    {
        throw InvalidAccountData();
    }
    catch (const LibFred::Banking::InvalidPriceValue&)
    {
        throw InvalidPaymentData();
    }
    catch (const LibFred::Banking::InvalidPaymentData&)
    {
        throw InvalidPaymentData();
    }
    catch (const LibFred::Invoicing::InvalidTaxDateFormat&)
    {
        throw InvalidTaxDateFormat();
    }
    catch (const LibFred::Invoicing::TaxDateTooOld&)
    {
        throw TaxDateTooOld();
    }
    catch (const LibFred::Banking::PaymentAlreadyProcessed&)
    {
        throw PaymentAlreadyProcessed();
    }
}

std::vector<RegistrarReference> get_registrar_references(
        LibFred::OperationContext& _ctx)
{
    std::vector<RegistrarReference> registrar_references;
    const Database::Result dbresult =
            _ctx.get_conn().exec(
                    // clang-format off
                    "SELECT handle, name FROM registrar "
                     "ORDER BY name");
                    // clang-format on
    registrar_references.reserve(dbresult.size());
    for (Database::Result::size_type row_index = 0; row_index < dbresult.size(); ++row_index)
    {
        registrar_references.emplace_back(
                        static_cast<std::string>(dbresult[row_index]["handle"]),
                        static_cast<std::string>(dbresult[row_index]["name"]));
    }
    return registrar_references;
}

} // namespace Fred::Backend::Accounting::Impl
} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred
