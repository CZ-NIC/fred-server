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

struct SSNType
{
    enum Enum//enum_ssntype table
    {
        RC,
        OP,
        PASS,
        ICO,
        MPSV,
        BIRTHDAY
    };
};

}//namespace Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Fred::SSNType::Enum value)
{
    switch (value)
    {
        case Fred::SSNType::RC:       return "RC";
        case Fred::SSNType::OP:       return "OP";
        case Fred::SSNType::PASS:     return "PASS";
        case Fred::SSNType::ICO:      return "ICO";
        case Fred::SSNType::MPSV:     return "MPSV";
        case Fred::SSNType::BIRTHDAY: return "BIRTHDAY";
    }
    throw std::invalid_argument("value doesn't exist in Fred::SSNType::Enum");
}

template < >
inline Fred::SSNType::Enum from_db_handle< Fred::SSNType >(const std::string &db_handle)
{
    if (to_db_handle(Fred::SSNType::RC) == db_handle) { return Fred::SSNType::RC; }
    if (to_db_handle(Fred::SSNType::OP) == db_handle) { return Fred::SSNType::OP; }
    if (to_db_handle(Fred::SSNType::PASS) == db_handle) { return Fred::SSNType::PASS; }
    if (to_db_handle(Fred::SSNType::ICO) == db_handle) { return Fred::SSNType::ICO; }
    if (to_db_handle(Fred::SSNType::MPSV) == db_handle) { return Fred::SSNType::MPSV; }
    if (to_db_handle(Fred::SSNType::BIRTHDAY) == db_handle) { return Fred::SSNType::BIRTHDAY; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::SSNType::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//SSNTYPE_H_4E9ED0E9B9EB357A6C4A3A774E87A1FE
