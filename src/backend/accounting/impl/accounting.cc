/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/accounting/impl/accounting.hh"

#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/impl/exceptions.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/payment_invoices.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/accounting/registrar_reference.hh"
#include "src/backend/credit.hh"
#include "src/libfred/banking/bank_manager.hh"
#include "src/libfred/banking/payment_invoices.hh"
#include "src/libfred/banking/exceptions.hh"
#include "src/libfred/object/object_type.hh"
#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/domain/info_domain.hh"
#include "src/libfred/registrable_object/keyset/info_keyset.hh"
#include "src/libfred/registrable_object/nsset/info_nsset.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/libfred/zone/exceptions.hh"
#include "src/libfred/zone/info_zone.hh"
#include "src/libfred/zone/info_zone_data.hh"
#include "src/util/log/context.hh"
#include "src/util/random.hh"
#include "src/util/subprocess.hh"
#include "src/util/util.hh"

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
    try {
        const auto registrar_id =
                LibFred::InfoRegistrarByHandle(_registrar_handle)
                .exec(_ctx).info_registrar_data.id;

        const auto zone_id = LibFred::Zone::get_zone_id(
                LibFred::Zone::InfoZone(_zone).exec(_ctx));

        const Database::Result registrar_credit_id_result =
                _ctx.get_conn().exec_params(
                        // clang-format off
                        "SELECT id FROM registrar_credit "
                         "WHERE registrar_id = $1::bigint AND zone_id = $2::bigint",
                        // clang-format on
                        Database::query_param_list
                               (registrar_id)
                               (zone_id));

        if (registrar_credit_id_result.size() != 1)
        {
            throw std::runtime_error("add_credit_to_invoice: registrar_credit not found");
        }
        const auto registrar_credit_id = static_cast<unsigned long long>(registrar_credit_id_result[0][0]);

        if (_credit_amount_to_add.value.is_special())
        {
            throw InvalidCreditValue();
        }

        const Database::Result registrar_credit_transaction_id_result =
                _ctx.get_conn().exec_params(
                        // clang-format off
                        "INSERT INTO registrar_credit_transaction "
                        "(id, balance_change, registrar_credit_id) "
                        "VALUES (DEFAULT, $1::numeric, $2::bigint) "
                        "RETURNING id",
                        // clang-format on
                        Database::query_param_list
                                (_credit_amount_to_add.value)
                                (registrar_credit_id));

        if (registrar_credit_transaction_id_result.size() != 1)
        {
            throw std::runtime_error("add_credit_to_invoice: registrar_credit_transaction not found");
        }

    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle()) {
            throw RegistrarNotFound();
        }
        throw;
    }
    catch (const LibFred::Zone::NonExistentZone&)
    {
        throw ZoneNotFound();
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

    LOGGER(PACKAGE).info(_payment_data.account_number);
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
                        no_registrar_handle);

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
    catch (const LibFred::Banking::PaymentAlreadyProcessed&)
    {
        throw PaymentAlreadyProcessed();
    }
}

PaymentInvoices import_payment_by_registrar_handle(
        const PaymentData& _payment_data,
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
                        _registrar_handle);

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
