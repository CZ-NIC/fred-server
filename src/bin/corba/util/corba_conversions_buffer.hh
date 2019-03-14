/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef CORBA_CONVERSIONS_BUFFER_HH_564E460334484E28B112CEB998F31F83
#define CORBA_CONVERSIONS_BUFFER_HH_564E460334484E28B112CEB998F31F83

#include "src/backend/buffer.hh"
#include "src/bin/corba/Buffer.hh"

namespace CorbaConversion {
namespace Util {

Registry::Buffer_var wrap_Buffer(const Fred::Backend::Buffer& _src);

} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
