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
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/info_contact.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/contact/info_contact_config_data.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

namespace {

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

std::string get_ident_type(const ::Epp::Contact::HideableOptional<::Epp::Contact::ContactIdent>& ident)
{
    const boost::optional<::LibFred::PersonalIdUnion> personal_id = get_ident(*ident);
    return static_cast<bool>(personal_id) ? personal_id->get_type()
                                          : std::string();
}

std::string get_ident_value(const ::Epp::Contact::HideableOptional<::Epp::Contact::ContactIdent>& ident)
{
    const boost::optional<::LibFred::PersonalIdUnion> personal_id = get_ident(*ident);
    return static_cast<bool>(personal_id) ? personal_id->get()
                                          : std::string();
}

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

std::string get_value(const boost::optional<std::string>& opt)
{
    if (static_cast<bool>(opt))
    {
        return *opt;
    }
    return std::string();
}

std::string get_value(const ::Epp::Contact::HideableOptional<std::string>& hideable)
{
    return get_value(*hideable);
}

void check_equal_authinfopw(const boost::optional<std::string>& epp_data_authinfopw, const std::string fred_data_authinfopw)
{
    BOOST_REQUIRE(epp_data_authinfopw != boost::none);
    BOOST_CHECK_EQUAL(*epp_data_authinfopw, fred_data_authinfopw);
}

void check_equal_except_authinfo(const ::Epp::Contact::InfoContactOutputData& epp_data, const ::LibFred::InfoContactData& fred_data)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(epp_data.handle), fred_data.handle);
    BOOST_CHECK_EQUAL(get_value(epp_data.name), fred_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value(epp_data.organization), fred_data.organization.get_value_or_default());
    BOOST_REQUIRE(static_cast<bool>(*epp_data.address));
    BOOST_CHECK_EQUAL(get_value((*epp_data.address)->street[0]), fred_data.place.get_value_or_default().street1);
    BOOST_CHECK_EQUAL(get_value((*epp_data.address)->street[1]), fred_data.place.get_value_or_default().street2.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value((*epp_data.address)->street[2]), fred_data.place.get_value_or_default().street3.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value((*epp_data.address)->city), fred_data.place.get_value_or_default().city);
    BOOST_CHECK_EQUAL(get_value((*epp_data.address)->postal_code), fred_data.place.get_value_or_default().postalcode);
    BOOST_CHECK_EQUAL(get_value((*epp_data.address)->state_or_province), fred_data.place.get_value_or_default().stateorprovince.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value((*epp_data.address)->country_code), fred_data.place.get_value_or_default().country);

    const bool epp_mailing_address_presents = static_cast<bool>(epp_data.mailing_address);
    const auto addresses_itr = fred_data.addresses.find(::LibFred::ContactAddressType::MAILING);
    const bool fred_mailing_address_presents = addresses_itr != fred_data.addresses.end();
    BOOST_CHECK_EQUAL(epp_mailing_address_presents, fred_mailing_address_presents);
    if (epp_mailing_address_presents && fred_mailing_address_presents)
    {
        BOOST_CHECK(!addresses_itr->second.company_name.isset());
        BOOST_CHECK(static_cast<bool>(epp_data.mailing_address->street[0]));
        if (static_cast<bool>(epp_data.mailing_address->street[0]))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->street[0]), addresses_itr->second.street1);
        }
        BOOST_CHECK_EQUAL(static_cast<bool>(epp_data.mailing_address->street[1]), addresses_itr->second.street2.isset());
        if ((static_cast<bool>(epp_data.mailing_address->street[1])) && (addresses_itr->second.street2.isset()))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->street[1]), addresses_itr->second.street2.get_value());
        }
        BOOST_CHECK_EQUAL(static_cast<bool>(epp_data.mailing_address->street[2]), addresses_itr->second.street3.isset());
        if ((static_cast<bool>(epp_data.mailing_address->street[2])) && (addresses_itr->second.street3.isset()))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->street[2]), addresses_itr->second.street3.get_value());
        }
        BOOST_CHECK(static_cast<bool>(epp_data.mailing_address->city));
        if (static_cast<bool>(epp_data.mailing_address->city))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->city), addresses_itr->second.city);
        }
        BOOST_CHECK_EQUAL(static_cast<bool>(epp_data.mailing_address->state_or_province), addresses_itr->second.stateorprovince.isset());
        if ((static_cast<bool>(epp_data.mailing_address->state_or_province)) && (addresses_itr->second.stateorprovince.isset()))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->state_or_province), addresses_itr->second.stateorprovince.get_value());
        }
        BOOST_CHECK(static_cast<bool>(epp_data.mailing_address->postal_code));
        if (static_cast<bool>(epp_data.mailing_address->postal_code))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->postal_code), addresses_itr->second.postalcode);
        }
        BOOST_CHECK(static_cast<bool>(epp_data.mailing_address->country_code));
        if (static_cast<bool>(epp_data.mailing_address->country_code))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->country_code), addresses_itr->second.country);
        }
    }

    BOOST_CHECK_EQUAL(get_value(epp_data.telephone), fred_data.telephone.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value(epp_data.fax), fred_data.fax.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value(epp_data.email), fred_data.email.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value(epp_data.notify_email), fred_data.notifyemail.get_value_or_default());
    BOOST_CHECK_EQUAL(get_value(epp_data.VAT), fred_data.vat.get_value_or_default());
    BOOST_CHECK_EQUAL(get_ident_value(epp_data.personal_id), fred_data.ssn.get_value_or_default());
    BOOST_CHECK_EQUAL(get_ident_type(epp_data.personal_id), fred_data.ssntype.get_value_or_default());
    BOOST_CHECK_EQUAL(is_public(epp_data.name), fred_data.disclosename);
    BOOST_CHECK_EQUAL(is_public(epp_data.organization), fred_data.discloseorganization);
    BOOST_CHECK_EQUAL(is_public(epp_data.address), fred_data.discloseaddress);
    BOOST_CHECK_EQUAL(is_public(epp_data.telephone), fred_data.disclosetelephone);
    BOOST_CHECK_EQUAL(is_public(epp_data.fax), fred_data.disclosefax);
    BOOST_CHECK_EQUAL(is_public(epp_data.email), fred_data.discloseemail);
    BOOST_CHECK_EQUAL(is_public(epp_data.VAT), fred_data.disclosevat);
    BOOST_CHECK_EQUAL(is_public(epp_data.personal_id), fred_data.discloseident);
    BOOST_CHECK_EQUAL(is_public(epp_data.notify_email), fred_data.disclosenotifyemail);
}

