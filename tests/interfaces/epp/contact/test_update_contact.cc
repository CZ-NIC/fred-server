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

#include "tests/interfaces/epp/fixture.h"
#include "tests/interfaces/epp/contact/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/contact/update_contact.h"
#include "src/epp/impl/disclose_policy.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>
#include <set>

namespace Test {

namespace {

template < ::Epp::Contact::ContactDisclose::Item::Enum ITEM>
bool updated(const ::Epp::Contact::ContactChange& _update, bool _before)
{
    return _update.disclose.is_initialized()
           ? _update.disclose->should_be_disclosed< ITEM >(::Epp::is_the_default_policy_to_disclose())
           : _before;
}

std::string get_new_value(
        const boost::optional<std::string>& change,
        const std::string& before)
{
    if (::Epp::Contact::ContactChange::does_value_mean< ::Epp::Contact::ContactChange::Value::to_set >(change)) {
        return ::Epp::Contact::ContactChange::get_value(change);
    }
    return before;
}

std::string get_new_value(
        const boost::optional<Nullable<std::string> >& change,
        const std::string& before)
{
    if (::Epp::Contact::ContactChange::does_value_mean< ::Epp::Contact::ContactChange::Value::to_set >(change)) {
        return ::Epp::Contact::ContactChange::get_value(change);
    }
    if (::Epp::Contact::ContactChange::does_value_mean< ::Epp::Contact::ContactChange::Value::to_delete >(change)) {
        return std::string();
    }
    return before;
}

Optional<std::string> get_new_value(
        const boost::optional<Nullable<std::string> >& change,
        const Optional<std::string>& before)
{
    if (::Epp::Contact::ContactChange::does_value_mean< ::Epp::Contact::ContactChange::Value::to_set >(change)) {
        return ::Epp::Contact::ContactChange::get_value(change);
    }
    if (::Epp::Contact::ContactChange::does_value_mean< ::Epp::Contact::ContactChange::Value::to_delete >(change)) {
        return Optional< std::string >();
    }
    return before;
}

Nullable<std::string> get_new_value(
        const boost::optional<Nullable<std::string> >& change,
        const Nullable<std::string>& before)
{
    if (::Epp::Contact::ContactChange::does_value_mean< ::Epp::Contact::ContactChange::Value::to_set >(change)) {
        return ::Epp::Contact::ContactChange::get_value(change);
    }
    if (::Epp::Contact::ContactChange::does_value_mean< ::Epp::Contact::ContactChange::Value::to_delete >(change)) {
        return Nullable< std::string >();
    }
    return before;
}

struct GetPersonalIdUnionFromContactIdent:boost::static_visitor<Fred::PersonalIdUnion>
{
    Fred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf< ::Epp::Contact::ContactIdentType::Op >& src)const
    {
        return Fred::PersonalIdUnion::get_OP(src.value);
    }
    Fred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf< ::Epp::Contact::ContactIdentType::Pass >& src)const
    {
        return Fred::PersonalIdUnion::get_PASS(src.value);
    }
    Fred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf< ::Epp::Contact::ContactIdentType::Ico >& src)const
    {
        return Fred::PersonalIdUnion::get_ICO(src.value);
    }
    Fred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf< ::Epp::Contact::ContactIdentType::Mpsv >& src)const
    {
        return Fred::PersonalIdUnion::get_MPSV(src.value);
    }
    Fred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf< ::Epp::Contact::ContactIdentType::Birthday >& src)const
    {
        return Fred::PersonalIdUnion::get_BIRTHDAY(src.value);
    }
};

Fred::PersonalIdUnion get_ident(const ::Epp::Contact::ContactIdent& ident)
{
    return boost::apply_visitor(GetPersonalIdUnionFromContactIdent(), ident);
}

Nullable<std::string> get_ident_type(
        const boost::optional< boost::optional< ::Epp::Contact::ContactIdent > >& ident,
        const Nullable<std::string>& previous_value)
{
    if (ident == boost::none)
    {
        return previous_value;
    }
    if (*ident == boost::none)
    {
        return Nullable<std::string>();
    }
    return get_ident(**ident).get_type();
}

Nullable<std::string> get_ident_value(
        const boost::optional< boost::optional< ::Epp::Contact::ContactIdent > >& ident,
        const Nullable<std::string>& previous_value)
{
    if (ident == boost::none)
    {
        return previous_value;
    }
    if (*ident == boost::none)
    {
        return Nullable<std::string>();
    }
    return get_ident(**ident).get();
}

} // namespace Test::{anonymous}

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(UpdateContact)

