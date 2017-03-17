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

#ifndef UPDATE_NSSET_INPUT_DATA_UNWRAPPED_H_AD02630BDA2D4103BF2EE2FE50FBBA5A
#define UPDATE_NSSET_INPUT_DATA_UNWRAPPED_H_AD02630BDA2D4103BF2EE2FE50FBBA5A

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/nsset/nsset_corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/nsset/dns_host_input.h"
#include "src/epp/nsset/dns_host_output.h"
#include "util/optional_value.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Corba {
namespace Epp {
namespace Nsset {

struct UpdateNssetInputDataUnwrapped
{
    std::string nsset_handle;
    Optional<std::string> authinfopw;
    std::vector< ::Epp::Nsset::DnsHostInput> dns_hosts_add;
    std::vector< ::Epp::Nsset::DnsHostInput> dns_hosts_rem;
    std::vector<std::string> tech_contacts_add;
    std::vector<std::string> tech_contacts_rem;
    boost::optional<short> tech_check_level;


    UpdateNssetInputDataUnwrapped(
            const char* _nsset_handle,
            const char* _authinfopw_chg,
            const ccReg::DNSHost& _dns_add,
            const ccReg::DNSHost& _dns_rem,
            const ccReg::TechContact& _tech_add,
            const ccReg::TechContact& _tech_rem,
            CORBA::Short _tech_check_level)
        : nsset_handle(Fred::Corba::unwrap_string_from_const_char_ptr(_nsset_handle)),
          authinfopw(Fred::Corba::unwrap_string_for_change_or_remove_to_Optional_string(_authinfopw_chg)),
          dns_hosts_add(Fred::Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(_dns_add)),
          dns_hosts_rem(Fred::Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(_dns_rem)),
          tech_contacts_add(Fred::Corba::unwrap_ccreg_techcontacts_to_vector_string(_tech_add)),
          tech_contacts_rem(Fred::Corba::unwrap_ccreg_techcontacts_to_vector_string(_tech_rem)),
          tech_check_level(Fred::Corba::unwrap_tech_check_level(_tech_check_level))
    {
    }


};

} // namespace Fred::Corba::Epp::Domain
} // namespace Fred::Corba::Epp
} // namespace Fred::Corba
} // namespace Fred

#endif
