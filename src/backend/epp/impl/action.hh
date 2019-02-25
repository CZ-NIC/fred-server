/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef ACTION_HH_8969C7ED087B41B8B5AEAB12AE3BEF81
#define ACTION_HH_8969C7ED087B41B8B5AEAB12AE3BEF81

namespace Epp {

struct Action
{
    enum Enum
    {
        ClientLogin         = 100,
        ClientLogout        = 101,

        PollAcknowledgement = 120,
        PollResponse        = 121,

        CheckContact        = 200,
        InfoContact         = 201,
        DeleteContact       = 202,
        UpdateContact       = 203,
        CreateContact       = 204,
        TransferContact     = 205,

        CheckNsset          = 400,
        InfoNsset           = 401,
        DeleteNsset         = 402,
        UpdateNsset         = 403,
        CreateNsset         = 404,
        TransferNsset       = 405,

        CheckDomain         = 500,
        InfoDomain          = 501,
        DeleteDomain        = 502,
        UpdateDomain        = 503,
        CreateDomain        = 504,
        TransferDomain      = 505,
        RenewDomain         = 506,
        TradeDomain         = 507,

        CheckKeyset         = 600,
        InfoKeyset          = 601,
        DeleteKeyset        = 602,
        UpdateKeyset        = 603,
        CreateKeyset        = 604,
        TransferKeyset      = 605,

        UnknowAction        = 1000,

        ListContact         = 1002,
        ListNsset           = 1004,
        ListDomain          = 1005,
        ListKeyset          = 1006,

        ClientCredit        = 1010,

        NssetTest           = 1012,

        ContactSendAuthInfo = 1101,
        NssetSendAuthInfo   = 1102,
        DomainSendAuthInfo  = 1103,
        KeysetSendAuthInfo  = 1106,

        Info                = 1104,
        GetInfoResults      = 1105
    };
};

}

#endif
