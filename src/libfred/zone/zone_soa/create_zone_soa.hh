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
    CreateZoneSoa(const std::string& _fqdn, const std::string& _hostmaster, const std::string& _ns_fqdn);

    CreateZoneSoa& set_ttl(boost::optional<unsigned long> _ttl);

    CreateZoneSoa& set_refresh(boost::optional<unsigned long> _refresh);

    CreateZoneSoa& set_update_retr(boost::optional<unsigned long> _update_retr);

    CreateZoneSoa& set_expiry(boost::optional<unsigned long> _expiry);

    CreateZoneSoa& set_minimum(boost::optional<unsigned long> _minimum);

    unsigned long long exec(OperationContext& _ctx) const;

private:
    std::string fqdn_;
    std::string hostmaster_;
    std::string ns_fqdn_;
    boost::optional<unsigned long> ttl_;
    boost::optional<unsigned long> refresh_;
    boost::optional<unsigned long> update_retr_;
    boost::optional<unsigned long> expiry_;
    boost::optional<unsigned long> minimum_;

};

} // namespace LibFred::Zone
} // namespace LibFred

#endif
