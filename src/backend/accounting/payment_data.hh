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
#ifndef PAYMENT_DATA_HH_AAD16DFCDC514E7FAFAC28DE9BDD7E87
#define PAYMENT_DATA_HH_AAD16DFCDC514E7FAFAC28DE9BDD7E87

#include "src/util/types/money.hh"

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <string>

namespace Fred {
namespace Backend {
namespace Accounting {

struct PaymentData
{
    std::string uuid;
    std::string account_number;
    std::string account_payment_ident;
    std::string counter_account_number;
    std::string counter_account_name;
    std::string constant_symbol;
    std::string variable_symbol;
    std::string specific_symbol;
    Money price;
    boost::gregorian::date date;
    std::string memo;
    boost::posix_time::ptime creation_time;
};

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
