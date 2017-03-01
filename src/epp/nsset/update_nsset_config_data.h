/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_NSSET_CONFIG_DATA_H_C69173EA86384A4E9BDD9545CBF15EF1
#define UPDATE_NSSET_CONFIG_DATA_H_C69173EA86384A4E9BDD9545CBF15EF1

namespace Epp {
namespace Nsset {

struct UpdateNssetConfigData
{
    unsigned int min_hosts;
    unsigned int max_hosts;


    UpdateNssetConfigData(
            const unsigned int _min_hosts,
            const unsigned int _max_hosts)
        : min_hosts(_min_hosts),
          max_hosts(_max_hosts)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
