/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "src/automatic_keyset_management/impl/util.hh"

#include <boost/lexical_cast.hpp>

#include <string>

namespace Fred {
namespace AutomaticKeysetManagement {
namespace Impl {


std::string quote(const std::string& str) {
    return "\"" + str + "\"";
}

std::string quote(int value) {
    return boost::lexical_cast<std::string>(value);
}

std::string to_string(const Fred::DnsKey& dnskey)
{
    static const std::string delim = ", ";
    return "["
           "flags: " + quote(dnskey.get_flags()) + delim +
           "protocol: " + quote(dnskey.get_protocol()) + delim +
           "algorithm: " + quote(dnskey.get_alg()) + delim +
           "key: " + quote(dnskey.get_key()) +
           "]";
}

} // namespace Fred::AutomaticKeysetManagement::Impl
} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred
