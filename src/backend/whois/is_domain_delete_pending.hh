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
#ifndef IS_DOMAIN_DELETE_PENDING_HH_AA08E1F658E14E74BE2C58B65ED936F7
#define IS_DOMAIN_DELETE_PENDING_HH_AA08E1F658E14E74BE2C58B65ED936F7

#include "libfred/opcontext.hh"
#include "src/deprecated/libfred/registrable_object/domain.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace Whois {

bool is_domain_delete_pending(const std::string& _fqdn, LibFred::OperationContext& _ctx, const std::string& _timezone);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
