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

#ifndef NSSET_CORBA_CONVERSION_H_24B9C847E8DA44528A04856129CF17AB
#define NSSET_CORBA_CONVERSION_H_24B9C847E8DA44528A04856129CF17AB

#include "src/corba/EPP.hh"
#include "src/epp/nsset/impl/dns_host_input.h"
#include "src/epp/nsset/impl/nsset_handle_registration_obstruction.h"
#include "src/epp/nsset/impl/nsset_handle_registration_obstruction_localized.h"

#include <map>
#include <string>
#include <vector>

#include <boost/optional.hpp>


namespace Corba {


std::vector<std::string>
unwrap_ccreg_techcontacts_to_vector_string(const ccReg::TechContact& in);


std::vector<Epp::Nsset::DnsHostInput>
unwrap_ccreg_dnshosts_to_vector_dnshosts(const ccReg::DNSHost& in);


boost::optional<short>
unwrap_tech_check_level(CORBA::Short level);


/**
 * @returns data ordered the same way as input nsset_handles
 */
ccReg::CheckResp
wrap_localized_check_info(
        const std::vector<std::string>& nsset_handles,
        const std::map<std::string,
                boost::optional<Epp::Nsset::NssetHandleRegistrationObstructionLocalized> >& nsset_handle_check_results);


ccReg::NSSet
wrap_localized_info_nsset(const Epp::Nsset::InfoNssetLocalizedOutputData& _input);


} // namespace Corba


#endif
