/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef RENEW_DOMAIN_INPUT_DATA_HH_1836C9F245B6483FAF65646EA1183226
#define RENEW_DOMAIN_INPUT_DATA_HH_1836C9F245B6483FAF65646EA1183226

#include "src/backend/epp/domain/domain_enum_validation.hh"
#include "src/backend/epp/domain/domain_registration_time.hh"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Domain {

struct RenewDomainInputData
{
    std::string fqdn;
    std::string current_exdate;
    DomainRegistrationTime period;
    boost::optional< ::Epp::Domain::EnumValidationExtension> enum_validation_extension;


    RenewDomainInputData(
            const std::string& _fqdn,
            const std::string& _current_exdate,
            const DomainRegistrationTime& _period,
            const boost::optional< ::Epp::Domain::EnumValidationExtension>& _enum_validation_extension)
        : fqdn(_fqdn),
          current_exdate(_current_exdate),
          period(_period),
          enum_validation_extension(_enum_validation_extension)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
