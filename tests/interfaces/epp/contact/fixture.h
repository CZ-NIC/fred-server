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

#ifndef TEST_INTERFACE_EPP_FIXTURE_681453104385310
#define TEST_INTERFACE_EPP_FIXTURE_681453104385310

#include "tests/setup/fixtures.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/contact/check_contact_config_data.h"
#include "src/epp/contact/info_contact_config_data.h"
#include "src/epp/contact/create_contact_config_data.h"
#include "src/epp/contact/update_contact_config_data.h"
#include "src/epp/contact/delete_contact_config_data.h"
#include "src/epp/contact/transfer_contact_config_data.h"
#include "src/epp/session_data.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "util/optional_value.h"

struct TestCheckContactConfigData : Epp::Contact::CheckContactConfigData
{
    TestCheckContactConfigData()
        : CheckContactConfigData(false)
    {
    }
};

struct TestInfoContactConfigData : Epp::Contact::InfoContactConfigData
{
    TestInfoContactConfigData()
        : InfoContactConfigData(false)
    {
    }
};

struct TestCreateContactConfigData : Epp::Contact::CreateContactConfigData
{
    TestCreateContactConfigData()
        : CreateContactConfigData(false)
    {
    }
};

struct TestUpdateContactConfigData : Epp::Contact::UpdateContactConfigData
{
    TestUpdateContactConfigData()
        : UpdateContactConfigData(false, false)
    {
    }
};

struct TestDeleteContactConfigData : Epp::Contact::DeleteContactConfigData
{
    TestDeleteContactConfigData()
        : DeleteContactConfigData(false)
    {
    }
};

struct TestTransferContactConfigData : Epp::Contact::TransferContactConfigData
{
    TestTransferContactConfigData()
        : TransferContactConfigData(false)
    {
    }
};

struct HasConfig {
    TestCheckContactConfigData check_contact_config_data;
    TestInfoContactConfigData info_contact_config_data;
    TestCreateContactConfigData create_contact_config_data;
    TestUpdateContactConfigData update_contact_config_data;
    TestDeleteContactConfigData delete_contact_config_data;
    TestTransferContactConfigData transfer_contact_config_data;

    HasConfig() {
    }
};

struct TestSessionData : Epp::SessionData
{
    TestSessionData()
        : SessionData(0, Epp::SessionLang::en, "", boost::optional<unsigned long long>(0))
    {
    }
};

struct HasSession {
    TestSessionData data;

    HasSession() {
    }
};

struct HasInvalidSessionRegistrar : virtual Test::autocommitting_context {
    HasSession session;
    HasConfig config;

    HasInvalidSessionRegistrar()
    {
        const unsigned long long invalid_session_registrar_id = 0;
        session.data.registrar_id = invalid_session_registrar_id;
        BOOST_REQUIRE(!is_session_registrar_valid(session.data));
    }
};

struct HasRegistrar : virtual Test::autocommitting_context {
    Fred::InfoRegistrarData registrar;
    HasSession session;
    HasConfig config;

    HasRegistrar() {
        const std::string reg_handle = "REGISTRAR1";
        Fred::CreateRegistrar(reg_handle).exec(ctx);
        registrar = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;

        session.data.registrar_id = registrar.id;
        BOOST_REQUIRE(is_session_registrar_valid(session.data));
    }

};

struct HasAnotherRegistrar : virtual Test::autocommitting_context {
    Fred::InfoRegistrarData data;
    HasSession session;
    HasConfig config;

    HasAnotherRegistrar() {
        const std::string reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(reg_handle).exec(ctx);
        data = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;

        session.data.registrar_id = data.id + 1;
        BOOST_REQUIRE(is_session_registrar_valid(session.data));

    }

};

struct HasContact : HasRegistrar {
    Fred::InfoContactData contact;

    HasContact() {
        const std::string contact_handle = "CONTACT1";
        Fred::CreateContact(contact_handle, registrar.handle).exec(ctx);
        contact = Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
    }

};

struct HasContactAndAnotherRegistrar : HasContact {
    Fred::InfoRegistrarData another_registrar;
    HasContactAndAnotherRegistrar() {
    }
};

struct HasContactWithStatusRequest : HasContact {
    const std::string status;

    HasContactWithStatusRequest(const std::string& _status)
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

struct HasContactWithStatus : HasContactWithStatusRequest {
    HasContactWithStatus(const std::string& _status)
    :   HasContactWithStatusRequest(_status)
    {
        Fred::PerformObjectStateRequest(contact.id).exec(ctx);
    }
};

struct HasContactWithServerUpdateProhibited : HasContactWithStatus {
    HasContactWithServerUpdateProhibited()
    :   HasContactWithStatus("serverUpdateProhibited")
    { }
};

struct HasContactWithServerTransferProhibited : HasContactWithStatus {
    HasContactWithServerTransferProhibited()
    :   HasContactWithStatus("serverTransferProhibited")
    { }
};

struct HasContactWithDeleteCandidate : HasContactWithStatus {
    HasContactWithDeleteCandidate()
    :   HasContactWithStatus("deleteCandidate")
    { }
};

struct HasContactWithDeleteCandidateRequest : HasContactWithStatusRequest {
    HasContactWithDeleteCandidateRequest()
    :   HasContactWithStatusRequest("deleteCandidate")
    { }
};

struct HasContactWithServerTransferProhibitedRequest : HasContactWithStatusRequest {
    HasContactWithServerTransferProhibitedRequest()
    :   HasContactWithStatusRequest("serverTransferProhibited")
    { }
};

struct HasContactWithServerUpdateProhibitedRequest : HasContactWithStatusRequest {
    HasContactWithServerUpdateProhibitedRequest()
    :   HasContactWithStatusRequest("serverUpdateProhibited")
    { }
};

#endif
