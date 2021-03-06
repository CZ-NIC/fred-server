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
#ifndef GET_IFACE_OF_HH_972243A2C8E74C318556B97643052155
#define GET_IFACE_OF_HH_972243A2C8E74C318556B97643052155

#include "libfred/public_request/public_request_type_iface.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

template <typename T>
const LibFred::PublicRequestTypeIface& get_iface_of();

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
