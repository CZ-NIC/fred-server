/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#ifndef GET_UPDATE_CONTACT_DATA_FILTER_HH_5A28C15EE725DB000D21BC9CB3A1542E//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define GET_UPDATE_CONTACT_DATA_FILTER_HH_5A28C15EE725DB000D21BC9CB3A1542E

#include "src/backend/epp/contact/update_contact_data_filter.hh"
#include "src/backend/epp/contact/config_data_filter.hh"

#include <memory>
#include <string>

namespace Epp {
namespace Contact {
namespace Impl {

std::shared_ptr<Epp::Contact::UpdateContactDataFilter> get_update_contact_data_filter(const ConfigDataFilter& filter);

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//GET_UPDATE_CONTACT_DATA_FILTER_HH_5A28C15EE725DB000D21BC9CB3A1542E
