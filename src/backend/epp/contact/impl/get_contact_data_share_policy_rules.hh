/*
 * Copyright (C) 2021  CZ.NIC, z. s. p. o.
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
#ifndef GET_CONTACT_DATA_SHARE_POLICY_RULES_HH_8DFB0156981D0A832B7D4CA0020FA0CB//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define GET_CONTACT_DATA_SHARE_POLICY_RULES_HH_8DFB0156981D0A832B7D4CA0020FA0CB

#include "src/backend/epp/contact/contact_data_share_policy_rules.hh"
#include "src/backend/epp/contact/config_data_filter.hh"

#include <memory>

namespace Epp {
namespace Contact {
namespace Impl {

std::shared_ptr<Epp::Contact::ContactDataSharePolicyRules> get_contact_data_share_policy_rules(const ConfigDataFilter& filter);

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//GET_CONTACT_DATA_SHARE_POLICY_RULES_HH_8DFB0156981D0A832B7D4CA0020FA0CB
