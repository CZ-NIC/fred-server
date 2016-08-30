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

/**
 *  @file domain_registration_obstruction.h
 *  <++>
 */

#ifndef SRC_EPP_DOMAIN_DOMAIN_REGISTRATION_OBSTRUCTION_H
#define SRC_EPP_DOMAIN_DOMAIN_REGISTRATION_OBSTRUCTION_H

#include "src/epp/exception.h"
#include "src/epp/reason.h"

#include <set>
#include <string>

namespace Epp {

namespace Domain {

struct DomainRegistrationObstruction {

    enum Enum {
        registered,
        blacklisted,
        zone_not_in_registry,
        invalid_fqdn
    };

    /**
     * @throws MissingLocalizedDescription
     */
    static Reason::Enum to_reason(Enum value) {
        switch (value) {
            case registered:           return Reason::existing;
            case blacklisted:          return Reason::blacklisted_domain;
            case zone_not_in_registry: return Reason::not_applicable_domain;
            case invalid_fqdn:         return Reason::bad_format_fqdn;
        }
        throw MissingLocalizedDescription();
    }
};

struct DomainLocalizedRegistrationObstruction {

    const DomainRegistrationObstruction::Enum state;
    const std::string description;

    DomainLocalizedRegistrationObstruction(
        const DomainRegistrationObstruction::Enum state,
        const std::string& description
    ) :
       state(state),
       description(description)
    { }

};

}

}

#endif
