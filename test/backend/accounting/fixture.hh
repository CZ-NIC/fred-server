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
#ifndef FIXTURE_HH_A1194C149A024A81B9E68C00F3FC2193
#define FIXTURE_HH_A1194C149A024A81B9E68C00F3FC2193

#include "src/backend/accounting/payment_data.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_data.hh"
#include "src/util/types/money.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/test/test_tools.hpp>

#include <string>


namespace Test {
namespace Backend {
namespace Accounting {

struct Registrar
{
    ::LibFred::InfoRegistrarData data;

    Registrar(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle = "REG-TEST",
            const std::string& _registrar_variable_symbol = "111111")
    {

        ::LibFred::CreateRegistrar(_registrar_handle)
                .set_name(boost::algorithm::to_lower_copy(_registrar_handle))
                .set_variable_symbol(_registrar_variable_symbol)
                .exec(_ctx);
        data = ::LibFred::InfoRegistrarByHandle(_registrar_handle).exec(_ctx).info_registrar_data;

        // clang-format off
        _ctx.get_conn().exec_params(
                "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                    "SELECT $1::bigint, z.id, NOW() "
                        "FROM zone z "
                        "WHERE z.fqdn = $2::text",
                Database::query_param_list(data.id)
                ("cz"));
        // clang-format on

        // clang-format off
        _ctx.get_conn().exec_params(
                "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                    "SELECT $1::bigint, z.id, NOW() "
                        "FROM zone z "
                        "WHERE z.fqdn = $2::text",
                Database::query_param_list(data.id)
                ("0.2.4.e164.arpa"));
        // clang-format on
    }
};

struct BankAccount
{
    BankAccount(
            ::LibFred::OperationContext& _ctx,
            const std::string& _account_number,
            const boost::optional<std::string>& _bank_code)
        : account_number(_account_number),
          bank_code(_bank_code)
    {
        // clang-format off
        _ctx.get_conn().exec_params(
                "INSERT INTO bank_account (zone, account_number, bank_code) "
                    "SELECT zone.id, $1::text, $2::text "
                        "FROM zone "
                        "WHERE zone.fqdn = $3::text",
                Database::query_param_list(account_number)(bank_code.get_value_or(""))("cz"));
        // clang-format on
    }

    std::string account_number;
    boost::optional<std::string> bank_code;
};

struct Payment
{
    Payment(
            const std::string& _uuid,
            const std::string& _account_number,
            const std::string& _account_payment_ident,
            const std::string& _counter_account_number,
            const std::string& _counter_account_name,
            const std::string& _constant_symbol,
            const std::string& _variable_symbol,
            const std::string& _specific_symbol,
            const Money& _price,
            const boost::gregorian::date& _date,
            const std::string& _memo,
            const boost::posix_time::ptime _creation_time)
    {
        data.uuid = _uuid;
        data.account_number = _account_number;
        data.account_payment_ident = _account_payment_ident;
        data.counter_account_number = _counter_account_number;
        data.counter_account_name = _counter_account_name;
        data.constant_symbol = _constant_symbol;
        data.variable_symbol = _variable_symbol;
        data.specific_symbol = _specific_symbol;
        data.price = _price;
        data.date = _date;
        data.memo = _memo;
        data.creation_time = _creation_time;
    }

    ::Fred::Backend::Accounting::PaymentData data;
};

struct HasRegistrarsAndPayments
{
    explicit HasRegistrarsAndPayments(::LibFred::OperationContext& _ctx)
        : registrar(_ctx, "REG-TEST_A", "101"),
          other_registrar(_ctx, "REG-TEST_B", "202"),
          bank_account(_ctx, "0123456789", boost::optional<std::string>("0300")), // existing bank_code from enum_bank_code
          payment_data(
                  "11111111-1111-1111-1111-111111111111",
                  bank_account.account_number + "/" + bank_account.bank_code.get_value_or(""),
                  "unique account payment identifier",
                  "111111111/0111",
                  "",
                  "",
                  boost::lexical_cast<std::string>(registrar.data.variable_symbol),
                  "",
                  Money("100"),
                  boost::gregorian::from_string("2018-08-11"),
                  "",
                  boost::posix_time::time_from_string("2018-08-11 12:00:00.000000")),
          other_payment_data(
                  "22222222-2222-2222-2222-222222222222",
                  bank_account.account_number + "/" + bank_account.bank_code.get_value_or(""),
                  "unique account payment identifier",
                  "222222222/0222",
                  "",
                  "",
                  boost::lexical_cast<std::string>(other_registrar.data.variable_symbol),
                  "",
                  Money("200"),
                  boost::gregorian::from_string("2018-08-11"),
                  "",
                  boost::posix_time::time_from_string("2018-08-11 12:00:00.000000")),
          nonexistent_registrar_payment_data(
                  "22222222-2222-2222-2222-222222222222",
                  bank_account.account_number + "/" + bank_account.bank_code.get_value_or(""),
                  "unique account payment identifier",
                  "222222222/0222",
                  "",
                  "",
                  "",
                  "",
                  Money("200"),
                  boost::gregorian::from_string("2018-08-11"),
                  "",
                  boost::posix_time::time_from_string("2018-08-11 12:00:00.000000")),
          invalid_payment_data(
                  "33333333-3333-3333-3333-333333333333",
                  bank_account.account_number + "/" + bank_account.bank_code.get_value_or("") + "666",
                  "unique account payment identifier",
                  "333333333/0333",
                  "",
                  "",
                  boost::lexical_cast<std::string>(registrar.data.variable_symbol),
                  "",
                  Money("-300"), // only way to check invalid data exception at impl level and only for import payment
                  boost::gregorian::from_string("2018/08/11"),
                  "",
                  boost::posix_time::time_from_string("2018-08-11 12:00:00.000000")),
          nonexistent_registrar_handle("REG-NONEXISTENT")
    {
        BOOST_CHECK_EXCEPTION(
                ::LibFred::InfoRegistrarByHandle(nonexistent_registrar_handle).exec(_ctx),
                ::LibFred::InfoRegistrarByHandle::Exception,
                [](const ::LibFred::InfoRegistrarByHandle::Exception& e){ return e.is_set_unknown_registrar_handle(); });
    }

    Registrar registrar;
    Registrar other_registrar;
    // Zone zone;
    BankAccount bank_account;
    Payment payment_data;
    Payment other_payment_data;
    Payment nonexistent_registrar_payment_data;
    Payment invalid_payment_data;
    std::string nonexistent_registrar_handle;
};

} // namespace Test::Backend::Accounting
} // namespace Test::Backend
} // namespace Test

#endif
