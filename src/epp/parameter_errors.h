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

#ifndef PARAMETER_ERRORS_D7D0DF2BB2FDF109C515EDC9D88A2767//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PARAMETER_ERRORS_D7D0DF2BB2FDF109C515EDC9D88A2767

#include "src/epp/param.h"
#include "src/epp/reason.h"

#include <map>
#include <set>

namespace Epp {

class ParameterErrors
{
public:
    bool is_empty()const;

    typedef std::set< unsigned short > Indexes;

    ParameterErrors& add_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason);
    bool has_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason)const;


    ParameterErrors& add_vector_parameter_error(Param::Enum _param, unsigned short _index, Reason::Enum _reason);
    bool has_vector_parameter_error(Param::Enum _param, Reason::Enum _reason)const;
    bool has_vector_parameter_error_at(Param::Enum _param, unsigned short _index, Reason::Enum _reason)const;
    const Indexes& get_vector_parameter_error(Param::Enum _param, Reason::Enum _reason)const;
private:
    class Reasons
    {
    public:
        Reasons& add_scalar_parameter_reason(Reason::Enum _reason);
        bool has_scalar_parameter_reason(Reason::Enum _reason)const;

        Reasons& add_vector_parameter_reason(Reason::Enum _reason, short unsigned _index);
        bool has_vector_parameter_reason(Reason::Enum _reason)const;
        bool has_vector_parameter_reason_at(Reason::Enum _reason, unsigned short _index)const;
        const Indexes& get_vector_parameter_reason(Reason::Enum _reason)const;
    private:
        typedef std::map< Reason::Enum, Indexes > ReasonAtPositions;
        ReasonAtPositions reason_at_positions_;
    };
    typedef std::map< Param::Enum, Reasons > ParamReasons;
    ParamReasons param_reasons_;
};

}

#endif//PARAMETER_ERRORS_D7D0DF2BB2FDF109C515EDC9D88A2767
