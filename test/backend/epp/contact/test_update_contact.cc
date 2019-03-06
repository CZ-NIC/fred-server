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
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/contact/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/update_contact.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <set>

namespace Test {

namespace {

Nullable<std::string> get_new_value(
        const ::Epp::Deletable<std::string>& change,
        const Nullable<std::string>& before)
{
    if (change == ::Epp::UpdateOperation::Action::set_value)
    {
        return Nullable<std::string>(*change);
    }
    if (change == ::Epp::UpdateOperation::Action::delete_value)
    {
        return Nullable<std::string>();
    }
    if (change == ::Epp::UpdateOperation::Action::no_operation)
    {
        return before;
    }
    throw std::runtime_error("unexpected operation");
}

std::string get_new_value(
        const ::Epp::Deletable<std::string>& change,
        const std::string& before)
{
    if (change == ::Epp::UpdateOperation::Action::set_value)
    {
        return *change;
    }
    if (change == ::Epp::UpdateOperation::Action::delete_value)
    {
        return std::string();
    }
    if (change == ::Epp::UpdateOperation::Action::no_operation)
    {
        return before;
    }
    throw std::runtime_error("unexpected operation");
}

std::string get_new_value(
        const ::Epp::Updateable<std::string>& change,
        const std::string& before)
{
    if (change == ::Epp::UpdateOperation::Action::set_value)
    {
        return *change;
    }
    if (change == ::Epp::UpdateOperation::Action::no_operation)
    {
        return before;
    }
    throw std::runtime_error("unexpected operation");
}

Optional<std::string> get_new_value(
        const ::Epp::Deletable<std::string>& change,
        const Optional<std::string>& before)
{
    if (change == ::Epp::UpdateOperation::Action::set_value)
    {
        return Optional<std::string>(*change);
    }
    if (change == ::Epp::UpdateOperation::Action::delete_value)
    {
        return Optional<std::string>();
    }
    if (change == ::Epp::UpdateOperation::Action::no_operation)
    {
        return before;
    }
    throw std::runtime_error("unexpected operation");
}

bool is_public(const boost::optional<::Epp::Contact::PrivacyPolicy>& publishability)
{
    if (static_cast<bool>(publishability))
    {
        switch (*publishability)
        {
            case ::Epp::Contact::PrivacyPolicy::show: return true;
            case ::Epp::Contact::PrivacyPolicy::hide: return false;
        }
        throw std::runtime_error("unknown PrivacyPolicy value");
    }
    throw std::runtime_error("missing PrivacyPolicy value");
}

enum class Data
{
    name,
    organization,
    address,
    telephone,
    fax,
    email,
    notify_email,
    vat,
    ident
};

template <Data> bool is_public(const ::Epp::Contact::ContactChange::Publishability&);

template <>
bool is_public<Data::name>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.name);
}

template <>
bool is_public<Data::organization>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.organization);
}

template <>
bool is_public<Data::address>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.address);
}

template <>
bool is_public<Data::telephone>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.telephone);
}

template <>
bool is_public<Data::fax>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.fax);
}

template <>
bool is_public<Data::email>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.email);
}

template <>
bool is_public<Data::notify_email>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.notify_email);
}

template <>
bool is_public<Data::vat>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.vat);
}

template <>
bool is_public<Data::ident>(const ::Epp::Contact::ContactChange::Publishability& disclose)
{
    return is_public(disclose.ident);
}


template <Data> bool is_public(const ::LibFred::InfoContactData&);

template <>
bool is_public<Data::name>(const ::LibFred::InfoContactData& info)
{
    return info.disclosename;
}

template <>
bool is_public<Data::organization>(const ::LibFred::InfoContactData& info)
{
    return info.discloseorganization;
}

template <>
bool is_public<Data::address>(const ::LibFred::InfoContactData& info)
{
    return info.discloseaddress;
}

template <>
bool is_public<Data::telephone>(const ::LibFred::InfoContactData& info)
{
    return info.disclosetelephone;
}

template <>
bool is_public<Data::fax>(const ::LibFred::InfoContactData& info)
{
    return info.disclosefax;
}

