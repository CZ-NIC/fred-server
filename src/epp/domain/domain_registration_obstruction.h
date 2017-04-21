/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef DOMAIN_REGISTRATION_OBSTRUCTION_H_EC6B8C64A07B47D7AB68755BAEB424A9
#define DOMAIN_REGISTRATION_OBSTRUCTION_H_EC6B8C64A07B47D7AB68755BAEB424A9

#include "src/epp/exception.h"
#include "src/epp/reason.h"

#include <string>

namespace Epp {
namespace Domain {

struct DomainRegistrationObstruction
{
    enum Enum
    {
        registered,
        blacklisted,
        zone_not_in_registry,
        invalid_fqdn

    };

    /**
     * @throws MissingLocalizedDescription
     */
    static Reason::Enum to_reason(Enum value)
    {
        switch (value)
        {
            case registered:
                return Reason::existing;

            case blacklisted:
                return Reason::blacklisted_domain;

            case zone_not_in_registry:
                return Reason::not_applicable_domain;

            case invalid_fqdn:
                return Reason::invalid_handle;
        }
        throw MissingLocalizedDescription();
    }


    /**
     * @throws MissingLocalizedDescription
     */
    static DomainRegistrationObstruction::Enum from_reason(const Reason::Enum value)
    {
        switch (value)
        {
            case Reason::existing:
                return DomainRegistrationObstruction::registered;

            case Reason::blacklisted_domain:
                return DomainRegistrationObstruction::blacklisted;

            case Reason::not_applicable_domain:
                return DomainRegistrationObstruction::zone_not_in_registry;

            case Reason::invalid_handle:
                return DomainRegistrationObstruction::invalid_fqdn;

            default:
                throw MissingLocalizedDescription();
        }
    }


};

struct DomainLocalizedRegistrationObstruction
{
    const DomainRegistrationObstruction::Enum state;
    const std::string description;


    DomainLocalizedRegistrationObstruction(
            const DomainRegistrationObstruction::Enum state,
            const std::string& description)
        : state(state),
          description(description)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
