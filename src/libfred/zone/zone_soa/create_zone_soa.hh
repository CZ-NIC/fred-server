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

#include <boost/optional.hpp>
#include <string>

namespace LibFred {
namespace Zone {

class CreateZoneSoa
{
public:
    explicit CreateZoneSoa(const std::string& _fqdn);

    CreateZoneSoa& set_ttl(boost::optional<int> _ttl);

    CreateZoneSoa& set_hostmaster(const boost::optional<std::string>& _hostmaster);

    CreateZoneSoa& set_refresh(boost::optional<int> _refresh);

    CreateZoneSoa& set_update_retr(boost::optional<int> _update_retr);

    CreateZoneSoa& set_expiry(boost::optional<int> _expiry);

    CreateZoneSoa& set_minimum(boost::optional<int> _minimum);

    CreateZoneSoa& set_ns_fqdn(const boost::optional<std::string>& _ns_fqdn);

    unsigned long long exec(OperationContext& _ctx) const;

private:
    std::string fqdn_;
    boost::optional<int> ttl_;
    boost::optional<std::string> hostmaster_;
    boost::optional<int> refresh_;
    boost::optional<int> update_retr_;
    boost::optional<int> expiry_;
    boost::optional<int> minimum_;
    boost::optional<std::string> ns_fqdn_;

};

} // namespace LibFred::Zone
} // namespace LibFred

#endif