template <>
bool is_public<Data::email>(const ::LibFred::InfoContactData& info)
{
    return info.discloseemail;
}

template <>
bool is_public<Data::notify_email>(const ::LibFred::InfoContactData& info)
{
    return info.disclosenotifyemail;
}

template <>
bool is_public<Data::vat>(const ::LibFred::InfoContactData& info)
{
    return info.disclosevat;
}

template <>
bool is_public<Data::ident>(const ::LibFred::InfoContactData& info)
{
    return info.discloseident;
}

template <Data item>
bool get_new_discloseflag(const ::Epp::Updateable<::Epp::Contact::ContactChange::Publishability>& update,
                          const ::LibFred::InfoContactData& info_before)
{
    if (update == ::Epp::UpdateOperation::Action::set_value)
    {
        return is_public<item>(*update);
    }
    if (update == ::Epp::UpdateOperation::Action::no_operation)
    {
        return is_public<item>(info_before);
    }
    throw std::runtime_error("unexpected operation");
}

struct GetPersonalIdUnionFromContactIdent : boost::static_visitor<::LibFred::PersonalIdUnion>
{
    ::LibFred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf<::Epp::Contact::ContactIdentType::Op>& src)const
    {
        return ::LibFred::PersonalIdUnion::get_OP(src.value);
    }
    ::LibFred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf<::Epp::Contact::ContactIdentType::Pass>& src)const
    {
        return ::LibFred::PersonalIdUnion::get_PASS(src.value);
    }
    ::LibFred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf<::Epp::Contact::ContactIdentType::Ico>& src)const
    {
        return ::LibFred::PersonalIdUnion::get_ICO(src.value);
    }
    ::LibFred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf<::Epp::Contact::ContactIdentType::Mpsv>& src)const
    {
        return ::LibFred::PersonalIdUnion::get_MPSV(src.value);
    }
    ::LibFred::PersonalIdUnion operator()(const ::Epp::Contact::ContactIdentValueOf<::Epp::Contact::ContactIdentType::Birthday>& src)const
    {
        return ::LibFred::PersonalIdUnion::get_BIRTHDAY(src.value);
    }
};

LibFred::PersonalIdUnion get_ident(const ::Epp::Contact::ContactIdent& ident)
{
    return boost::apply_visitor(GetPersonalIdUnionFromContactIdent(), ident);
}

Nullable<std::string> get_ident_type(
        const ::Epp::Deletable<::Epp::Contact::ContactIdent>& ident,
        const Nullable<std::string>& previous_value)
{
    if (ident == ::Epp::UpdateOperation::Action::no_operation)
    {
        return previous_value;
    }
    if (ident == ::Epp::UpdateOperation::Action::delete_value)
    {
        return Nullable<std::string>();
    }
    if (ident == ::Epp::UpdateOperation::Action::set_value)
    {
        return get_ident(*ident).get_type();
    }
    throw std::runtime_error("unexpected operation");
}

Nullable<std::string> get_ident_value(
        const ::Epp::Deletable<::Epp::Contact::ContactIdent>& ident,
        const Nullable<std::string>& previous_value)
{
    if (ident == ::Epp::UpdateOperation::Action::no_operation)
    {
        return previous_value;
    }
    if (ident == ::Epp::UpdateOperation::Action::delete_value)
    {
        return Nullable<std::string>();
    }
    if (ident == ::Epp::UpdateOperation::Action::set_value)
    {
        return get_ident(*ident).get();
    }
    throw std::runtime_error("unexpected operation");
}

}//namespace Test::{anonymous}

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(UpdateContact)

namespace {

bool update_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

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

namespace {

bool update_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

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

namespace {

bool update_fail_wrong_registrar_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::unauthorized_registrar);
    return true;
}

}//namespace Test::{anonymous}

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

namespace {

bool update_fail_prohibiting_status1_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

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

namespace {

bool update_fail_prohibiting_status2_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

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

namespace {

bool update_fail_prohibiting_status_request_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

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
            const auto object_states = ::LibFred::GetObjectStates(contact_with_status_request_delete_candidate.data.id).exec(ctx);
            for (const auto& state : object_states)
            {
                object_states_after.insert(state.state_name);
            }
        }
        BOOST_CHECK(object_states_after.find(contact_with_status_request_delete_candidate.status) != object_states_after.end());
    }
}

