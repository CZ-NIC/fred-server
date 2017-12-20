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

#ifndef MESSAGE_TYPE_SET_H_0B4BA34DDD0A49A0BA25D11D84EF9EA1
#define MESSAGE_TYPE_SET_H_0B4BA34DDD0A49A0BA25D11D84EF9EA1

#include "src/libfred/poll/message_type.hh"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <set>

namespace Conversion {
namespace Enums {
namespace Sets {

template<typename T> inline std::set<typename T::Enum> from_config_string(const std::string &db_handle);

template<>
inline std::set<LibFred::Poll::MessageType::Enum> from_config_string<LibFred::Poll::MessageType>(
        const std::string &db_handle)
{
    std::set<LibFred::Poll::MessageType::Enum> ret;

    std::vector<std::string> message_type_list;
    boost::split(message_type_list, db_handle, boost::is_any_of(","));
    for (const auto& message_type: message_type_list)
    {
        ret.insert(from_db_handle<LibFred::Poll::MessageType>(message_type));
    }

    return ret;
}

} // namespace Conversion::Enums::Sets
} // namespace Conversion::Enums
} // namespace Conversion

#endif
