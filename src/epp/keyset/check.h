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

#ifndef CHECK_H_62795D72C8FB7ACA38CE9C700EB7B637//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CHECK_H_62795D72C8FB7ACA38CE9C700EB7B637

#include "src/epp/keyset/handle_check_result.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <map>
#include <set>
#include <string>

namespace Epp {

/**
 * @returns check results for given contact handles
 */
std::map< std::string, Nullable< Keyset::HandleCheckResult::Enum > > keyset_check(
    Fred::OperationContext& _ctx,
    const std::set< std::string > &_keyset_handles);

}

#endif//CHECK_H_62795D72C8FB7ACA38CE9C700EB7B637
