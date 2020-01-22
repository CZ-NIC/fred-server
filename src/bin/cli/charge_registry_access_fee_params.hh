/*
 * Copyright (C) 2019-2020  CZ.NIC, z. s. p. o.
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

#ifndef CHARGE_REGISTRY_ACCESS_FEE_PARAMS_HH_292A8AE6AB364B19BFD622CDE0ED1DDB
#define CHARGE_REGISTRY_ACCESS_FEE_PARAMS_HH_292A8AE6AB364B19BFD622CDE0ED1DDB

#include "src/util/types/optional.hh"

#include <boost/optional.hpp>

#include <vector>

struct ChargeRegistryAccessFeeAnnualArgs
{
    std::vector<std::string> registrars;
    std::vector<std::string> except_registrars;
    bool all_registrars;
    std::string zone;
    std::string year;
    bool charge_to_end_of_previous_month;

    ChargeRegistryAccessFeeAnnualArgs() = default;

    ChargeRegistryAccessFeeAnnualArgs(
            const std::vector<std::string>& _registrars,
            const std::vector<std::string>& _except_registrars,
            const bool _all_registrars,
            const std::string& _zone,
            const std::string& _year,
            const bool _charge_to_end_of_previous_month
            ) :
        registrars(_registrars),
        except_registrars(_except_registrars),
        all_registrars(_all_registrars),
        zone(_zone),
        year(_year),
        charge_to_end_of_previous_month(_charge_to_end_of_previous_month)
    {
    }
};

struct ChargeRegistryAccessFeeMonthlyArgs
{
    std::vector<std::string> registrars;
    std::vector<std::string> except_registrars;
    bool all_registrars;
    std::string zone;
    std::string year_month;
    bool charge_to_end_of_previous_month;

    ChargeRegistryAccessFeeMonthlyArgs() = default;

    ChargeRegistryAccessFeeMonthlyArgs(
            const std::vector<std::string>& _registrars,
            const std::vector<std::string>& _except_registrars,
            const bool _all_registrars,
            const std::string& _zone,
            const std::string& _year_month,
            const bool _charge_to_end_of_previous_month
            ) :
        registrars(_registrars),
        except_registrars(_except_registrars),
        all_registrars(_all_registrars),
        zone(_zone),
        year_month(_year_month),
        charge_to_end_of_previous_month(_charge_to_end_of_previous_month)
    {
    }
};

#endif
