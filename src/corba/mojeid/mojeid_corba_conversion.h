/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  declaration for MojeID CORBA conversion
 */

#ifndef MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7
#define MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7

#include "util/corba_conversion.h"
#include "src/corba/MojeID2.hh"

namespace CorbaConversion
{

    template <> struct DEFAULT_UNWRAPPER<Registry::MojeID::NullableString_var, Nullable<std::string> >
    {
        typedef Unwrapper_NullableString_var_into_Nullable_std_string<Registry::MojeID::NullableString_var> type;
    };

    template <> struct DEFAULT_WRAPPER<Nullable<std::string>, Registry::MojeID::NullableString_var>
    {
        typedef Wrapper_Nullable_std_string_into_NullableString_var<Registry::MojeID::NullableString, Registry::MojeID::NullableString_var> type;
    };

}
#endif

