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

#ifndef PARAMETER_ERRORS_D7D0DF2BB2FDF109C515EDC9D88A2767//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PARAMETER_ERRORS_D7D0DF2BB2FDF109C515EDC9D88A2767

#include "src/epp/error.h"

#include <map>
#include <set>

namespace Epp {

class ParameterErrors
{
public:
    bool is_empty()const;

    struct Where
    {
        typedef std::set< unsigned short > Indexes;
        bool is_scalar()const { return indexes.empty(); }
        bool is_vector()const { return !indexes.empty(); }
        bool has_element(unsigned short _index)const { return 0 < indexes.count(_index); }
        Where& add_element(unsigned short _index);
        Indexes indexes;
    };

    ParameterErrors& add_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason);
    bool has_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason)const;

    ParameterErrors& add_vector_parameter_error(Param::Enum _param, unsigned short _index, Reason::Enum _reason);
    bool has_vector_parameter_error(Param::Enum _param, Reason::Enum _reason)const;
    bool has_vector_parameter_error_at(Param::Enum _param, unsigned short _index, Reason::Enum _reason)const;
    const Where& get_vector_parameter_error(Param::Enum _param, Reason::Enum _reason)const;
    std::set< Error > get_set_of_error()const;
private:
    class What
    {
    public:
        What(Param::Enum, Reason::Enum);
        bool operator<(const What&)const;
        Param::Enum get_param()const;
        Reason::Enum get_reason()const;
    private:
        Param::Enum param_;
        Reason::Enum reason_;
    };
    typedef std::map< What, Where > WhatWhere;
    WhatWhere what_where_;
};

}

#endif//PARAMETER_ERRORS_D7D0DF2BB2FDF109C515EDC9D88A2767