bool update_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::update_contact(
                    ctx,
                    ValidHandle().handle,
                    DefaultUpdateContactInputData(),
                    DefaultUpdateContactConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            update_invalid_registrar_id_exception);
}

bool update_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::update_contact(
                    ctx,
                    contact.data.handle + "abc",
                    DefaultUpdateContactInputData(),
                    DefaultUpdateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            update_fail_nonexistent_handle_exception);
}

bool update_fail_wrong_registrar_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_wrong_registrar, supply_ctx<HasRegistrarWithSessionAndContactOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::update_contact(
                    ctx,
                    contact_of_different_registrar.data.handle,
                    DefaultUpdateContactInputData(),
                    DefaultUpdateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            update_fail_wrong_registrar_exception);
}

bool update_fail_prohibiting_status1_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status1, supply_ctx<HasRegistrarWithSession>)
{
    ContactWithStatusRequestServerUpdateProhibited contact_with_status_request_server_update_prohibited(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::update_contact(
                    ctx,
                    contact_with_status_request_server_update_prohibited.data.handle,
                    DefaultUpdateContactInputData(),
                    DefaultUpdateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            update_fail_prohibiting_status1_exception);
}

bool update_fail_prohibiting_status2_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status2, supply_ctx<HasRegistrarWithSession>)
{
    ContactWithStatusDeleteCandidate contact_with_status_delete_candidate(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::update_contact(
            ctx,
            contact_with_status_delete_candidate.data.handle,
            DefaultUpdateContactInputData(),
            DefaultUpdateContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        update_fail_prohibiting_status2_exception
    );
}

bool update_fail_prohibiting_status_request_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status_request, supply_ctx<HasRegistrarWithSession>)
{
    ContactWithStatusRequestDeleteCandidate contact_with_status_request_delete_candidate(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::update_contact(
                    ctx,
                    contact_with_status_request_delete_candidate.data.handle,
                    DefaultUpdateContactInputData().add_additional_data(),
                    DefaultUpdateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            update_fail_prohibiting_status_request_exception);

    // now object has the state deleteCandidate itself
    {
        std::set<std::string> object_states_after;
        {
            BOOST_FOREACH (const Fred::ObjectStateData& state, Fred::GetObjectStates(contact_with_status_request_delete_candidate.data.id).exec(ctx))
            {
                object_states_after.insert(state.state_name);
            }
        }

        BOOST_CHECK(object_states_after.find(contact_with_status_request_delete_candidate.status) != object_states_after.end());
    }
}

bool update_fail_nonexistent_country_code_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::contact_cc, ::Epp::Reason::country_notexist));
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_country_code, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.organization = "Spol s r. o.";
    update_contact_input_data.country_code = "X123Z"; // <- !!!
    update_contact_input_data.disclose     = get_all_items();

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::update_contact(
                    ctx,
                    contact.data.handle,
                    update_contact_input_data,
                    DefaultUpdateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            update_fail_nonexistent_country_code_exception);
}

bool update_fail_address_cant_be_undisclosed_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(update_fail_address_cant_be_undisclosed, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.organization = "";
    update_contact_input_data.disclose = get_all_items(false); // address <- !!!

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::update_contact(
                    ctx,
                    contact.data.handle,
                    update_contact_input_data,
                    DefaultUpdateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            update_fail_address_cant_be_undisclosed_exception);
}

