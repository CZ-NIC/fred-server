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

#ifndef AUTHORIZATION_REQUIRED_HH_61B14361C45AD2671DB1ECC88BC2AAFA//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define AUTHORIZATION_REQUIRED_HH_61B14361C45AD2671DB1ECC88BC2AAFA

#include "src/backend/epp/password.hh"

#include "libfred/opcontext.hh"
#include "libfred/object/types.hh"

namespace Epp {

void authorization_required(
        LibFred::OperationContext& ctx,
        LibFred::Object::ObjectId object_id,
        const Password& password);

} // namespace Epp

#endif//AUTHORIZATION_REQUIRED_HH_61B14361C45AD2671DB1ECC88BC2AAFA
