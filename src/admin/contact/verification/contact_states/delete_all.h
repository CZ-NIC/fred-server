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
    bool conditionally_delete_all(
        Fred::OperationContext& ctx,
        unsigned long long contact_id,
        const Optional<std::string>& name_change_to,
        const Optional<std::string>& organization_change_to,
        const Optional<std::string>& street1_change_to,
        const Optional<std::string>& street2_change_to,
        const Optional<std::string>& street3_change_to,
        const Optional<std::string>& city_change_to,
        const Optional<std::string>& stateorprovince_change_to,
        const Optional<std::string>& postalcode_change_to,
        const Optional<std::string>& country_change_to,
        const Optional<std::string>& telephone_change_to,
        const Optional<std::string>& fax_change_to,
        const Optional<std::string>& email_change_to,
        const Optional<std::string>& notifyemail_change_to,
        const Optional<std::string>& vat_change_to,
        const Optional<std::string>& ssntype_change_to,
        const Optional<std::string>& ssn_change_to
    );

    void cancel_all_states(Fred::OperationContext& ctx, unsigned long long contact_id);

    /* legacy version (EPP, MojeID) */
    bool conditionally_cancel_final_states_legacy(
        unsigned long long contact_id
    );
}
}
#endif // #include guard end

