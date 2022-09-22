/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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
#ifndef REGISTRAR_HH_20BC54842441606AABF1664FDAA675CE//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define REGISTRAR_HH_20BC54842441606AABF1664FDAA675CE

#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar_data.hh"
#include "libfred/opcontext.hh"

#include <string>


namespace Test {

struct Registrar
{
    explicit Registrar(LibFred::OperationContext& ctx, LibFred::CreateRegistrar create);
    LibFred::InfoRegistrarData data;
};

struct SystemRegistrar
{
    explicit SystemRegistrar(LibFred::OperationContext& ctx, LibFred::CreateRegistrar create);
    LibFred::InfoRegistrarData data;
};

namespace Setter {

LibFred::CreateRegistrar registrar(LibFred::CreateRegistrar create, int index = 0);
LibFred::CreateRegistrar system_registrar(LibFred::CreateRegistrar create, int index = 0);

}//namespace Test::Setter

}//namespace Test

#endif//REGISTRAR_HH_20BC54842441606AABF1664FDAA675CE
