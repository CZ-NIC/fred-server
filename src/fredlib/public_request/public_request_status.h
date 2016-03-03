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
    enum Enum//enum_public_request_status table
    {
        NEW,
        ANSWERED,
        INVALIDATED,
    };
};

}//Fred::PublicRequest
}//Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Fred::PublicRequest::Status::Enum value)
{
    switch (value)
    {
        case Fred::PublicRequest::Status::NEW:         return "new";
        case Fred::PublicRequest::Status::ANSWERED:    return "answered";
        case Fred::PublicRequest::Status::INVALIDATED: return "invalidated";
    }
    throw std::invalid_argument("value doesn't exist in Fred::PublicRequest::Status::Enum");
}

template < >
inline Fred::PublicRequest::Status::Enum from_db_handle< Fred::PublicRequest::Status >(const std::string &db_handle)
{
    static const Fred::PublicRequest::Status::Enum values[] =
    {
        Fred::PublicRequest::Status::NEW,
        Fred::PublicRequest::Status::ANSWERED,
        Fred::PublicRequest::Status::INVALIDATED
    };
    return from_db_handle_impl(db_handle, values, "Fred::PublicRequest::Status::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//PUBLIC_REQUEST_STATUS_H_648D4833B94F11152913135FA0FE767E
