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

#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/contact/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/create_contact.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(CreateContact)

namespace {

template <typename T>
bool is_public(const ::Epp::Contact::Hideable<T>& hideable)
{
    if (hideable.is_publishability_specified())
    {
        if (hideable.is_public())
        {
            return true;
        }
        if (hideable.is_private())
        {
            return false;
        }
        throw std::runtime_error("unexpected publishability specified");
    }
    throw std::runtime_error("no publishability specified");
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

boost::optional<::LibFred::PersonalIdUnion> get_ident(const boost::optional<::Epp::Contact::ContactIdent>& ident)
{
    return static_cast<bool>(ident) ? boost::apply_visitor(GetPersonalIdUnionFromContactIdent(), *ident)
                                    : boost::optional<::LibFred::PersonalIdUnion>();
}

std::string get_ident_type(const boost::optional<::Epp::Contact::ContactIdent>& ident)
{
    const boost::optional<::LibFred::PersonalIdUnion> personal_id = get_ident(ident);
    return static_cast<bool>(personal_id) ? personal_id->get_type()
                                          : std::string();
}

std::string get_ident_value(const boost::optional<::Epp::Contact::ContactIdent>& ident)
{
    const boost::optional<::LibFred::PersonalIdUnion> personal_id = get_ident(ident);
    return static_cast<bool>(personal_id) ? personal_id->get()
                                          : std::string();
}

template <class T>
T get_value(const boost::optional<T>& src)
{
    return static_cast<bool>(src) ? *src : T();
}

template <class T>
T get_value(const Optional<T>& src)
{
    return src.isset() ? src.get_value() : T();
}

template <class T>
T get_value(const Nullable<T>& src)
{
    return src.isnull() ? T() : src.get_value();
}

void check_equal(const ::Epp::Contact::CreateContactInputData &create_data, const ::LibFred::InfoContactData &info_data)
{
    BOOST_CHECK_EQUAL(*create_data.name, get_value(info_data.name));
    BOOST_CHECK_EQUAL(*create_data.organization, get_value(info_data.organization));

    BOOST_CHECK_EQUAL(create_data.address->street[0], info_data.place.get_value_or_default().street1);
    BOOST_CHECK_EQUAL(create_data.address->street[1], info_data.place.get_value_or_default().street2.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.address->street[2], info_data.place.get_value_or_default().street3.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.address->city, info_data.place.get_value_or_default().city);
    BOOST_CHECK_EQUAL(create_data.address->postal_code, info_data.place.get_value_or_default().postalcode);
    BOOST_CHECK_EQUAL(create_data.address->state_or_province, get_value(info_data.place.get_value_or_default().stateorprovince));
    BOOST_CHECK_EQUAL(create_data.address->country_code, info_data.place.get_value_or_default().country);
    BOOST_CHECK_EQUAL(*create_data.telephone, get_value(info_data.telephone));
    BOOST_CHECK_EQUAL(*create_data.fax, get_value(info_data.fax));
    BOOST_CHECK_EQUAL(*create_data.email, get_value(info_data.email));
    BOOST_CHECK_EQUAL(*create_data.notify_email, get_value(info_data.notifyemail));
    BOOST_CHECK_EQUAL(*create_data.vat, get_value(info_data.vat));
    BOOST_CHECK_EQUAL(get_ident_value(*create_data.ident), get_value(info_data.ssn));
    BOOST_CHECK_EQUAL(get_ident_type(*create_data.ident), get_value(info_data.ssntype));

    const bool to_create_mailing_address = static_cast<bool>(create_data.mailing_address);
    const auto addresses_itr = info_data.addresses.find(::LibFred::ContactAddressType::MAILING);
    const bool mailing_address_created = addresses_itr != info_data.addresses.end();
    BOOST_CHECK_EQUAL(to_create_mailing_address, mailing_address_created);
    if (to_create_mailing_address && mailing_address_created)
    {
        BOOST_CHECK(!addresses_itr->second.company_name.isset());
        BOOST_CHECK_EQUAL(create_data.mailing_address->street[0], addresses_itr->second.street1);
        BOOST_CHECK_EQUAL(create_data.mailing_address->street[1], get_value(addresses_itr->second.street2));
        BOOST_CHECK_EQUAL(create_data.mailing_address->street[2], get_value(addresses_itr->second.street3));
        BOOST_CHECK_EQUAL(create_data.mailing_address->city, addresses_itr->second.city);
        BOOST_CHECK_EQUAL(create_data.mailing_address->state_or_province, get_value(addresses_itr->second.stateorprovince));
        BOOST_CHECK_EQUAL(create_data.mailing_address->postal_code, addresses_itr->second.postalcode);
        BOOST_CHECK_EQUAL(create_data.mailing_address->country_code, addresses_itr->second.country);
    }

    BOOST_CHECK_EQUAL(create_data.authinfopw ? *create_data.authinfopw : std::string("not set"), info_data.authinfopw);
    BOOST_CHECK_EQUAL(is_public(create_data.name), info_data.disclosename);
    BOOST_CHECK_EQUAL(is_public(create_data.organization), info_data.discloseorganization);
    BOOST_CHECK_EQUAL(is_public(create_data.address), info_data.discloseaddress);
    BOOST_CHECK_EQUAL(is_public(create_data.telephone), info_data.disclosetelephone);
    BOOST_CHECK_EQUAL(is_public(create_data.fax), info_data.disclosefax);
    BOOST_CHECK_EQUAL(is_public(create_data.email), info_data.discloseemail);
    BOOST_CHECK_EQUAL(is_public(create_data.vat), info_data.disclosevat);
    BOOST_CHECK_EQUAL(is_public(create_data.ident), info_data.discloseident);
    BOOST_CHECK_EQUAL(is_public(create_data.notify_email), info_data.disclosenotifyemail);
}

bool create_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    DefaultCreateContactInputData().handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            create_invalid_registrar_id_exception);
}

