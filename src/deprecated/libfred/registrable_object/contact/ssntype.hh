/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  header of ssn type enum
 */

#ifndef SSNTYPE_HH_C642E51AB57B4DCEAC42C3FF68BE7F36
#define SSNTYPE_HH_C642E51AB57B4DCEAC42C3FF68BE7F36

#include "util/enum_conversion.hh"

namespace LibFred {

struct SSNType
{
    enum Enum//enum_ssntype table
    {
        rc,
        op,
        pass,
        ico,
        mpsv,
        birthday
    };
};

} // namespace LibFred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(LibFred::SSNType::Enum value)
{
    switch (value)
    {
        case LibFred::SSNType::rc:       return "RC";
        case LibFred::SSNType::op:       return "OP";
        case LibFred::SSNType::pass:     return "PASS";
        case LibFred::SSNType::ico:      return "ICO";
        case LibFred::SSNType::mpsv:     return "MPSV";
        case LibFred::SSNType::birthday: return "BIRTHDAY";
    }
    throw std::invalid_argument("value doesn't exist in LibFred::SSNType::Enum");
}

template < >
inline LibFred::SSNType::Enum from_db_handle< LibFred::SSNType >(const std::string &db_handle)
{
    if (to_db_handle(LibFred::SSNType::rc) == db_handle) { return LibFred::SSNType::rc; }
    if (to_db_handle(LibFred::SSNType::op) == db_handle) { return LibFred::SSNType::op; }
    if (to_db_handle(LibFred::SSNType::pass) == db_handle) { return LibFred::SSNType::pass; }
    if (to_db_handle(LibFred::SSNType::ico) == db_handle) { return LibFred::SSNType::ico; }
    if (to_db_handle(LibFred::SSNType::mpsv) == db_handle) { return LibFred::SSNType::mpsv; }
    if (to_db_handle(LibFred::SSNType::birthday) == db_handle) { return LibFred::SSNType::birthday; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to LibFred::SSNType::Enum");
}

} // namespace Conversion::Enums
} // namespace Conversion

#endif
