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

#ifndef CREATE_NSSET_INPUT_DATA_UNWRAPPED_H_27745CA7BFB1438BBEA53D5BC7C108D1
#define CREATE_NSSET_INPUT_DATA_UNWRAPPED_H_27745CA7BFB1438BBEA53D5BC7C108D1

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/nsset/nsset_corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/nsset/dns_host_input.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Corba {
namespace Epp {
namespace Nsset {

struct CreateNssetInputDataUnwrapped
{
    std::string nsset_handle;
    boost::optional<std::string> authinfopw;
    std::vector<std::string> tech_contacts;
    std::vector< ::Epp::Nsset::DnsHostInput> dns_hosts;
    boost::optional<short> tech_check_level;

    CreateNssetInputDataUnwrapped(
            const char* _nsset_handle,
            const char* _authinfopw,
            const ccReg::TechContact& _tech_contacts,
            const ccReg::DNSHost& _dns_hosts,
            CORBA::Short _tech_check_level)
        : nsset_handle(Fred::Corba::unwrap_string_from_const_char_ptr(_nsset_handle)),
          authinfopw(Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw)),
          tech_contacts(Fred::Corba::unwrap_ccreg_techcontacts_to_vector_string(_tech_contacts)),
          dns_hosts(Fred::Corba::unwrap_ccreg_dnshosts_to_vector_dnshosts(_dns_hosts)),
          tech_check_level(Fred::Corba::unwrap_tech_check_level(_tech_check_level))
    {
    }


};

} // namespace Fred::Corba::Epp::Domain
} // namespace Fred::Corba::Epp
} // namespace Fred::Corba
} // namespace Fred

#endif
