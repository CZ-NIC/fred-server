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

#ifndef RENEW_DOMAIN_INPUT_DATA_H_D65B0A775E5B45A1A12B3920EF972A63
#define RENEW_DOMAIN_INPUT_DATA_H_D65B0A775E5B45A1A12B3920EF972A63

#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/domain_registration_time.h"

#include <string>
#include <vector>

namespace Epp {
namespace Domain {

struct RenewDomainInputData
{
    std::string fqdn;
    std::string current_exdate;
    DomainRegistrationTime period;
    std::vector<EnumValidationExtension> enum_validation_extension_list;


    RenewDomainInputData(
            const std::string& _fqdn,
            const std::string& _current_exdate,
            const DomainRegistrationTime& _period,
            const std::vector<EnumValidationExtension>& _enum_validation_extension_list)
        : fqdn(_fqdn),
          current_exdate(_current_exdate),
          period(_period),
          enum_validation_extension_list(_enum_validation_extension_list)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
