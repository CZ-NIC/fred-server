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

namespace Epp {
namespace Contact {

struct UpdateContactConfigData
{
    const bool rifd_epp_operations_charging;
    const bool epp_update_contact_enqueue_check;


    UpdateContactConfigData(
            const bool _rifd_epp_operations_charging,
            const bool _epp_update_contact_enqueue_check)
        : rifd_epp_operations_charging(_rifd_epp_operations_charging),
          epp_update_contact_enqueue_check(_epp_update_contact_enqueue_check)
    {
    }


};

} // namespace Epp::Contact
} // namespace Epp

#endif