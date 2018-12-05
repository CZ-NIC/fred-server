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

#ifndef GET_UPDATE_CONTACT_CHECK_HH_5A28C15EE725DB000D21BC9CB3A1542E//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define GET_UPDATE_CONTACT_CHECK_HH_5A28C15EE725DB000D21BC9CB3A1542E

#include "src/backend/epp/contact/update_operation_check.hh"
#include "src/backend/epp/contact/config_check.hh"

#include <memory>
#include <string>

namespace Epp {
namespace Contact {
namespace Impl {

std::shared_ptr<Epp::Contact::UpdateContactDataFilter> get_update_contact_data_filter(const ConfigDataFilter& filter);

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//GET_UPDATE_CONTACT_CHECK_HH_5A28C15EE725DB000D21BC9CB3A1542E
