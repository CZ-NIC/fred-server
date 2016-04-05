/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  delete all contact states related to admin contact verification
 */

#ifndef CONTACT_VERIFICATION_CONTACT_STATES_DELETE_ALL_38972124897_
#define CONTACT_VERIFICATION_CONTACT_STATES_DELETE_ALL_38972124897_


#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

// legacy stuff
#include "src/fredlib/db_settings.h"

#include <string>

namespace Admin
{
namespace AdminContactVerificationObjectStates
{
    /* OperationContext versions */
    bool conditionally_cancel_final_states(
        Fred::OperationContext& ctx,
        unsigned long long contact_id,
        bool name_changed,
        bool organization_changed,
        bool street1_changed,
        bool street2_changed,
        bool street3_changed,
        bool city_changed,
        bool stateorprovince_changed,
        bool postalcode_changed,
        bool country_changed,
        bool telephone_changed,
        bool fax_changed,
        bool email_changed,
        bool notifyemail_changed,
        bool vat_changed,
        bool ssntype_changed,
        bool ssn_changed
    );

    /** @returns true if contact has changed data related to verification false otherwise */
    bool conditionally_cancel_final_states(
        Fred::OperationContext& ctx,
        unsigned long long contact_id
    );

    void cancel_all_states(Fred::OperationContext& ctx, unsigned long long contact_id);

    void cancel_final_states(Fred::OperationContext& ctx, unsigned long long contact_id);

    /* legacy version (EPP, MojeID) */
    bool conditionally_cancel_final_states_legacy(
        unsigned long long contact_id
    );
}
}
#endif // #include guard end

