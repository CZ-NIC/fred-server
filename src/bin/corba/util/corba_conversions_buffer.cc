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
#include "src/bin/corba/util/corba_conversions_buffer.hh"

#include <cstring>
#include <stdexcept>

namespace CorbaConversion {
namespace Util {

Registry::Buffer_var wrap_Buffer(const Fred::Backend::Buffer& _src)
{
    Registry::Buffer_var result(new Registry::Buffer());
    try
    {
        result->data.length(_src.data.size());
        if (!_src.data.empty())
        {
            std::memcpy(result->data.get_buffer(), _src.data.c_str(), _src.data.size());
        }
    }
    catch (...)
    {
        throw std::runtime_error("memory allocation failed");
    }
    return result._retn();
}

} // namespace CorbaConversion::Util
} // namespace CorbaConversion
