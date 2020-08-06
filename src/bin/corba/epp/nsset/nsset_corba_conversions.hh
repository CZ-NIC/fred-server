/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
#ifndef NSSET_CORBA_CONVERSIONS_HH_8BFAB0F8B96E4F5F930C946006979713
#define NSSET_CORBA_CONVERSIONS_HH_8BFAB0F8B96E4F5F930C946006979713

#include "corba/EPP.hh"

#include "src/backend/epp/nsset/dns_host_input.hh"
#include "src/backend/epp/nsset/info_nsset_localized_output_data.hh"
#include "src/backend/epp/nsset/nsset_handle_registration_obstruction.hh"
#include "src/backend/epp/nsset/nsset_handle_registration_obstruction_localized.hh"

#include <map>
#include <string>
#include <vector>

#include <boost/optional.hpp>


namespace LibFred {
namespace Corba {


std::vector<std::string>
unwrap_ccreg_techcontacts_to_vector_string(const ccReg::TechContact& in);


std::vector< ::Epp::Nsset::DnsHostInput>
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
                boost::optional< ::Epp::Nsset::NssetHandleRegistrationObstructionLocalized> >& nsset_handle_check_results);


ccReg::NSSet
wrap_localized_info_nsset(const ::Epp::Nsset::InfoNssetLocalizedOutputData& _input);


} // namespace LibFred::Corba
} // namespace Corba

#endif
