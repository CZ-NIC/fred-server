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
 *  @file
 */

#ifndef FQDN_STATE_HH_F4D94B717F3044329EAC81ADDEAAADDA
#define FQDN_STATE_HH_F4D94B717F3044329EAC81ADDEAAADDA

namespace LibFred
{

namespace DomainFqdnState {
    struct InRegistry {
        enum Enum {
            registered,
            in_protection_period,
            unregistered,
        };
    };

    struct SyntaxValidity {
        enum Enum {
            valid,
            invalid,
        };
    };

    struct Blacklisted {
        enum Enum {
            blacklisted,
            not_blacklisted,
        };
    };

    struct ZoneInRegister {
        enum Enum {
            zone_in_registry,
            zone_not_in_registry
        };
    };
}

}
#endif
