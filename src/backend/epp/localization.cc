/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/localization.hh"

#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/util.hh"
#include "src/backend/epp/session_lang.hh"
#include "libfred/db_settings.hh"

#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <stdexcept>
#include <vector>

namespace Epp {

std::string get_reason_description_localized_column_name(SessionLang::Enum _lang)
{
    switch (_lang)
    {
        case SessionLang::en:
            return "reason";

        case SessionLang::cs:
            return "reason_cs";
    }
    throw UnknownLocalizationLanguage();
}

std::string get_result_description_localized_column_name(SessionLang::Enum _lang)
{
    switch (_lang)
    {
        case SessionLang::en:
            return "status";

        case SessionLang::cs:
            return "status_cs";
    }
    throw UnknownLocalizationLanguage();
}

std::string get_reason_description_localized(
    LibFred::OperationContext& _ctx,
    Reason::Enum _reason,
    SessionLang::Enum _lang)
{
    const std::string column_name = get_reason_description_localized_column_name(_lang);

    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT " + column_name + " "
        "FROM enum_reason "
        "WHERE id=$1::INTEGER",
        Database::query_param_list(to_description_db_id(_reason)));

    if (res.size() < 1)
    {
        throw MissingLocalizedDescription();
    }

    if (1 < res.size())
    {
        throw std::runtime_error("0 or 1 row expected");
    }

    return static_cast<std::string>(res[0][0]);
}

/**
 * @returns untyped postgres array
 * Caller should cast it properly in query.
 */
std::string convert_values_to_pg_array(const std::set<unsigned>& _input)
{
    std::vector<std::string> string_values;
    string_values.reserve(_input.size());

    for (const auto elem : _input)
    {
        string_values.push_back(boost::lexical_cast<std::string>(elem));
    }

    return "{" + boost::algorithm::join(string_values, ",") + "}";
}

}//namespace Epp
