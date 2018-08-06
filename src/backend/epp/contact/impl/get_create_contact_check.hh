/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef GET_CREATE_CONTACT_CHECK_HH_DD99A7E34F2E2A4E57B0B1035DCA8A41//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define GET_CREATE_CONTACT_CHECK_HH_DD99A7E34F2E2A4E57B0B1035DCA8A41

#include "src/backend/epp/contact/create_operation_check.hh"
#include "src/util/cfg/handle_rifd_args.hh"

#include <memory>
#include <string>

namespace Epp {
namespace Contact {
namespace Impl {

std::shared_ptr<Epp::Contact::CreateOperationCheck> get_create_contact_check(const HandleRifdArgs::Check& check);

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//GET_CREATE_CONTACT_CHECK_HH_DD99A7E34F2E2A4E57B0B1035DCA8A41