namespace {

bool update_fail_nonexistent_country_code_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::contact_cc, ::Epp::Reason::country_notexist));
    return true;
}

template <typename T, typename S>
auto make_set_value(const S& src)
{
    return ::Epp::UpdateOperation::set_value(T(src));
}

auto make_no_operation()
{
    return ::Epp::UpdateOperation::no_operation();
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_country_code, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.name = make_no_operation();
    update_contact_input_data.organization = make_set_value<std::string>("Spol s r. o.");
    update_contact_input_data.address.country_code = make_set_value<std::string>("X123Z");
    update_contact_input_data.telephone = make_no_operation();
    update_contact_input_data.fax = make_no_operation();
    update_contact_input_data.email = make_no_operation();
    update_contact_input_data.notify_email = make_no_operation();
    update_contact_input_data.vat = make_no_operation();
    update_contact_input_data.ident = make_no_operation();
    ::Epp::Contact::ContactChange::Publishability disclose;
    disclose.name = ::Epp::Contact::PrivacyPolicy::show;
    disclose.organization = ::Epp::Contact::PrivacyPolicy::show;
    disclose.address = ::Epp::Contact::PrivacyPolicy::show;
    disclose.telephone = ::Epp::Contact::PrivacyPolicy::show;
    disclose.fax = ::Epp::Contact::PrivacyPolicy::show;
    disclose.email = ::Epp::Contact::PrivacyPolicy::show;
    disclose.notify_email = ::Epp::Contact::PrivacyPolicy::show;
    disclose.vat = ::Epp::Contact::PrivacyPolicy::show;
    disclose.ident = ::Epp::Contact::PrivacyPolicy::show;
    update_contact_input_data.disclose = make_set_value<::Epp::Contact::ContactChange::Publishability>(disclose);

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

namespace {

bool update_fail_address_cant_be_undisclosed_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(update_fail_address_cant_be_undisclosed, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.name = make_no_operation();
    update_contact_input_data.organization = make_set_value<std::string>("");
    update_contact_input_data.telephone = make_no_operation();
    update_contact_input_data.fax = make_no_operation();
    update_contact_input_data.email = make_no_operation();
    update_contact_input_data.notify_email = make_no_operation();
    update_contact_input_data.vat = make_no_operation();
    update_contact_input_data.ident = make_no_operation();
    ::Epp::Contact::ContactChange::Publishability disclose;
    disclose.name = ::Epp::Contact::PrivacyPolicy::show;
    disclose.organization = ::Epp::Contact::PrivacyPolicy::show;
    disclose.address = ::Epp::Contact::PrivacyPolicy::hide;
    disclose.telephone = ::Epp::Contact::PrivacyPolicy::hide;
    disclose.fax = ::Epp::Contact::PrivacyPolicy::hide;
    disclose.email = ::Epp::Contact::PrivacyPolicy::hide;
    disclose.notify_email = ::Epp::Contact::PrivacyPolicy::hide;
    disclose.vat = ::Epp::Contact::PrivacyPolicy::hide;
    disclose.ident = ::Epp::Contact::PrivacyPolicy::hide;
    update_contact_input_data.disclose = make_set_value<::Epp::Contact::ContactChange::Publishability>(disclose);

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

namespace {

void check_equal(
    const ::LibFred::InfoContactData& info_before,
    const ::Epp::Contact::ContactChange& update,
    const ::LibFred::InfoContactData& info_after)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(info_after.handle), info_before.handle);
    BOOST_CHECK_EQUAL(info_after.name, get_new_value(update.name, info_before.name));
    BOOST_CHECK_EQUAL(info_after.organization, get_new_value(update.organization, info_before.organization));
    BOOST_CHECK_EQUAL(
            info_after.place.get_value_or_default().street1,
            get_new_value(update.address.street[0], info_before.place.get_value_or_default().street1));
    BOOST_CHECK_EQUAL(
            info_after.place.get_value_or_default().street2,
            get_new_value(update.address.street[1], info_before.place.get_value_or_default().street2));
    BOOST_CHECK_EQUAL(
            info_after.place.get_value_or_default().street3,
            get_new_value(update.address.street[2], info_before.place.get_value_or_default().street3));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().city,
                      get_new_value(update.address.city, info_before.place.get_value_or_default().city));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().postalcode,
                      get_new_value(update.address.postal_code, info_before.place.get_value_or_default().postalcode));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().stateorprovince,
                      get_new_value(update.address.state_or_province, info_before.place.get_value_or_default().stateorprovince));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().country,
                      get_new_value(update.address.country_code, info_before.place.get_value_or_default().country));
    if (update.mailing_address == ::Epp::UpdateOperation::Action::set_value)
    {
        const auto addresses_itr = info_after.addresses.find(::LibFred::ContactAddressType::MAILING);
        const bool mailing_address_presents_after = addresses_itr != info_after.addresses.end();
        BOOST_CHECK(mailing_address_presents_after);
        if (mailing_address_presents_after)
        {
            const auto mailing_address = *update.mailing_address;
            BOOST_CHECK(!addresses_itr->second.company_name.isset());
            BOOST_CHECK(static_cast<bool>(mailing_address.street[0]));
            if (static_cast<bool>(mailing_address.street[0]))
            {
                BOOST_CHECK_EQUAL(*(mailing_address.street[0]), addresses_itr->second.street1);
            }
            BOOST_CHECK_EQUAL(static_cast<bool>(mailing_address.street[1]), addresses_itr->second.street2.isset());
            if ((static_cast<bool>(mailing_address.street[1])) && (addresses_itr->second.street2.isset()))
            {
                BOOST_CHECK_EQUAL(*(mailing_address.street[1]), addresses_itr->second.street2.get_value());
            }
            BOOST_CHECK_EQUAL(static_cast<bool>(mailing_address.street[2]), addresses_itr->second.street3.isset());
            if ((static_cast<bool>(mailing_address.street[2])) && (addresses_itr->second.street3.isset()))
            {
                BOOST_CHECK_EQUAL(*(mailing_address.street[2]), addresses_itr->second.street3.get_value());
            }
            BOOST_CHECK(static_cast<bool>(mailing_address.city));
            if (static_cast<bool>(mailing_address.city))
            {
                BOOST_CHECK_EQUAL(*mailing_address.city, addresses_itr->second.city);
            }
            BOOST_CHECK_EQUAL(static_cast<bool>(mailing_address.state_or_province), addresses_itr->second.stateorprovince.isset());
            if ((static_cast<bool>(mailing_address.state_or_province)) && (addresses_itr->second.stateorprovince.isset()))
            {
                BOOST_CHECK_EQUAL(*mailing_address.state_or_province, addresses_itr->second.stateorprovince.get_value());
            }
            BOOST_CHECK(static_cast<bool>(mailing_address.postal_code));
            if (static_cast<bool>(mailing_address.postal_code))
            {
                BOOST_CHECK_EQUAL(*mailing_address.postal_code, addresses_itr->second.postalcode);
            }
            BOOST_CHECK(static_cast<bool>(mailing_address.country_code));
            if (static_cast<bool>(mailing_address.country_code))
            {
                BOOST_CHECK_EQUAL(*mailing_address.country_code, addresses_itr->second.country);
            }
        }
    }
    else if (update.mailing_address == ::Epp::UpdateOperation::Action::delete_value)
    {
        const auto addresses_after_itr =
                info_after.addresses.find(::LibFred::ContactAddressType::MAILING);
        const bool mailing_address_presents_after = addresses_after_itr != info_after.addresses.end();
        BOOST_CHECK(!mailing_address_presents_after);
    }
    else if (update.mailing_address == ::Epp::UpdateOperation::Action::no_operation)
    {
        const auto addresses_before_itr =
                info_before.addresses.find(::LibFred::ContactAddressType::MAILING);
        const bool mailing_address_presents_before = addresses_before_itr != info_before.addresses.end();
        const auto addresses_after_itr =
                info_after.addresses.find(::LibFred::ContactAddressType::MAILING);
        const bool mailing_address_presents_after = addresses_after_itr != info_after.addresses.end();
        BOOST_CHECK_EQUAL(mailing_address_presents_before, mailing_address_presents_after);
        if (mailing_address_presents_before && mailing_address_presents_after)
        {
            BOOST_CHECK_EQUAL(addresses_before_itr->second.company_name.isset(),
                              addresses_after_itr->second.company_name.isset());
            if (addresses_before_itr->second.company_name.isset() && addresses_after_itr->second.company_name.isset())
            {
                BOOST_CHECK_EQUAL(addresses_before_itr->second.company_name.get_value(),
                                  addresses_after_itr->second.company_name.get_value());
            }
            BOOST_CHECK_EQUAL(addresses_before_itr->second.street1, addresses_after_itr->second.street1);
            BOOST_CHECK_EQUAL(addresses_before_itr->second.street2.isset(),
                              addresses_after_itr->second.street2.isset());
            if (addresses_before_itr->second.street2.isset() && addresses_after_itr->second.street2.isset())
            {
                BOOST_CHECK_EQUAL(addresses_before_itr->second.street2.get_value(),
                                  addresses_after_itr->second.street2.get_value());
            }
            BOOST_CHECK_EQUAL(addresses_before_itr->second.street3.isset(),
                              addresses_after_itr->second.street3.isset());
            if (addresses_before_itr->second.street3.isset() && addresses_after_itr->second.street3.isset())
            {
                BOOST_CHECK_EQUAL(addresses_before_itr->second.street3.get_value(),
                                  addresses_after_itr->second.street3.get_value());
            }
            BOOST_CHECK_EQUAL(addresses_before_itr->second.city, addresses_after_itr->second.city);
            BOOST_CHECK_EQUAL(addresses_before_itr->second.stateorprovince.isset(),
                              addresses_after_itr->second.stateorprovince.isset());
            if (addresses_before_itr->second.stateorprovince.isset() && addresses_after_itr->second.stateorprovince.isset())
            {
                BOOST_CHECK_EQUAL(addresses_before_itr->second.stateorprovince.get_value(),
                                  addresses_after_itr->second.stateorprovince.get_value());
            }
            BOOST_CHECK_EQUAL(addresses_before_itr->second.postalcode, addresses_after_itr->second.postalcode);
            BOOST_CHECK_EQUAL(addresses_before_itr->second.country, addresses_after_itr->second.country);
        }
    }
    BOOST_CHECK_EQUAL(info_after.telephone, get_new_value(update.telephone, info_before.telephone));
    BOOST_CHECK_EQUAL(info_after.fax, get_new_value(update.fax, info_before.fax));
    BOOST_CHECK_EQUAL(info_after.email, get_new_value(update.email, info_before.email));
    BOOST_CHECK_EQUAL(info_after.notifyemail, get_new_value(update.notify_email, info_before.notifyemail));
    BOOST_CHECK_EQUAL(info_after.vat, get_new_value(update.vat, info_before.vat));
    BOOST_CHECK_EQUAL(info_after.ssn, get_ident_value(update.ident, info_before.ssn));
    BOOST_CHECK_EQUAL(info_after.ssntype, get_ident_type(update.ident, info_before.ssntype));
    BOOST_CHECK_EQUAL(info_after.authinfopw, get_new_value(update.authinfopw, info_before.authinfopw));
    BOOST_CHECK_EQUAL(info_after.disclosename,
                      get_new_discloseflag<Data::name>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.discloseorganization,
                      get_new_discloseflag<Data::organization>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.discloseaddress,
                      get_new_discloseflag<Data::address>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.disclosetelephone,
                      get_new_discloseflag<Data::telephone>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.disclosefax,
                      get_new_discloseflag<Data::fax>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.discloseemail,
                      get_new_discloseflag<Data::email>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.disclosevat,
                      get_new_discloseflag<Data::vat>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.discloseident,
                      get_new_discloseflag<Data::ident>(update.disclose, info_before));
    BOOST_CHECK_EQUAL(info_after.disclosenotifyemail,
                      get_new_discloseflag<Data::notify_email>(update.disclose, info_before));
}

