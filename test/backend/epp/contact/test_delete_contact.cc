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
#include "test/backend/epp/contact/fixture.hh"
#include "test/backend/epp/domain/fixture.hh"
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/keyset/fixture.hh"
#include "test/backend/epp/nsset/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/delete_contact.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/poll/poll_request.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(DeleteContact)

bool delete_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::delete_contact(
            ctx,
            ValidHandle().handle,
            DefaultDeleteContactConfigData(),
            session_with_unauthenticated_registrar.data
        ),
        ::Epp::EppResponseFailure,
        delete_invalid_registrar_id_exception
    );
}

bool delete_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::delete_contact(
            ctx,
            NonexistentHandle().handle,
            DefaultDeleteContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        delete_fail_nonexistent_handle_exception
    );
}

bool delete_fail_wrong_registrar_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_wrong_registrar, supply_ctx<HasRegistrarWithSessionAndContactOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::delete_contact(
            ctx,
            contact_of_different_registrar.data.handle,
            DefaultDeleteContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        delete_fail_wrong_registrar_exception
    );
}

bool delete_fail_prohibiting_status1_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status1, supply_ctx<HasRegistrarWithSession>)
{
    ContactWithStatusServerUpdateProhibited contact_with_status_server_update_prohibited(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::delete_contact(
                    ctx,
                    contact_with_status_server_update_prohibited.data.handle,
                    DefaultDeleteContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_prohibiting_status1_exception);
}

bool delete_fail_prohibiting_status2_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status2, supply_ctx<HasRegistrarWithSession>)
{
    ContactWithStatusDeleteCandidate contact_with_status_delete_candidate(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::delete_contact(
                    ctx,
                    contact_with_status_delete_candidate.data.handle,
                    DefaultDeleteContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_prohibiting_status2_exception);
}

BOOST_FIXTURE_TEST_CASE(delete_ok_states_are_upgraded, supply_ctx<HasRegistrarWithSession>)
{
    ContactWithStatusRequestServerTransferProhibited contact_with_status_request_server_transfer_prohibited(ctx, registrar.data.handle);

    ::Epp::Contact::delete_contact(
        ctx,
        contact_with_status_request_server_transfer_prohibited.data.handle,
        DefaultDeleteContactConfigData(),
        session.data
    );

    /* now object has the state server_transfer_prohibited request itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const ::LibFred::ObjectStateData& state, ::LibFred::GetObjectStates(contact_with_status_request_server_transfer_prohibited.data.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), contact_with_status_request_server_transfer_prohibited.status )
            !=
            object_states_after.end()
        );
    }
}

bool delete_fail_owning_domain_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_owning_domain, supply_ctx<Domain::HasRegistrarWithSessionAndDomain>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::delete_contact(
            ctx,
            domain.registrant.data.handle,
            DefaultDeleteContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        delete_fail_owning_domain_exception
    );
}

bool delete_fail_administrating_domain_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_administrating_domain, supply_ctx<HasRegistrarWithSession>)
{
    Contact contact(ctx, registrar.data.handle);
    Contact admin_contact(ctx, registrar.data.handle, "ADMINCONTACT");

    ::LibFred::CreateDomain("domain.cz", registrar.data.handle, contact.data.handle)
            .set_admin_contacts(boost::assign::list_of(admin_contact.data.handle))
            .exec(ctx);


    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::delete_contact(
            ctx,
            admin_contact.data.handle,
            DefaultDeleteContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        delete_fail_administrating_domain_exception
    );
}

bool delete_fail_linked_nsset_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_nsset, supply_ctx<HasRegistrarWithSession>)
{
    Nsset::NssetWithTechContact nsset(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::delete_contact(
                    ctx,
                    nsset.tech_contact.data.handle,
                    DefaultDeleteContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_linked_nsset_exception);
}

bool delete_fail_linked_keyset_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_keyset, supply_ctx<HasRegistrarWithSession>)
{
    Keyset::KeysetWithTechContact keyset(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::delete_contact(
                    ctx,
                    keyset.tech_contact.data.handle,
                    DefaultDeleteContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_linked_keyset_exception);
}

BOOST_FIXTURE_TEST_CASE(delete_ok, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    ::Epp::Contact::delete_contact(
        ctx,
        contact.data.handle,
        DefaultDeleteContactConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoContactHistoryById(contact.data.id).exec(ctx).rbegin()->info_contact_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(ok_system_registrar_different_contact, supply_ctx<HasSystemRegistrarWithSessionAndDifferentRegistrar>)
{
    Contact contact(ctx, different_registrar.data.handle);

    ::Epp::Contact::delete_contact(
        ctx,
        contact.data.handle,
        DefaultDeleteContactConfigData(),
        session.data
    );

    const auto info_contact_data = ::LibFred::InfoContactHistoryById(contact.data.id).exec(ctx).rbegin()->info_contact_data;
    BOOST_CHECK_EQUAL(
        info_contact_data.delete_time.isnull(),
        false
    );

    try
    {
        const auto poll_msg = ::Epp::Poll::poll_request(ctx, different_registrar.data.id);
        const auto event = boost::get<::Epp::Poll::MessageEvent>(poll_msg.message);
        const auto delete_event = boost::get<::Epp::Poll::MessageEvent::Data<::Epp::Poll::MessageEvent::DeleteContact>>(event.message);
        BOOST_CHECK_EQUAL(delete_event.handle, contact.data.handle);
        BOOST_CHECK_EQUAL(delete_event.date, info_contact_data.delete_time.get_value().date());
    }
    catch (...)
    {
        BOOST_FAIL("Unable to get delete contact poll message");
    }
}

BOOST_FIXTURE_TEST_CASE(ok_system_registrar_contact_lock_delete, supply_ctx<HasSystemRegistrarWithSessionAndDifferentRegistrar>)
{
    ContactWithStatusServerDeleteProhibited contact(ctx, different_registrar.data.handle);

    ::Epp::Contact::delete_contact(
        ctx,
        contact.data.handle,
        DefaultDeleteContactConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoContactHistoryById(contact.data.id).exec(ctx).rbegin()->info_contact_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(ok_system_registrar_contact_lock_update, supply_ctx<HasSystemRegistrarWithSessionAndDifferentRegistrar>)
{
    ContactWithStatusServerUpdateProhibited contact(ctx, different_registrar.data.handle);

    ::Epp::Contact::delete_contact(
        ctx,
        contact.data.handle,
        DefaultDeleteContactConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoContactHistoryById(contact.data.id).exec(ctx).rbegin()->info_contact_data.delete_time.isnull(),
        false
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
