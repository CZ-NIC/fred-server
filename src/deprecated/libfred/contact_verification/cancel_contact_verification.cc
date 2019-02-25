/*
 * Copyright (C) 2012-2019  CZ.NIC, z. s. p. o.
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
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/db_settings.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "src/deprecated/libfred/contact_verification/cancel_contact_verification.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_state.hh"

namespace LibFred {
namespace Contact {
namespace Verification {

//check conditions for cancel verification
bool check_contact_change_for_cancel_verification(
                    const std::string & contact_handle)
{
    Logging::Context ctx("check_contact_change_for_cancel_verification");

    try
    {
        //get contact id
        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec_params(
            "SELECT c.id FROM contact c "
            " JOIN object_registry oreg ON oreg.id = c.id "
            " WHERE oreg.name = upper($1::text)",
            Database::query_param_list(contact_handle));
        if (result.size() != 1)
            throw std::runtime_error("unable to get contact id");
        const unsigned long long contact_id = static_cast<unsigned long long>(result[0][0]);

        const State contact_state = get_contact_verification_state(contact_id);

        //diff contact change if conditionally identified or identified
        if (contact_state.has_any(State::CIvm))
        {
            Database::Connection conn = Database::Manager::acquire();
            Database::Result result = conn.exec_params(
                "SELECT ((COALESCE(ch1.email,'') != COALESCE(ch2.email,'')) OR "
                " (COALESCE(ch1.telephone,'') != COALESCE(ch2.telephone,'')) OR "
                " (COALESCE(ch1.name,'') != COALESCE(ch2.name,'')) OR "
                " (COALESCE(ch1.organization,'') != COALESCE(ch2.organization,'')) OR "
                " (COALESCE(ch1.street1,'') != COALESCE(ch2.street1,'')) OR "
                " (COALESCE(ch1.street2,'') != COALESCE(ch2.street2,'')) OR "
                " (COALESCE(ch1.street3,'') != COALESCE(ch2.street3,'')) OR "
                " (COALESCE(ch1.city,'') != COALESCE(ch2.city,'')) OR "
                " (COALESCE(ch1.stateorprovince,'') != COALESCE(ch2.stateorprovince,'')) OR "
                " (COALESCE(ch1.postalcode,'') != COALESCE(ch2.postalcode,'')) OR "
                " (COALESCE(ch1.country,'') != COALESCE(ch2.country,'')) "
                " )AS differ "
                " FROM object_registry oreg "
                " JOIN contact_history ch1 ON ch1.historyid = oreg.historyid "
                " JOIN history h ON h.next = ch1.historyid "
                " JOIN contact_history ch2 ON ch2.historyid = h.id "
                " JOIN contact c ON c.id = oreg.id "
                " WHERE oreg.name = upper($1::text) "
                  , Database::query_param_list(contact_handle));
            if (result.size() != 1)
                throw std::runtime_error("unable to get contact difference");
            bool contact_differ = static_cast<bool>(result[0][0]);
            return contact_differ;
        }
    }//try
    catch (std::exception &_ex)
    {
        LOGGER.error(_ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown exception");
        throw;
    }

    return false;
}



//cancel contact state
//conditionaly_identified if state conditionaly_identified is set
//and state identified if state identified is set
//throw if error
void contact_cancel_verification(
        const std::string & contact_handle)
{
    Logging::Context ctx("contact_cancel_verification");
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec_params(
                "SELECT c.id FROM contact c "
                " JOIN object_registry oreg ON oreg.id = c.id "
                " WHERE oreg.name = upper($1::text)",
                Database::query_param_list(contact_handle));
        if (result.size() != 1)
            throw std::runtime_error("unable to get contact id");

        const unsigned long long contact_id = static_cast<unsigned long long>(result[0][0]);

        const State contact_state = get_contact_verification_state(contact_id);
        if (contact_state.has_all(State::Civm)) {
            LibFred::cancel_object_state(contact_id
                , LibFred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);
            update_object_states(contact_id);
        }

        if (contact_state.has_all(State::cIvm)) {
            LibFred::cancel_object_state(contact_id
                , LibFred::ObjectState::IDENTIFIED_CONTACT);
            update_object_states(contact_id);
        }
    }//try
    catch (std::exception &_ex)
    {
        LOGGER.error(_ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("unknown exception");
        throw;
    }
}
}}}//namespace
