/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef UTIL_HH_D98CD47F7B6A47699889AD58CD5A631A
#define UTIL_HH_D98CD47F7B6A47699889AD58CD5A631A

#include "libfred/opcontext.hh"

#include <boost/optional.hpp>
#include <string>

namespace Test {

unsigned long long get_epp_auth_id(::LibFred::OperationContext& _ctx,
        const std::string& _registrar_handle,
        const std::string& _certificate_fingerprint,
        const boost::optional<std::string>& _plain_password);

unsigned long long add_epp_authentications(::LibFred::OperationContext& _ctx,
        const std::string& _registrar_handle,
        const std::string& _certificate_fingerprint,
        const std::string& _plain_password);

} //namespace Test

#endif
