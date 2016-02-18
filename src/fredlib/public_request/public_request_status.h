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
*  header of Fred::PublicRequest::Status class
*/
#ifndef PUBLIC_REQUEST_STATUS_H_648D4833B94F11152913135FA0FE767E//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_STATUS_H_648D4833B94F11152913135FA0FE767E

#include "util/enum_conversion.h"

/// Fred matters
namespace Fred {
/// Fred public request matters
namespace PublicRequest {

/**
 * Bidirectional conversions between string and enum representation of public request status.
 */
struct Status
{
    /**
     * Names of particular public request status.
     */
    enum Value
    {
        NEW,
        ANSWERED,
        INVALIDATED,
    };
    /**
     * String value converts to its enum equivalent.
     * @param _str database representation of public request status
     * @return its enum equivalent
     * @throw std::runtime_error if conversion is impossible
     */
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into< Value >(_str);
    }
};

}//Fred::PublicRequest
}//Fred

namespace Conversion {
namespace Enums {

template < >
struct tools_for< Fred::PublicRequest::Status::Value >
{
    static void define_enum_to_string_relation(void (*set_matching_string_counterpart)(Fred::PublicRequest::Status::Value, const std::string&))
    {
        using Fred::PublicRequest::Status;
        set_matching_string_counterpart(Status::NEW,         "new");
        set_matching_string_counterpart(Status::ANSWERED,    "answered");
        set_matching_string_counterpart(Status::INVALIDATED, "invalidated");
    }
};

}//namespace Conversion::Enums
}//namespace Conversion

#endif//PUBLIC_REQUEST_STATUS_H_648D4833B94F11152913135FA0FE767E
