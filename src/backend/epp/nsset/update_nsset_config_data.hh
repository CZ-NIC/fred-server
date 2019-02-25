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
#ifndef UPDATE_NSSET_CONFIG_DATA_HH_245661F3E3184132AD0F5230C7BA2DE2
#define UPDATE_NSSET_CONFIG_DATA_HH_245661F3E3184132AD0F5230C7BA2DE2

namespace Epp {
namespace Nsset {

struct UpdateNssetConfigData
{
    const bool rifd_epp_operations_charging;
    const unsigned int min_hosts;
    const unsigned int max_hosts;


    UpdateNssetConfigData(
            const bool _rifd_epp_operations_charging,
            const unsigned int _min_hosts,
            const unsigned int _max_hosts)
        : rifd_epp_operations_charging(_rifd_epp_operations_charging),
          min_hosts(_min_hosts),
          max_hosts(_max_hosts)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
