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

#ifndef CREATE_CONTACT_CONFIG_DATA_HH_DFE73286069847C7803E1DABB05D9CA0
#define CREATE_CONTACT_CONFIG_DATA_HH_DFE73286069847C7803E1DABB05D9CA0

#include "src/backend/epp/contact/create_operation_check.hh"

#include <memory>

namespace Epp {
namespace Contact {

class CreateContactConfigData
{
public:
    CreateContactConfigData(
            bool _rifd_epp_operations_charging,
            const std::shared_ptr<CreateContactDataFilter>& _data_filter);
    bool are_rifd_epp_operations_charged()const;
    const CreateContactDataFilter& get_data_filter()const;
private:
    bool rifd_epp_operations_charging_;
    std::shared_ptr<CreateContactDataFilter> data_filter_;
};

} // namespace Epp::Contact
} // namespace Epp

#endif
