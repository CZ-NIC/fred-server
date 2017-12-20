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

#include "src/libfred/opexception.hh"
#include "src/libfred/opcontext.hh"

namespace LibFred {
namespace Zone {

    ///zone data
    struct Data
    {
        unsigned long long id;
        bool is_enum;
        std::string name;
        unsigned dots_max;
        unsigned ex_period_min;
        unsigned ex_period_max;
        unsigned enum_validation_period;

        Data()
        : id(0), is_enum(false), dots_max(0),
          ex_period_min(0), ex_period_max(0),
          enum_validation_period(0)
        {}

        Data(unsigned long long _id, bool _is_enum, const std::string& _fqdn,
            unsigned _dots_max, unsigned _ex_period_min, unsigned _ex_period_max,
            unsigned _enum_validation_period)
        : id(_id), is_enum(_is_enum), name(_fqdn), dots_max(_dots_max),
          ex_period_min(_ex_period_min), ex_period_max(_ex_period_max),
          enum_validation_period(_enum_validation_period)
        {}
    };

    DECLARE_EXCEPTION_DATA(unknown_zone_in_fqdn, std::string);

    struct Exception
    : virtual LibFred::OperationException
    , ExceptionData_unknown_zone_in_fqdn<Exception>
    {};

    ///look for zone in domain name and return zone data
    Data find_zone_in_fqdn(OperationContext& ctx, const std::string& fqdn);
    ///lock zone for share and get zone data
    Data get_zone(OperationContext& ctx, const std::string& zone_name);

    /**
     * Remove optional root dot from fqdn. Domain names are considered fully qualified without trailing dot internally.
     * Optional dot is not part of preferred name syntax (RFC1035 section 2.3.1.), but must be accepted by applications
     * according to RFC3696 section 2.
     */
    std::string rem_trailing_dot(const std::string& fqdn);

} // namespace Zone
} // namespace LibFred

#endif //ZONE_ZONE_H_
