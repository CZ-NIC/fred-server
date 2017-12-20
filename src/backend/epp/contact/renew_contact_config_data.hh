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

#ifndef RENEW_CONTACT_CONFIG_DATA_H_019F32684BA44AB893AEC20D7C875F0F
#define RENEW_CONTACT_CONFIG_DATA_H_019F32684BA44AB893AEC20D7C875F0F

namespace Epp {
namespace Contact {

struct UpdateContactConfigData
{
    const bool rifd_epp_operations_charging;


    UpdateContactConfigData(
            const bool _rifd_epp_operations_charging)
        : rifd_epp_operations_charging(_rifd_epp_operations_charging)
    {
    }


};

} // namespace Epp::Contact
} // namespace Epp

#endif
