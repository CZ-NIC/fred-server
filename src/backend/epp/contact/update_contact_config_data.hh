/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_CONTACT_CONFIG_DATA_HH_560A4052E8EB4FEF9DBD656BAC6C346B
#define UPDATE_CONTACT_CONFIG_DATA_HH_560A4052E8EB4FEF9DBD656BAC6C346B

#include "src/backend/epp/contact/update_operation_check.hh"

#include <memory>

namespace Epp {
namespace Contact {

class UpdateContactConfigData
{
public:
    UpdateContactConfigData(
            bool _rifd_epp_operations_charging,
            bool _epp_update_contact_enqueue_check,
            const std::shared_ptr<UpdateContactDataFilter>& _data_filter);
    bool are_rifd_epp_operations_charged()const;
    bool are_epp_update_contact_checks_enqueued()const;
    const UpdateContactDataFilter& get_data_filter()const;
private:
    bool rifd_epp_operations_charging_;
    bool epp_update_contact_enqueue_check_;
    std::shared_ptr<UpdateContactDataFilter> data_filter_;
};

}//namespace Epp::Contact
}//namespace Epp

#endif
