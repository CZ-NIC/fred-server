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

#include "src/backend/accounting/accounting.hh"

#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/buffer.hh"
#include "src/backend/credit.hh"
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

#include <algorithm>
#include <cstring>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace Fred {
namespace Backend {
namespace Accounting {

namespace {

std::string create_ctx_function_name(const char* fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const std::string& _op_name)
        : ctx_operation_(_op_name)
    {
    }

private:
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(create_ctx_function_name(__FUNCTION__))

} // namespace Fred::Backend::Accounting::{anonymous}

namespace Impl {

struct InvalidAccountNumberWithBankCode : std::exception
{
    const char* what() const noexcept override
    {
        return "invalid account_number / bank_code";
    }
};

struct ZoneNotFound : std::exception
{
    const char* what() const noexcept override
    {
        return "zone not found";
    }
};

struct BankAccount
{
    static BankAccount by_account_nubmer_with_bank_code(const std::string& _account_number_with_bank_code)
    {
        std::vector<std::string> parts;
        boost::split(
                parts,
                _account_number_with_bank_code,
                boost::is_any_of("/"));
        if (parts.size() != 2)
        {
            throw InvalidAccountNumberWithBankCode();
        }
        return BankAccount(parts.at(0), parts.at(1));
    }

    const std::string account_number;
    const std::string bank_code;

    private:
        BankAccount(
                const std::string& _account_number,
                const std::string& _bank_code)
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

        Database::Result registrar_credit_id_result =
                _ctx.get_conn().exec_params(
                        "SELECT id FROM registrar_credit "
                        " WHERE registrar_id = $1::bigint AND zone_id = $2::bigint",
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

        Database::Result registrar_credit_transaction_id_result =
               _ctx.get_conn().exec_params(
                       "INSERT INTO registrar_credit_transaction "
                       " (id, balance_change, registrar_credit_id) "
                       " VALUES (DEFAULT, $1::numeric, $2::bigint) "
                       " RETURNING id",
                       Database::query_param_list
                               (_credit_amount_to_add.value)
                               (registrar_credit_id));

        if (registrar_credit_transaction_id_result.size() != 1)
        {
            throw std::runtime_error("add_credit_to_invoice: registrar_credit_transaction not found");
        }

        // unsigned long long registrar_credit_transaction_id = registrar_credit_transaction_id_result[0][0];
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e) {
        if (e.is_set_unknown_registrar_handle()) {
            throw RegistrarNotFound();
        }
        throw;
    }
    catch (const LibFred::Zone::NonExistentZone& e) {
        throw InvalidZone();
    }
}

void increase_zone_credit_of_registrar(
        LibFred::OperationContext& _ctx,
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_add)
{
    if (_credit_amount_to_add.value.is_special() ||
        _credit_amount_to_add.value.is_negative())
    {
        throw InvalidCreditValue();
    }
    change_zone_credit_of_registrar(
            _ctx, _transaction_ident, _registrar_handle, _zone, _credit_amount_to_add);
}

void decrease_zone_credit_of_registrar(
        LibFred::OperationContext& _ctx,
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_substract)
{
    if (_credit_amount_to_substract.value.is_special() ||
        _credit_amount_to_substract.value.is_negative())
    {
        throw InvalidCreditValue();
    }
    change_zone_credit_of_registrar(
            _ctx, _transaction_ident, _registrar_handle, _zone, _credit_amount_to_substract);
}

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        LibFred::OperationContext& _ctx,
        const PaymentData& _payment_data)
{
    Database::Result dbres = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT id, "
                   "ico, " //
                   "dic, " //
                   "varsymb, "
                   "vat, "
                   "handle, " //
                   "name, " //
                   "organization, " //
                   "street1, " //
                   "street2, " //
                   "street3, " //
                   "city, " //
                   "stateorprovince, " //
                   "postalcode, " //
                   "country, " //
                   "telephone, " //
                   "fax, " //
                   "email, "
                   "url " //
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
    Database::Result dbres = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT id, "
                   "ico, " //
                   "dic, " //
                   "varsymb, "
                   "vat, "
                   "handle, " //
                   "name, " //
                   "organization, " //
                   "street1, " //
                   "street2, " //
                   "street3, " //
                   "city, " //
                   "stateorprovince, " //
                   "postalcode, " //
                   "country, " //
                   "telephone, " //
                   "fax, " //
                   "email, "
                   "url " //
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
        throw std::runtime_error("too many registrars for given varsymb");
    }

