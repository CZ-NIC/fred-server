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

#ifndef CREATE_DOMAIN_INPUT_DATA_H_F9503633D88E4A2C89BE98249E429A98
#define CREATE_DOMAIN_INPUT_DATA_H_F9503633D88E4A2C89BE98249E429A98

#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/domain_registration_time.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Domain {

struct CreateDomainInputData
{
    std::string fqdn;
    std::string registrant;
    std::string nsset;
    std::string keyset;
    boost::optional<std::string> authinfopw;
    DomainRegistrationTime period;
    std::vector<std::string> admin_contacts;
    std::vector<EnumValidationExtension> enum_validation_list;


    CreateDomainInputData(
            const std::string& _fqdn,
            const std::string& _registrant,
            const std::string& _nsset,
            const std::string& _keyset,
            const boost::optional<std::string>& _authinfopw,
            const DomainRegistrationTime& _period,
            const std::vector<std::string>& _admin_contacts,
            const std::vector<EnumValidationExtension>& _enum_validation_list)
        : fqdn(_fqdn),
          registrant(_registrant),
          nsset(_nsset),
          keyset(_keyset),
          authinfopw(_authinfopw),
          period(_period),
          admin_contacts(_admin_contacts),
          enum_validation_list(_enum_validation_list)
    {
    }

};

} // namespace Epp::Domain
} // namespace Epp

#endif