namespace {

bool create_fail_handle_format_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::contact_handle);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_contact_handle);
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(create_fail_handle_format, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    InvalidHandle().handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_handle_format_exception);
}

namespace {

bool create_fail_already_existing_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_exists);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    contact.data.handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_already_existing_exception);
}

namespace {

bool create_fail_protected_handle_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::contact_handle, ::Epp::Reason::protected_period));
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(create_fail_protected_handle, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    ::LibFred::DeleteContactByHandle(contact.data.handle).exec(ctx);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    contact.data.handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_protected_handle_exception);
}

namespace {

bool create_fail_nonexistent_countrycode_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::contact_cc, ::Epp::Reason::country_notexist));
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(create_fail_nonexistent_countrycode, supply_ctx<HasRegistrarWithSession>)
{
    DefaultCreateContactInputData create_contact_input_data;
    auto address = *create_contact_input_data.address;
    address.country_code = "1Z9";
    create_contact_input_data.address = create_contact_input_data.address.make_with_the_same_privacy(address);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    create_contact_input_data.handle,
                    create_contact_input_data,
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_nonexistent_countrycode_exception);
}

BOOST_FIXTURE_TEST_CASE(create_ok_all_data, supply_ctx<HasRegistrarWithSession>)
{
    DefaultCreateContactInputData create_contact_input_data;

    const ::Epp::Contact::CreateContactResult result =
            ::Epp::Contact::create_contact(
                    ctx,
                    create_contact_input_data.handle,
                    create_contact_input_data,
                    DefaultCreateContactConfigData(),
                    session.data);

    /* check returned data and db changes */
    {
        const ::LibFred::InfoContactData check_sample = ::LibFred::InfoContactByHandle(create_contact_input_data.handle).exec(ctx).info_contact_data;
        BOOST_CHECK_EQUAL( check_sample.id, result.id );
        const Database::Result db_res = ctx.get_conn().exec("SELECT NOW() AT TIME ZONE 'utc'");
        const std::string current_utc_time = static_cast< std::string >(db_res[0][0]);
        BOOST_CHECK_EQUAL(boost::posix_time::time_from_string(current_utc_time), result.crdate);
        BOOST_CHECK_EQUAL(boost::to_upper_copy(create_contact_input_data.handle), check_sample.handle);
        check_equal(create_contact_input_data, check_sample);
    }
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact/CreateContact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

}//namespace Test
