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
/**
 *  @file
 */

#ifndef UTIL_HH_1946C028CFD1459D9A3D10E559FDC6CE
#define UTIL_HH_1946C028CFD1459D9A3D10E559FDC6CE

#include "libfred/opcontext.hh"
#include "test/setup/fixtures_utils.hh"

#include <utility>
#include <string>
#include <map>
#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

struct has_autocomitting_ctx : Test::instantiate_db_template {
    ::LibFred::OperationContextCreator ctx;

    has_autocomitting_ctx() {
        ctx.get_conn().exec("TRUNCATE notification_queue");
    }

    ~has_autocomitting_ctx() {
        ctx.commit_transaction();
    }
};

/**
 * check given std::maps are equal with reasonable debug output
 */
inline void check_maps_are_equal(const std::map<std::string, std::string>& _lhs, const std::map<std::string, std::string>& _rhs) {
    // proper comparison
    BOOST_CHECK(_lhs ==  _rhs);

    std::set<std::string> key_values;

    for(
        std::map<std::string, std::string>::const_iterator it = _lhs.begin();
        it != _lhs.end();
        ++it
    ) {
        key_values.insert(it->first);
    }

    for(
        std::map<std::string, std::string>::const_iterator it = _rhs.begin();
        it != _rhs.end();
        ++it
    ) {
        key_values.insert(it->first);
    }

    // debug print
    BOOST_FOREACH(const std::string& key, key_values) {

        std::map<std::string, std::string>::const_iterator etalon_it = _lhs.find(key);
        std::map<std::string, std::string>::const_iterator result_it = _rhs.find(key);

        BOOST_CHECK_MESSAGE(
            /* beware of possible end() dereferrencing */
            etalon_it != _lhs.end() && result_it != _rhs.end() && etalon_it->second == result_it->second,
            key + " : " + ( etalon_it != _lhs.end() ? etalon_it->second : std::string("[undefined]") )
            + " != " +
            ( result_it != _rhs.end() ? result_it->second : std::string("[undefined]") )
        );

        ++etalon_it;
        ++result_it;
    }
}

/** make object in registry look older */
inline void make_history_version_begin_older(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _historyid,
    unsigned _move_years_to_history,
    bool _move_crdate
) {
    _ctx.get_conn().exec_params(
        std::string(
            _move_crdate
            ?   "WITH historyid_ AS( "
                    "UPDATE object_registry "
                    "SET crdate = crdate - interval '" + boost::lexical_cast<std::string>(_move_years_to_history) + " year' "
                    "WHERE crid = $1::INT "
                ") "
            :   ""
        ) +
        "UPDATE history "
        "SET "
            "valid_from = valid_from - interval '"+ boost::lexical_cast<std::string>(_move_years_to_history) +" year' "
        "WHERE id = $1::INT ",
        Database::query_param_list(_historyid)
    );
}

/** make object in registry look older */
inline void make_history_version_end_older(
    ::LibFred::OperationContext& _ctx,
    unsigned long long _historyid,
    unsigned _move_years_to_history
) {
    _ctx.get_conn().exec_params(
        "UPDATE history "
        "SET "
            "valid_to   = valid_to   - interval '"+ boost::lexical_cast<std::string>(_move_years_to_history) +" year' "
        "WHERE id = $1::INT ",
        Database::query_param_list(_historyid)
    );
}

#endif
