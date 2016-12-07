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

#include "src/epp/impl/reason.h"
#include "src/epp/impl/param.h"

namespace Epp {

class Error
{
public:
    static Error of_scalar_parameter(Param::Enum _param, Reason::Enum _reason)
    {
        return Error(_param, position_of_scalar_parameter, _reason);
    }
    static Error of_vector_parameter(Param::Enum _param, unsigned short _index, Reason::Enum _reason)
    {
        return Error(_param, position_of_first_element_of_vector_parameter + _index, _reason);
    }
    Param::Enum param;
    unsigned short position;
    Reason::Enum reason;
    //Only intended for std::set usage - ordering definition is irrelevant.
    bool operator<(const Error &_b)const
    {
        const Error &_a = *this;
        return (_a.param < _b.param) ||
               ((_a.param == _b.param) && (_a.position < _b.position)) ||
               ((_a.param == _b.param) && (_a.position == _b.position) && (_a.reason < _b.reason));
    }
private:
    Error(Param::Enum _param, unsigned short _position, Reason::Enum _reason)
    :   param(_param),
        position(_position),
        reason(_reason)
    { }

    static const unsigned short position_of_scalar_parameter = 0;
    static const unsigned short position_of_first_element_of_vector_parameter = 1;
};

}//namespace Epp

#endif
