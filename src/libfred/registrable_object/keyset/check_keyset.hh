/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#ifndef CHECK_KEYSET_HH_F7E13C5423334FA691DE5A6B0A040BBD
#define CHECK_KEYSET_HH_F7E13C5423334FA691DE5A6B0A040BBD

#include "src/libfred/registrable_object/keyset/handle_state.hh"
#include "src/libfred/opcontext.hh"

#include <string>

namespace LibFred {
namespace Keyset {

HandleState::SyntaxValidity get_handle_syntax_validity(OperationContext &_ctx,
                                                       const std::string &_keyset_handle);

HandleState::Registrability get_handle_registrability(OperationContext &_ctx,
                                                      const std::string &_keyset_handle);

} // namespace LibFred::Keyset
} // namespace LibFred

#endif
