/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#ifndef PAYMENT_DATA_HH_EEA105A1639F43E79CEFCF87DCBC8AA1
#define PAYMENT_DATA_HH_EEA105A1639F43E79CEFCF87DCBC8AA1

#include "src/deprecated/libfred/exceptions.hh"
#include "src/deprecated/libfred/common_new.hh"
#include "src/util/types/money.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>

#include <string>

namespace LibFred {
namespace Banking {

struct PaymentData
{
    PaymentData(
            const std::string& _uuid,
            const unsigned long long _account_id,
            const std::string& _account_payment_ident,
            const std::string& _counter_account_number,
            const boost::optional<std::string>& _counter_account_bank_code,
            const std::string& _counter_account_name,
            const std::string& _constant_symbol,
            const std::string& _variable_symbol,
            const std::string& _specific_symbol,
            const Money& _price,
            const boost::gregorian::date& _date,
            const std::string& _memo,
            const boost::posix_time::ptime _creation_time)
        : uuid(_uuid),
          account_id(_account_id),
          account_payment_ident(_account_payment_ident),
          counter_account_number(_counter_account_number),
          counter_account_bank_code(_counter_account_bank_code),
          counter_account_name(_counter_account_name),
          constant_symbol(_constant_symbol),
          variable_symbol(_variable_symbol),
          specific_symbol(_specific_symbol),
          price(_price),
          date(_date),
          memo(_memo),
          creation_time(_creation_time)
    {
    }

    std::string uuid;
    unsigned long long account_id;
    std::string account_payment_ident;
    std::string counter_account_number;
    boost::optional<std::string> counter_account_bank_code;
    std::string counter_account_name;
    std::string constant_symbol;
    std::string variable_symbol;
    std::string specific_symbol;
    Money price;
    boost::gregorian::date date;
    std::string memo;
    boost::posix_time::ptime creation_time;
};

} // namespace Banking
} // namespace LibFred

#endif
