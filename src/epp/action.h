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

#ifndef EPP_ACTION_H_545454453137
#define EPP_ACTION_H_545454453137

namespace Epp {

struct Action {
    enum Enum {
        ClientLogin =        100,
        ClientLogout =       101,

        PollAcknowledgement = 120,
        PollResponse =        121,

        ContactCheck =       200,
        ContactInfo =        201,
        ContactDelete =      202,
        ContactUpdate =      203,
        ContactCreate =      204,
        ContactTransfer =    205,

        NSsetCheck =         400,
        NSsetInfo =          401,
        NSsetDelete =        402,
        NSsetUpdate =        403,
        NSsetCreate =        404,
        NSsetTransfer =      405,

        DomainCheck =        500,
        DomainInfo =         501,
        DomainDelete =       502,
        DomainUpdate =       503,
        DomainCreate =       504,
        DomainTransfer =     505,
        DomainRenew =        506,
        DomainTrade =        507,

        KeySetCheck =        600,
        KeySetInfo =         601,
        KeySetDelete =       602,
        KeySetUpdate =       603,
        KeySetCreate =       604,
        KeySetTransfer =     605,

        UnknowAction =      1000,

        ListContact =       1002,
        ListNSset =         1004,
        ListDomain =        1005,
        ListKeySet =        1006,

        ClientCredit =      1010,

        NSsetTest =         1012,

        ContactSendAuthInfo =   1101,
        NSSetSendAuthInfo =     1102,
        DomainSendAuthInfo =    1103,
        KeySetSendAuthInfo =    1106,

        Info =                  1104,
        GetInfoResults =        1105
    };
};

}

#endif
