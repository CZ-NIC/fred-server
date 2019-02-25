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
#ifndef GET_DEFAULT_DOCUMENT_MANAGER_HH_2CA35F58124F4C64AF71A45D2CAEA4FA
#define GET_DEFAULT_DOCUMENT_MANAGER_HH_2CA35F58124F4C64AF71A45D2CAEA4FA

#include "src/deprecated/libfred/documents.hh"

#include <memory>

namespace Fred {
namespace Backend {
namespace PublicRequest {

std::shared_ptr<LibFred::Document::Manager> get_default_document_manager();

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