void check_equal(::Epp::Contact::InfoContactOutputData epp_data, const ::LibFred::InfoContactData& fred_data)
{
    check_equal_except_authinfo(epp_data, fred_data);
    check_equal_authinfopw(epp_data.authinfopw, fred_data.authinfopw);
}

void check_equal_but_no_authinfopw(::Epp::Contact::InfoContactOutputData epp_data, const ::LibFred::InfoContactData& fred_data)
{
    check_equal_except_authinfo(epp_data, fred_data);
    BOOST_CHECK_EQUAL(epp_data.authinfopw, boost::none);
}

}//namespace Test::{anonymous}

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(InfoContact)

namespace {

bool info_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(info_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::info_contact(
            ctx,
            ValidHandle().handle,
            DefaultInfoContactConfigData(),
            session_with_unauthenticated_registrar.data
        ),
        ::Epp::EppResponseFailure,
        info_invalid_registrar_id_exception);
}

namespace {

bool info_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(info_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::info_contact(
            ctx,
            NonexistentHandle().handle,
            DefaultInfoContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        info_fail_nonexistent_handle_exception);
}

BOOST_FIXTURE_TEST_CASE(info_ok_full_data_for_sponsoring_registrar, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    check_equal(
            ::Epp::Contact::info_contact(
                    ctx,
                    contact.data.handle,
                    DefaultInfoContactConfigData(),
                    session.data),
            contact.data);
}

BOOST_FIXTURE_TEST_CASE(info_ok_full_data_for_different_registrar, supply_ctx<HasRegistrarWithSessionAndContactOfDifferentRegistrar>)
{
    check_equal_but_no_authinfopw(
            ::Epp::Contact::info_contact(
                    ctx,
                    contact_of_different_registrar.data.handle,
                    DefaultInfoContactConfigData(),
                    session.data),
            contact_of_different_registrar.data);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact/InfoContact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

}//namespace Test
