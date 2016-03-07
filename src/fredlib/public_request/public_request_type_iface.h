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
 *  declaration of PublicRequestTypeIface class
 */

#ifndef PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D

#include <string>
#include <memory>

namespace Fred {

/**
 * Common class for particular public request type.
 */
class PublicRequestTypeIface
{
public:
    /**
     * Convert public request type into string representation used in database.
     * @return string representation of this public request type
     */
    virtual std::string get_public_request_type()const = 0;
    /**
     * Instance pointer is publicly deletable.
     */
    virtual ~PublicRequestTypeIface() { }
};

}//namespace Fred

#endif//PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D
