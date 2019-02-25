/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef INFO_CONTACT_CONFIG_DATA_HH_F7DDDCCA0C744EEB846B216F7AF03958
#define INFO_CONTACT_CONFIG_DATA_HH_F7DDDCCA0C744EEB846B216F7AF03958

namespace Epp {
namespace Contact {

struct InfoContactConfigData
{
    const bool rifd_epp_operations_charging;


    InfoContactConfigData(
            const bool _rifd_epp_operations_charging)
        : rifd_epp_operations_charging(_rifd_epp_operations_charging)
    {
    }


};

} // namespace Epp::Contact
} // namespace Epp

#endif
