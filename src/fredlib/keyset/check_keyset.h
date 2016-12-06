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

#ifndef CHECK_KEYSET_H_2978916EE20C45F98D9C16CA2D424FE4
#define CHECK_KEYSET_H_2978916EE20C45F98D9C16CA2D424FE4

#include "src/fredlib/keyset/handle_state.h"
#include "src/fredlib/opcontext.h"

#include <string>

namespace Fred {
namespace Keyset {

HandleState::SyntaxValidity get_handle_syntax_validity(const std::string &_keyset_handle);

HandleState::Registrability get_handle_registrability(OperationContext &_ctx,
                                                      const std::string &_keyset_handle);

}//namespace Fred::Keyset
}//namespace Fred

#endif
