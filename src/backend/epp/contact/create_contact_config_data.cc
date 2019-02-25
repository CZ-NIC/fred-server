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
#include "src/backend/epp/contact/create_contact_config_data.hh"

namespace Epp {
namespace Contact {

CreateContactConfigData::CreateContactConfigData(
        bool _rifd_epp_operations_charging,
        const std::shared_ptr<CreateContactDataFilter>& _data_filter)
    : rifd_epp_operations_charging_(_rifd_epp_operations_charging),
      data_filter_(_data_filter)
{ }

bool CreateContactConfigData::are_rifd_epp_operations_charged()const
{
    return rifd_epp_operations_charging_;
}

const CreateContactDataFilter& CreateContactConfigData::get_data_filter()const
{
    return *data_filter_;
}

}//namespace Epp::Contact
}//namespace Epp
