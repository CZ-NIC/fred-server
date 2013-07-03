/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file zone.h
 *  zone
 */

#ifndef ZONE_ZONE_H_
#define ZONE_ZONE_H_

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"

namespace Fred {
namespace Zone {

    ///zone data
    struct Data
    {
        unsigned long long id;
        bool is_enum;
        std::string name;

        Data()
        : id(0), is_enum(false) {}

        Data(unsigned long long _id, bool _is_enum, const std::string& _fqdn)
        : id(_id), is_enum(_is_enum), name(_fqdn){}
    };

    DECLARE_EXCEPTION_DATA(unknown_zone_in_fqdn, std::string);

    struct Exception
    : virtual Fred::OperationException
    , ExceptionData_unknown_zone_in_fqdn<Exception>
    {};

    ///look for zone in domain name
    Data find_zone_in_fqdn(OperationContext& ctx, const std::string& fqdn);

}//namespace Zone
}//namespace Fred

#endif //ZONE_ZONE_H_
