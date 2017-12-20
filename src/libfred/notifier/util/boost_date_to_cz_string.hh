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

#ifndef FREDLIB_NOTIFIER2_UTIL_BOOST_DATE_TO_CZ_STRING_H_933180698734
#define FREDLIB_NOTIFIER2_UTIL_BOOST_DATE_TO_CZ_STRING_H_933180698734

#include <string>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/lexical_cast.hpp>

namespace Notification {

    /**
     * @returns DD. MM. YYYY. or empty string in case _input is_special()
     */
    std::string to_cz_format(const boost::gregorian::date& _input) {
        // XXX I know. Ugly as hell. But still better than official way with dynamicaly allocated facet.
        return
            _input.is_special()
                ? ""
                :
                    boost::lexical_cast<std::string>( _input.day().as_number() )
                    + "." +
                    boost::lexical_cast<std::string>( _input.month().as_number() )
                    + "." +
                    boost::lexical_cast<std::string>( _input.year() );
    }
}

#endif
