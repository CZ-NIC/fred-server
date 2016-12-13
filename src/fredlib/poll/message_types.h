/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  poll message types
 */

#ifndef MESSAGE_TYPES_H_B95C664FD50B40C2B5A521511970095C
#define MESSAGE_TYPES_H_B95C664FD50B40C2B5A521511970095C

#include <string>

namespace Fred {
namespace Poll {
    using std::string;

    const string CREDIT                 = "credit";
    const string REQUEST_FEE_INFO       = "request_fee_info";

    const string TECHCHECK              = "techcheck";

    const string TRANSFER_CONTACT       = "transfer_contact";
    const string TRANSFER_DOMAIN        = "transfer_domain";
    const string TRANSFER_NSSET         = "transfer_nsset";
    const string TRANSFER_KEYSET        = "transfer_keyset";

    const string IDLE_DELETE_CONTACT    = "idle_delete_contact";
    const string IDLE_DELETE_DOMAIN     = "idle_delete_domain";
    const string IDLE_DELETE_NSSET      = "idle_delete_nsset";
    const string IDLE_DELETE_KEYSET     = "idle_delete_keyset";

    const string IMP_EXPIRATION         = "imp_expiration";
    const string EXPIRATION             = "expiration";
    const string OUTZONE                = "outzone";

    const string IMP_VALIDATION         = "imp_validation";
    const string VALIDATION             = "validation";

    const string UPDATE_DOMAIN          = "update_domain";
    const string UPDATE_NSSET           = "update_nsset";
    const string UPDATE_KEYSET          = "update_keyset";

    const string DELETE_CONTACT         = "delete_contact";
    const string DELETE_DOMAIN          = "delete_domain";
}
}

#endif