static const ::Epp::Updateable<::Epp::Contact::PrivacyPolicy> leave_privacy_unchanged =
        ::Epp::UpdateOperation::no_operation();

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(update_ok_full_data, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.add_additional_data().drop_mailing_address();

    ::Epp::Contact::update_contact(
            ctx,
            contact.data.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);

    check_equal(contact.data, update_contact_input_data, ::LibFred::InfoContactByHandle(contact.data.handle).exec(ctx).info_contact_data);
}

BOOST_FIXTURE_TEST_CASE(update_ok_streets, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;

    auto info_before = contact.data;
    update_contact_input_data.add_additional_data().drop_mailing_address();
    update_contact_input_data.address.street[0] = make_set_value<std::string>("street1");
    update_contact_input_data.address.street[1] = make_set_value<std::string>("street2");
    update_contact_input_data.address.street[2] = make_set_value<std::string>("street3");
    ::Epp::Contact::update_contact(
            ctx,
            info_before.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);
    auto info_after = ::LibFred::InfoContactByHandle(info_before.handle).exec(ctx).info_contact_data;
    check_equal(info_before, update_contact_input_data, info_after);

    info_before = info_after;
    update_contact_input_data.address.street[2] = ::Epp::UpdateOperation::delete_value();
    BOOST_CHECK_EQUAL(*(update_contact_input_data.address.street[0]), info_before.place.get_value().street1);
    BOOST_CHECK_EQUAL(*(update_contact_input_data.address.street[1]), info_before.place.get_value().street2);
    ::Epp::Contact::update_contact(
            ctx,
            info_before.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);
    info_after = ::LibFred::InfoContactByHandle(info_before.handle).exec(ctx).info_contact_data;
    check_equal(info_before, update_contact_input_data, info_after);

    info_before = info_after;
    update_contact_input_data.address.street[1] = ::Epp::UpdateOperation::delete_value();
    BOOST_CHECK_EQUAL(*(update_contact_input_data.address.street[0]), info_before.place.get_value().street1);
    ::Epp::Contact::update_contact(
            ctx,
            info_before.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);
    info_after = ::LibFred::InfoContactByHandle(info_before.handle).exec(ctx).info_contact_data;
    check_equal(info_before, update_contact_input_data, info_after);

    info_before = info_after;
    update_contact_input_data.address.street[0] = ::Epp::UpdateOperation::delete_value();
    ::Epp::Contact::update_contact(
            ctx,
            info_before.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);
    info_after = ::LibFred::InfoContactByHandle(info_before.handle).exec(ctx).info_contact_data;
    check_equal(info_before, update_contact_input_data, info_after);
}

BOOST_FIXTURE_TEST_CASE(update_mailing_address_ok, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    DefaultUpdateContactInputData update_contact_input_data;
    update_contact_input_data.update_mailing_address();

    ::Epp::Contact::update_contact(
            ctx,
            contact.data.handle,
            update_contact_input_data,
            DefaultUpdateContactConfigData(),
            session.data);

    check_equal(contact.data, update_contact_input_data, ::LibFred::InfoContactByHandle(contact.data.handle).exec(ctx).info_contact_data);
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

    check_equal(contact_with_status_request_server_transfer_prohibited.data, update_contact_input_data, ::LibFred::InfoContactByHandle(contact_with_status_request_server_transfer_prohibited.data.handle).exec(ctx).info_contact_data);

    // now object has the state server_transfer_prohibited itself
    {
        std::set<std::string> object_states_after;
        {
            const auto object_states = ::LibFred::GetObjectStates(contact_with_status_request_server_transfer_prohibited.data.id).exec(ctx);
            for (const auto& state : object_states)
            {
                object_states_after.insert(state.state_name);
            }
        }
        BOOST_CHECK(object_states_after.find(contact_with_status_request_server_transfer_prohibited.status) != object_states_after.end());
    }
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact/UpdateContact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

} // namespace Test
