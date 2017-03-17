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

#ifndef RENEW_DOMAIN_INPUT_DATA_UNWRAPPED_H_67EAB1DE27BC4A24997F0741BD6145F7
#define RENEW_DOMAIN_INPUT_DATA_UNWRAPPED_H_67EAB1DE27BC4A24997F0741BD6145F7

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/domain/domain_corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/domain_registration_time.h"

#include <string>
#include <vector>

namespace Fred {
namespace Corba {
namespace Epp {
namespace Domain {

struct RenewDomainInputDataUnwrapped
{
    std::string fqdn;
    std::string current_exdate;
    ::Epp::Domain::DomainRegistrationTime period;
    std::vector< ::Epp::Domain::EnumValidationExtension> enum_validation_extension_list;


    RenewDomainInputDataUnwrapped(
            const char* _fqdn,
            const char* _current_exdate,
            const ccReg::Period_str& _period,
            const ccReg::ExtensionList& _enum_validation_extension_list)
        : fqdn(Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn)),
          current_exdate(Fred::Corba::unwrap_string_from_const_char_ptr(_current_exdate)),
          period(unwrap_domain_registration_period(_period)),
          enum_validation_extension_list(unwrap_enum_validation_extension_list(_enum_validation_extension_list))
    {
    }
};


} // namespace Fred::Corba::Epp::Domain
} // namespace Fred::Corba::Epp
} // namespace Fred::Corba
} // namespace Fred

#endif
