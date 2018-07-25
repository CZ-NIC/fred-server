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

#ifndef INFO_ZONE_SOA_DATA_HH_17AB51EB359B448AA1B70336C30C8457
#define INFO_ZONE_SOA_DATA_HH_17AB51EB359B448AA1B70336C30C8457

#include <string>

namespace LibFred {
namespace Zone {

struct InfoZoneSoaData
{
    unsigned long long zone;
    int ttl;
    std::string hostmaster;
    int refresh;
    int update_retr;
    int expiry;
    int minimum;
    std::string ns_fqdn;
};

} // namespace LibFred::Zone
} // namespace LibFred


#endif
