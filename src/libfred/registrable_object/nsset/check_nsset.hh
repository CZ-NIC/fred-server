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

#ifndef CHECK_NSSET_HH_B06A1F479EB2476FA4B91EFD8DBBE353
#define CHECK_NSSET_HH_B06A1F479EB2476FA4B91EFD8DBBE353

#include "src/libfred/registrable_object/nsset/handle_state.hh"

#include <string>

#include "src/libfred/opcontext.hh"

namespace LibFred
{
namespace Nsset
{
    NssetHandleState::SyntaxValidity::Enum get_handle_syntax_validity(OperationContext& _ctx, const std::string& _nsset_handle);
    NssetHandleState::Registrability::Enum get_handle_registrability(OperationContext& _ctx, const std::string& _nsset_handle);
}
}
#endif
