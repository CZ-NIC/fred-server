/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 */

#ifndef UTIL_H_1EFC4C4310E84E82A77F0ABA8DB8B97F
#define UTIL_H_1EFC4C4310E84E82A77F0ABA8DB8B97F

#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/opcontext.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <set>
#include <vector>

// TODO fredlib_modification mozna by vsechno tohle casem melo byt soucasti fredlibu
namespace Epp {

inline bool is_country_code_valid(Fred::OperationContext& ctx, const std::string& cc) {
    return
        ctx.get_conn().exec_params(
                "SELECT 1 FROM enum_country WHERE id = $1::text FOR SHARE ",
                Database::query_param_list(cc)
                ).size() > 0;
}

std::string get_db_table_enum_reason_column_name_for_language(SessionLang::Enum _lang);

std::string convert_values_to_pg_array(const std::set<unsigned>& _input);

template <typename C>
std::map<typename C::Enum, std::string> get_reasons_descriptions(const Fred::OperationContext& _ctx, const std::set<unsigned>& _reasons_ids, const SessionLang::Enum _lang) {
    std::map<typename C::Enum, std::string> result;
    if(!_reasons_ids.empty()) {
        const std::string column_name = get_db_table_enum_reason_column_name_for_language(_lang);
        const Database::Result db_result = _ctx.get_conn().exec_params(
                "SELECT "
                    "id AS reason_id, " +
                    column_name + " AS reason_description "
                "FROM enum_reason "
                "WHERE id = ANY( $1::integer[] ) ",
                Database::query_param_list(
                    convert_values_to_pg_array(_reasons_ids)
                ));

        for (::size_t i = 0; i < db_result.size(); ++i) {
            const typename C::Enum reason = C::from_reason(from_description_db_id<Reason>(static_cast<unsigned>(db_result[i]["reason_id"])));
            const std::string description = static_cast<std::string>(db_result[i]["reason_description"]);
            result[reason] = description;
        }
    }
    return result;
}


}

#endif
