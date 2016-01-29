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
 *  header of ssn type enum
 */

#ifndef SSNTYPE_H_4E9ED0E9B9EB357A6C4A3A774E87A1FE//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define SSNTYPE_H_4E9ED0E9B9EB357A6C4A3A774E87A1FE

#include "util/enum_conversion.h"

namespace Fred {

class SSNType
{
public:
    enum Value
    {
        RC,
        OP,
        PASS,
        ICO,
        MPSV,
        BIRTHDAY
    };
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into_from< Value >::into_enum_from(_str);
    }
};

}//namespace Fred

namespace Conversion {
namespace Enums {

template < >
struct tools_for< Fred::SSNType::Value >
{
    static void enum_to_other_init(void (*set_relation)(Fred::SSNType::Value, const std::string&))
    {
        using Fred::SSNType;
        set_relation(SSNType::RC,       "RC");
        set_relation(SSNType::OP,       "OP");
        set_relation(SSNType::PASS,     "PASS");
        set_relation(SSNType::ICO,      "ICO");
        set_relation(SSNType::MPSV,     "MPSV");
        set_relation(SSNType::BIRTHDAY, "BIRTHDAY");
    }
};

}//namespace Conversion::Enums
}//namespace Conversion

#endif//SSNTYPE_H_4E9ED0E9B9EB357A6C4A3A774E87A1FE
