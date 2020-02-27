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
#ifndef DOMAIN_CORBA_CONVERSIONS_HH_C83F184EAE0044F6A19B40F4BB084085
#define DOMAIN_CORBA_CONVERSIONS_HH_C83F184EAE0044F6A19B40F4BB084085

#include "corba/EPP.hh"

#include "src/backend/epp/domain/check_domain_localized.hh"
#include "src/backend/epp/domain/domain_registration_time.hh"
#include "src/backend/epp/domain/info_domain_localized_output_data.hh"
#include "src/backend/epp/domain/info_domain_localized.hh"

#include "src/util/corba_conversion.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"

#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace LibFred {
namespace Corba {
namespace Epp {
namespace Domain {

/**
 * Unwrapper for attributes which can be empty with special meaning and can have control char with special meaning
 *
 * @param _src string to be unwrapped, should not be NULL
 *
 * @return empty string if input string empty, Optinal(Nullable()) if input ocntains special control char, unwrapped input in other cases
 */
Optional<Nullable<std::string> >
unwrap_string_for_change_or_remove_to_Optional_Nullable_string(const char* _src);


/**
 * @returns data ordered the same way as input fqdns
 */
ccReg::CheckResp
wrap_Epp_Domain_CheckDomainLocalizedResponse(
        const std::vector<std::string>& _domain_fqdns,
        const std::map<std::string,
                boost::optional< ::Epp::Domain::DomainLocalizedRegistrationObstruction> >& _domain_fqdn_to_domain_localized_registration_obstruction);


void
wrap_Epp_Domain_InfoDomainLocalizedOutputData(
        const ::Epp::Domain::InfoDomainLocalizedOutputData& _src,
        ccReg::Domain& _dst);


/**
 * length of domain registration period
 */
::Epp::Domain::DomainRegistrationTime
unwrap_domain_registration_period(const ccReg::Period_str& period);


/**
 * domain administrative contacts unwrapper
 */
std::vector<std::string>
unwrap_ccreg_admincontacts_to_vector_string(const ccReg::AdminContact& in);


/**
 * ENUM validation list unwrapper
 */
boost::optional< ::Epp::Domain::EnumValidationExtension>
unwrap_enum_validation_extension_list(const ccReg::ExtensionList& ext);


} // namespace LibFred::Corba;:Epp::Domain
} // namespace LibFred::Corba::Epp
} // namespace LibFred::Corba
} // namespace LibFred

#endif
