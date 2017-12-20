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

#ifndef FREDLIB_NOTIFIER2_UTIL_STRING_LIST_UTILS_H_5265405415
#define FREDLIB_NOTIFIER2_UTIL_STRING_LIST_UTILS_H_5265405415

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <boost/asio/ip/address.hpp>
#include "src/libfred/object/object_id_handle_pair.hh"

#include <boost/foreach.hpp>

namespace Notification {

    /**
     * @returns vector of handles from given id, handle pairs
     * - could be potentialy non-unique despite using std::set on input because LibFred::ObjectIdHandlePair compares both handle AND id
     */
    inline std::vector<std::string> get_handles(const std::vector<LibFred::ObjectIdHandlePair>& _in) {
        std::vector<std::string> result;

        BOOST_FOREACH(const LibFred::ObjectIdHandlePair& ob, _in) {
            result.push_back(ob.handle);
        }

        return result;
    }

    /**
     * @returns vector of handles from given id, handle pairs
     * - could be potentialy non-unique despite using std::set on input because LibFred::ObjectIdHandlePair compares both handle AND id
     */
    inline std::vector<std::string> get_string_addresses(const std::vector<boost::asio::ip::address>& _in) {
        std::vector<std::string> result;

        BOOST_FOREACH(const boost::asio::ip::address& ob, _in) {
            result.push_back(ob.to_string());
        }

        return result;
    }

    /**
     * interface adapter for std::sort
     */
    inline std::vector<std::string> sort(const std::vector<std::string>& _in) {
        std::vector<std::string> result = _in;
        std::sort(result.begin(), result.end());

        return result;
    }
}

#endif
