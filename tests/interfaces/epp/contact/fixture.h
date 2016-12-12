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

#ifndef TEST_INTERFACE_EPP_FIXTURE_681453104385310
#define TEST_INTERFACE_EPP_FIXTURE_681453104385310

#include "tests/setup/fixtures.h"
#include "tests/interfaces/epp/util.h"

#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/get_object_states.h"

struct has_invalid_registrar_id : virtual Test::autocomitting_context {
    static const unsigned long long invalid_registrar_id = 0;
};

struct has_registrar : virtual Test::autocommitting_context {
    Fred::InfoRegistrarData registrar;

    has_registrar() {
        const std::string reg_handle = "REGISTRAR1";
        Fred::CreateRegistrar(reg_handle).exec(ctx);
        registrar = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_contact : has_registrar {
    Fred::InfoContactData contact;

    has_contact() {
        const std::string contact_handle = "CONTACT1";
        Fred::CreateContact(contact_handle, registrar.handle).exec(ctx);
        contact = Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
    }
};

struct has_contact_and_a_different_registrar : has_contact {
    Fred::InfoRegistrarData the_different_registrar;

    has_contact_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_contact_with_status_request : has_contact {
    const std::string status;

    has_contact_with_status_request(const std::string& _status)
    :   status(_status)
    {
        ctx.get_conn().exec_params(
            "UPDATE enum_object_states SET manual = 'true'::bool WHERE name = $1::text",
            Database::query_param_list(_status)
        );

        const std::set<std::string> statuses = boost::assign::list_of(_status);

        Fred::CreateObjectStateRequestId(contact.id, statuses).exec(ctx);

        // ensure object has only request, not the state itself
        {
            std::vector<std::string> object_states_before;
            {
                BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(contact.id).exec(ctx) ) {
                    object_states_before.push_back(state.state_name);
                }
            }

            BOOST_CHECK(
                std::find( object_states_before.begin(), object_states_before.end(), _status )
                ==
                object_states_before.end()
            );
        }
    }
};

struct has_contact_with_status : has_contact_with_status_request {
    has_contact_with_status(const std::string& _status)
    :   has_contact_with_status_request(_status)
    {
        Fred::PerformObjectStateRequest(contact.id).exec(ctx);
    }
};

struct has_contact_with_server_update_prohibited : has_contact_with_status {
    has_contact_with_server_update_prohibited()
    :   has_contact_with_status("serverUpdateProhibited")
    { }
};

struct has_contact_with_server_transfer_prohibited : has_contact_with_status {
    has_contact_with_server_transfer_prohibited()
    :   has_contact_with_status("serverTransferProhibited")
    { }
};

struct has_contact_with_delete_candidate : has_contact_with_status {
    has_contact_with_delete_candidate()
    :   has_contact_with_status("deleteCandidate")
    { }
};

struct has_contact_with_delete_candidate_request : has_contact_with_status_request {
    has_contact_with_delete_candidate_request()
    :   has_contact_with_status_request("deleteCandidate")
    { }
};

struct has_contact_with_server_transfer_prohibited_request : has_contact_with_status_request {
    has_contact_with_server_transfer_prohibited_request()
    :   has_contact_with_status_request("serverTransferProhibited")
    { }
};

struct has_contact_with_server_update_prohibited_request : has_contact_with_status_request {
    has_contact_with_server_update_prohibited_request()
    :   has_contact_with_status_request("serverUpdateProhibited")
    { }
};

#endif
