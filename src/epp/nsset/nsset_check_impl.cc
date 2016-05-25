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

#include "src/epp/nsset/nsset_check_impl.h"

#include "src/fredlib/nsset/check_nsset.h"
#include "src/epp/nsset/nsset_handle_state_to_check_result.h"

#include <boost/foreach.hpp>

namespace Epp {


std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > nsset_check_impl(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _nsset_handles
) {
    std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > result;

    BOOST_FOREACH(const std::string& handle, _nsset_handles) {
        result[handle] = nsset_handle_state_to_check_result(
            Fred::Nsset::get_handle_syntax_validity(handle),
            Fred::Nsset::get_handle_registrability(_ctx, handle)
        );
    }

    return result;
}

}