    Registrar registrar;
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
    BankAccount bank_account = BankAccount::by_account_nubmer_with_bank_code(_payment_data.account_number);

    Database::Result dbres = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT fqdn "
              "FROM zone "
              "JOIN bank_account "
                "ON zone.id = bank_account.zone "
             "WHERE bank_account.account_number = $1::text "
               "AND bank_account.bank_code = $2::text",
            // clang-format on
            Database::query_param_list(bank_account.account_number)
                                      (bank_account.bank_code));

    if (dbres.size() < 1)
    {
        throw ZoneNotFound();
    }
    if (dbres.size() > 1)
    {
        throw std::runtime_error("too many registrars for given varsymb");
    }

    const std::string zone = static_cast<std::string>(dbres[0]["fqdn"]);

    return zone;
}

void import_payment(
        const PaymentData& _payment_data,
        Credit& _credit)
{
    // init file manager
    //CorbaClient corba_client(0, 0, m_nsAddr, nameservice_context);
    //FileManagerClient fm_client(corba_client.getNS());
    //LibFred::File::ManagerPtr file_manager(LibFred::File::Manager::create(&fm_client));
    // bank manager
    //LibFred::Banking::ManagerPtr banking_manager(LibFred::Banking::Manager::create(file_manager.get()));
    LibFred::Banking::ManagerPtr banking_manager(LibFred::Banking::Manager::create());
    banking_manager->importPayment(
            _payment_data.bank_payment,
            _payment_data.uuid,
            _payment_data.account_number,
            _payment_data.counter_account_number,
            _payment_data.counter_account_name,
            _payment_data.constant_symbol,
            _payment_data.variable_symbol,
            _payment_data.specific_symbol,
            _payment_data.price,
            _payment_data.date,
            _payment_data.memo,
            _payment_data.creation_time);
}

} // namespace Fred::Backend::Accounting::Impl

void increase_zone_credit_of_registrar(
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_add)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER(PACKAGE).info(__FUNCTION__);
    try
    {
        LibFred::OperationContextCreator ctx;

        Impl::increase_zone_credit_of_registrar(
                ctx,
                _transaction_ident,
                _registrar_handle,
                _zone,
                _credit_amount_to_add);

        ctx.commit_transaction();
    }
    catch (const InvalidZone& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const RegistrarNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const InvalidCreditValue& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

void decrease_zone_credit_of_registrar(
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_substract)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER(PACKAGE).info(__FUNCTION__);
    try
    {
        LibFred::OperationContextCreator ctx;

        Impl::decrease_zone_credit_of_registrar(
                ctx,
                _transaction_ident,
                _registrar_handle,
                _zone,
                _credit_amount_to_substract);

        ctx.commit_transaction();
    }
    catch (const InvalidZone& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const RegistrarNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const InvalidCreditValue& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        const PaymentData& _payment_data,
        std::string& _zone)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER(PACKAGE).info(__FUNCTION__);
    try
    {
        LibFred::OperationContextCreator ctx;

        _zone = Impl::get_zone_by_payment(
                ctx,
                _payment_data);
        return Impl::get_registrar_by_payment(
                ctx,
                _payment_data);
    }
    catch (const Impl::ZoneNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw RegistrarNotFound();
    }
    catch (const Impl::InvalidAccountNumberWithBankCode& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw RegistrarNotFound();
    }
    catch (const RegistrarNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

Fred::Backend::Accounting::Registrar get_registrar_by_handle_and_payment(
        const std::string& _registrar_handle,
        const PaymentData& _payment_data,
        std::string& _zone)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER(PACKAGE).info(__FUNCTION__);
    try
    {
        LibFred::OperationContextCreator ctx;

        _zone = Impl::get_zone_by_payment(
                ctx,
                _payment_data);
        return Impl::get_registrar_by_handle(
                ctx,
                _registrar_handle);
    }
    catch (const Impl::ZoneNotFound& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw RegistrarNotFound();
    }
    catch (const Impl::InvalidAccountNumberWithBankCode& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw RegistrarNotFound();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

void import_payment(
        const PaymentData& _payment_data,
        Credit& _credit)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER(PACKAGE).info(__FUNCTION__);
    try
    {
        return Impl::import_payment(
                _payment_data,
                _credit);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred
