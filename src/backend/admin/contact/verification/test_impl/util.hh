/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  shared implementation for tests
 */

#ifndef UTIL_HH_ED5811D1A69D4FC2BE585F9F0414A7DF
#define UTIL_HH_ED5811D1A69D4FC2BE585F9F0414A7DF

#include "libfred/opcontext.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {
namespace Util {

/**
 * @param country_code 2 char uppercase country code
 * @returns English name of country specified by country_code
 * @throws std::runtime_error in case no country with country_code is found
 */

inline std::string get_country_name(
        LibFred::OperationContext& ctx,
        const std::string& country_code)
{
    try
    {
        Database::Result country_res = ctx.get_conn().exec_params(
                "SELECT country FROM enum_country WHERE id = $1::text ",
                Database::query_param_list(country_code));

        if (country_res.size() != 1)
        {
            throw 123;     /* goto catch */
        }

        return static_cast<std::string>(country_res[0]["country"]);

    }
    catch (...)
    {
        throw std::runtime_error("failed to get country name");
    }
}

} // namespace Fred::Backend::Admin::Contact::Verification::Util
} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
