/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef EPP_ERROR_9347842721087
#define EPP_ERROR_9347842721087

#include "src/epp/reason.h"
#include "src/epp/param.h"

#include <boost/utility.hpp>

namespace Epp {

    struct Error {
        Param::Enum param;
        unsigned short position;
        Reason::Enum reason;

        Error(
            Param::Enum _param,
            unsigned short _position,
            Reason::Enum _reason
        ) :
            param(_param),
            position(_position),
            reason(_reason)
        { }
    };
}

/* Only intended for std::set usage - ordering definition is irrelevant. */

namespace std {
    template<> struct less<Epp::Error> {
        bool operator()(const Epp::Error& lhs, const Epp::Error& rhs) const {
            /* order by param, position, reason */

            if(      static_cast<int>(lhs.param) < static_cast<int>(rhs.param) ) { return true; }
            else if( static_cast<int>(lhs.param) > static_cast<int>(rhs.param) ) { return false; }
            else {

                if(      lhs.position < rhs.position ) { return true; }
                else if( lhs.position > rhs.position ) { return false; }
                else {

                    if( static_cast<int>(lhs.reason) < static_cast<int>(rhs.reason) ) { return true; }
                    else { return false; }
                }
            }

        }
    };
};

#endif