static void check_equal(
    const Fred::InfoContactData& info_before,
    const ::Epp::Contact::ContactChange& update,
    const Fred::InfoContactData& info_after)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(info_after.handle), info_before.handle);
    BOOST_CHECK_EQUAL(info_after.name, get_new_value(update.name, info_before.name));
    BOOST_CHECK_EQUAL(info_after.organization, get_new_value(update.organization, info_before.organization));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().street1,
        0 < update.streets.size() ? get_new_value(update.streets[0], info_before.place.get_value_or_default().street1)
                                  : "");
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().street2,
        1 < update.streets.size() ? get_new_value(update.streets[1], info_before.place.get_value_or_default().street2)
                                  : "");
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().street3,
        2 < update.streets.size() ? get_new_value(update.streets[2], info_before.place.get_value_or_default().street3)
                                  : "");
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().city,
                      get_new_value(update.city, info_before.place.get_value_or_default().city));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().postalcode,
                      get_new_value(update.postal_code, info_before.place.get_value_or_default().postalcode));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().stateorprovince,
                      get_new_value(update.state_or_province, info_before.place.get_value_or_default().stateorprovince));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().country,
                      get_new_value(update.country_code, info_before.place.get_value_or_default().country));
    BOOST_CHECK_EQUAL(info_after.telephone, get_new_value(update.telephone, info_before.telephone));
    BOOST_CHECK_EQUAL(info_after.fax, get_new_value(update.fax, info_before.fax));
    BOOST_CHECK_EQUAL(info_after.email, get_new_value(update.email, info_before.email));
    BOOST_CHECK_EQUAL(info_after.notifyemail, get_new_value(update.notify_email, info_before.notifyemail));
    BOOST_CHECK_EQUAL(info_after.vat, get_new_value(update.vat, info_before.vat));
    BOOST_CHECK_EQUAL(info_after.ssn, get_ident_value(update.ident, info_before.ssn));
    BOOST_CHECK_EQUAL(info_after.ssntype, get_ident_type(update.ident, info_before.ssntype));
    BOOST_CHECK_EQUAL(info_after.authinfopw, get_new_value(update.authinfopw, info_before.authinfopw));
    BOOST_CHECK_EQUAL(info_after.disclosename,
                      updated< ::Epp::Contact::ContactDisclose::Item::name         >(update, info_before.disclosename));
    BOOST_CHECK_EQUAL(info_after.discloseorganization,
                      updated< ::Epp::Contact::ContactDisclose::Item::organization >(update, info_before.discloseorganization));
    BOOST_CHECK_EQUAL(info_after.discloseaddress,
                      updated< ::Epp::Contact::ContactDisclose::Item::address      >(update, info_before.discloseaddress));
    BOOST_CHECK_EQUAL(info_after.disclosetelephone,
                      updated< ::Epp::Contact::ContactDisclose::Item::telephone    >(update, info_before.disclosetelephone));
    BOOST_CHECK_EQUAL(info_after.disclosefax,
                      updated< ::Epp::Contact::ContactDisclose::Item::fax          >(update, info_before.disclosefax));
    BOOST_CHECK_EQUAL(info_after.discloseemail,
                      updated< ::Epp::Contact::ContactDisclose::Item::email        >(update, info_before.discloseemail));
    BOOST_CHECK_EQUAL(info_after.disclosevat,
                      updated< ::Epp::Contact::ContactDisclose::Item::vat          >(update, info_before.disclosevat));
    BOOST_CHECK_EQUAL(info_after.discloseident,
                      updated< ::Epp::Contact::ContactDisclose::Item::ident        >(update, info_before.discloseident));
    BOOST_CHECK_EQUAL(info_after.disclosenotifyemail,
                      updated< ::Epp::Contact::ContactDisclose::Item::notify_email >(update, info_before.disclosenotifyemail));
}

BOOST_FIXTURE_TEST_CASE(update_ok_full_data, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.add_additional_data();

    ::Epp::Contact::update_contact(
            ctx,
            contact.data.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);

    check_equal(contact.data, update_contact_input_data, Fred::InfoContactByHandle(contact.data.handle).exec(ctx).info_contact_data);
}

BOOST_FIXTURE_TEST_CASE(update_ok_states_are_upgraded, supply_ctx<HasRegistrarWithSession>)
{
    ContactWithStatusRequestServerTransferProhibited contact_with_status_request_server_transfer_prohibited(ctx, registrar.data.handle);

    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.add_additional_data();

    ::Epp::Contact::update_contact(
            ctx,
            contact_with_status_request_server_transfer_prohibited.data.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);

    check_equal(contact_with_status_request_server_transfer_prohibited.data, update_contact_input_data, Fred::InfoContactByHandle(contact_with_status_request_server_transfer_prohibited.data.handle).exec(ctx).info_contact_data);

    // now object has the state server_transfer_prohibited itself
    {
        std::set<std::string> object_states_after;
        {
            BOOST_FOREACH (const Fred::ObjectStateData& state, Fred::GetObjectStates(contact_with_status_request_server_transfer_prohibited.data.id).exec(ctx))
            {
                object_states_after.insert(state.state_name);
            }
        }

        BOOST_CHECK(object_states_after.find(contact_with_status_request_server_transfer_prohibited.status) != object_states_after.end());
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
