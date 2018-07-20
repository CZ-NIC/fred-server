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

#ifndef CREATE_ZONE_SOA_HH_2816933E658E4D0CB9A4326C1DDA364E
#define CREATE_ZONE_SOA_HH_2816933E658E4D0CB9A4326C1DDA364E

#include "src/libfred/opcontext.hh"

#include <string>

namespace LibFred {
namespace ZoneSoa {

class CreateZoneSoa
{
public:
    explicit CreateZoneSoa(const std::string& _fqdn);

    CreateZoneSoa& set_ttl(int _ttl);

    CreateZoneSoa& set_hostmaster(const std::string& _hostmaster);

    CreateZoneSoa& set_refresh(int _refresh);

    CreateZoneSoa& set_update_retr(int _update_retr);

    CreateZoneSoa& set_expiry(int _expiry);

    CreateZoneSoa& set_minimum(int _minimum);

    CreateZoneSoa& set_ns_fqdn(const std::string& _ns_fqdn);

    unsigned long long exec(OperationContext& _ctx) const;

private:
    std::string fqdn_;
    int ttl_;
    std::string hostmaster_;
    int refresh_;
    int update_retr_;
    int expiry_;
    int minimum_;
    std::string ns_fqdn_;

};

} // namespace LibFred::ZoneSoa
} // namespace LibFred

#endif
