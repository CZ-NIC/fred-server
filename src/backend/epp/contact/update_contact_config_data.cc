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

#include "src/backend/epp/contact/update_contact_config_data.hh"

namespace Epp {
namespace Contact {

UpdateContactConfigData::UpdateContactConfigData(
        bool _rifd_epp_operations_charging,
        bool _epp_update_contact_enqueue_check,
        const std::shared_ptr<UpdateOperationCheck>& _operation_check)
    : rifd_epp_operations_charging_(_rifd_epp_operations_charging),
      epp_update_contact_enqueue_check_(_epp_update_contact_enqueue_check),
      operation_check_(_operation_check)
{ }

bool UpdateContactConfigData::are_rifd_epp_operations_charged()const
{
    return rifd_epp_operations_charging_;
}

bool UpdateContactConfigData::are_epp_update_contact_checks_enqueued()const
{
    return epp_update_contact_enqueue_check_;
}

const UpdateOperationCheck& UpdateContactConfigData::get_operation_check()const
{
    return *operation_check_;
}

}//namespace Epp::Contact
}//namespace Epp
