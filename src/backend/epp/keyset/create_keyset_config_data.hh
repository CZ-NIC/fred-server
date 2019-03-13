/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef CREATE_KEYSET_CONFIG_DATA_HH_0C7F4A6A2C8B47E3A7C6EED78B5D7325
#define CREATE_KEYSET_CONFIG_DATA_HH_0C7F4A6A2C8B47E3A7C6EED78B5D7325

namespace Epp {
namespace Keyset {

struct CreateKeysetConfigData
{
    const bool rifd_epp_operations_charging;


    CreateKeysetConfigData(
            const bool _rifd_epp_operations_charging)
        : rifd_epp_operations_charging(_rifd_epp_operations_charging)
    {
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
