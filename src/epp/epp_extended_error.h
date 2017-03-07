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

#ifndef EPP_EXTENDED_ERROR_H_99879BF0F9DD4F748F8180F415BB299E
#define EPP_EXTENDED_ERROR_H_99879BF0F9DD4F748F8180F415BB299E

#include "src/epp/param.h"
#include "src/epp/reason.h"

namespace Epp {

/**
 * EppExtendedError represents STD 69 EPP error response result's extValue.
 */
class EppExtendedError
{

public:
    static EppExtendedError of_scalar_parameter(
            Param::Enum _param,
            Reason::Enum _reason)
    {
        return EppExtendedError(_param, position_of_scalar_parameter, _reason);
    }


    static EppExtendedError of_vector_parameter(
            Param::Enum _param,
            unsigned short _index,
            Reason::Enum _reason)
    {
        return EppExtendedError(_param, position_of_first_element_of_vector_parameter + _index, _reason);
    }


    Param::Enum param() const
    {
        return param_;
    }


    unsigned short position() const
    {
        return position_;
    }


    Reason::Enum reason() const
    {
        return reason_;
    }


    // only intended for std::set usage - ordering definition is irrelevant
    friend bool operator <(
            const Epp::EppExtendedError& lhs,
            const Epp::EppExtendedError& rhs);


private:
    EppExtendedError(
            Param::Enum _param,
            unsigned short _position,
            Reason::Enum _reason)
        : param_(_param),
          position_(_position),
          reason_(_reason)
    {
    }


    Param::Enum param_; ///< represents STD 69 response result extValue's value
    unsigned short position_;
    Reason::Enum reason_; ///< represents STD 69 response result extValue's reason

    static const unsigned short position_of_scalar_parameter = 0;
    static const unsigned short position_of_first_element_of_vector_parameter = 1;
};

} // namespace Epp

#endif
