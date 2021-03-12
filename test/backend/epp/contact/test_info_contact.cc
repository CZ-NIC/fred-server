/*
 * Copyright (C) 2016-2021  CZ.NIC, z. s. p. o.
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
#include "test/backend/epp/contact/util.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/info_contact.hh"
#include "src/backend/epp/contact/info_contact_config_data.hh"
#include "src/backend/epp/contact/util.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/contact/info_contact_config_data.hh"
#include "src/backend/epp/contact/impl/info_contact_data_filter.hh"

#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>

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

void check_equal(const ::Epp::Contact::InfoContactOutputData& epp_data, const ::LibFred::InfoContactData& fred_data)
{
    check_equal_except_authinfo(epp_data, fred_data);
    check_equal_authinfopw(epp_data.authinfopw, fred_data.authinfopw);
}

void check_equal_but_no_authinfopw(const ::Epp::Contact::InfoContactOutputData& epp_data, const ::LibFred::InfoContactData& fred_data)
{
    check_equal_except_authinfo(epp_data, fred_data);
    BOOST_CHECK_EQUAL(epp_data.authinfopw, boost::none);
}

template <typename T>
bool is_empty(const ::Epp::Contact::HideableOptional<T>& hideable)
{
    return (*hideable == boost::none) || (*hideable)->empty();
}

void check_equal_except_authinfo_respect_discloseflags(const ::Epp::Contact::InfoContactOutputData& epp_data, const ::LibFred::InfoContactData& fred_data)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(epp_data.handle), fred_data.handle);
    if (is_public(epp_data.name))
    {
        BOOST_CHECK_EQUAL(get_value(epp_data.name), fred_data.name.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(is_empty(epp_data.name));
    }
    if (is_public(epp_data.organization))
    {
        BOOST_CHECK_EQUAL(get_value(epp_data.organization), fred_data.organization.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(is_empty(epp_data.organization));
    }
    if (is_public(epp_data.address))
    {
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
    }
    else
    {
        BOOST_CHECK(!static_cast<bool>(*epp_data.address));
        BOOST_CHECK(!static_cast<bool>(epp_data.mailing_address));
    }

    if (is_public(epp_data.telephone))
    {
        BOOST_CHECK_EQUAL(get_value(epp_data.telephone), fred_data.telephone.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(is_empty(epp_data.telephone));
    }
    if (is_public(epp_data.fax))
    {
        BOOST_CHECK_EQUAL(get_value(epp_data.fax), fred_data.fax.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(is_empty(epp_data.fax));
    }
    if (is_public(epp_data.email))
    {
        BOOST_CHECK_EQUAL(get_value(epp_data.email), fred_data.email.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(is_empty(epp_data.email));
    }
    if (is_public(epp_data.notify_email))
    {
        BOOST_CHECK_EQUAL(get_value(epp_data.notify_email), fred_data.notifyemail.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(is_empty(epp_data.notify_email));
    }
    if (is_public(epp_data.VAT))
    {
        BOOST_CHECK_EQUAL(get_value(epp_data.VAT), fred_data.vat.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(is_empty(epp_data.VAT));
    }
    if (is_public(epp_data.personal_id))
    {
        BOOST_CHECK_EQUAL(get_ident_value(epp_data.personal_id), fred_data.ssn.get_value_or_default());
        BOOST_CHECK_EQUAL(get_ident_type(epp_data.personal_id), fred_data.ssntype.get_value_or_default());
    }
    else
    {
        BOOST_CHECK(*epp_data.personal_id == boost::none);
    }
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

namespace {

template <typename>
std::string relationship_name();

using ContactRegistrarRelationship = ::Epp::Contact::Impl::ContactRegistrarRelationship;

template <>
std::string relationship_name<ContactRegistrarRelationship::AuthorizedRegistrar>()
{
    return "authorized_registrar";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::SponsoringRegistrar>()
{
    return "sponsoring_registrar";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>()
{
    return "admin_contact";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>()
{
    return "domain_holder";
}

template <>
std::string relationship_name<ContactRegistrarRelationship::OtherRelationship>()
{
    return "other";
}

template <typename ...> struct AddRelationships;

template <typename First, typename ...Tail>
struct AddRelationships<First, Tail...>
{
    static void into(std::vector<std::string>& relationships)
    {
        relationships.push_back(relationship_name<First>());
        AddRelationships<Tail...>::into(relationships);
    }
};

template <>
struct AddRelationships<>
{
    static void into(const std::vector<std::string>&) { }
};

template <typename ...Relationships>
auto make_info_contact_data_filter()
{
    std::vector<std::string> relationships;
    AddRelationships<Relationships...>::into(relationships);
    boost::program_options::variables_map vm;
    vm.insert(std::make_pair("rifd::info_contact.show_private_data_to", boost::program_options::variable_value{relationships, false}));
    return ::Epp::Contact::Impl::get_info_contact_data_filter(
                ::Epp::Contact::ConfigDataFilter{}.template set_all_values<::Epp::Contact::Impl::InfoContact>(vm));
}

template <typename ...Relationships>
struct InfoContactConfigData : ::Epp::Contact::InfoContactConfigData
{
    explicit InfoContactConfigData(const char* authinfopw = "")
        : ::Epp::Contact::InfoContactConfigData{
              false,
              authinfopw,
              make_info_contact_data_filter<Relationships...>()}
    { }
};

struct Contact
{
    Contact(::LibFred::OperationContext& ctx,
            const Registrar& registrar,
            const std::string& contact_handle)
        : data{
            [&]()
            {
                ::LibFred::CreateContact{
                        contact_handle,
                        registrar.data.handle,
                        "authInfo123",
                        "Jan Novák Jr.",
                        Optional<std::string>{},
                        LibFred::Contact::PlaceAddress{
                            "ulice 1",
                            "ulice 2",
                            "ulice 3",
                            "město",
                            "hejtmanství",
                            "12345",
                            "CZ"},
                        "+420 123 456 789",
                        "+420 987 654 321",
                        "jan@novak.novak",
                        "jan.notify@novak.novak",
                        "MyVATstring",
                        "PASS",
                        "7001010005",
                        LibFred::ContactAddressList{
                            {LibFred::ContactAddressType::MAILING,
                             LibFred::ContactAddress{
                                 Optional<std::string>{},
                                 "Korešpondenčná",
                                 "ulica",
                                 "1",
                                 "Korešpondenčné Mesto",
                                 "Korešpondenčné hajtmanstvo",
                                 "54321",
                                 "SK"}}
                        },
                        true,
                        true,
                        false,
                        false,
                        false,
                        false,
                        false,
                        false,
                        false,
                        Optional<Nullable<bool>>{},
                        Optional<unsigned long long>{}
                }.exec(ctx);
                return ::LibFred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
            }()}
    { }
    ::LibFred::InfoContactData data;
};

template <typename ...> struct CollectHandles;

template <typename ...Ts>
struct CollectHandles<Contact, Ts...>
{
    static void into(std::vector<std::string>& handles, const Contact& contact, const Ts& ...tail)
    {
        handles.push_back(contact.data.handle);
        CollectHandles<Ts...>::into(handles, tail...);
    }
};

template <>
struct CollectHandles<>
{
    static void into(const std::vector<std::string>&) { }
};

template <typename ...Ts>
std::vector<std::string> collect_handles(const Ts& ...contacts)
{
    std::vector<std::string> handles;
    CollectHandles<Ts...>::into(handles, contacts...);
    return handles;
}

struct Domain
{
    template <typename ...Ts>
    Domain(::LibFred::OperationContext& ctx,
           const Registrar& registrar,
           const std::string& fqdn,
           const Contact& registrant,
           const Ts& ...admin_contacts)
        : data{
            [&]()
            {
                ::LibFred::CreateDomain update_op{fqdn, registrar.data.handle, registrant.data.handle};
                const auto handles = collect_handles(admin_contacts...);
                if (!handles.empty())
                {
                    update_op.set_admin_contacts(handles);
                }
                update_op.exec(ctx);
                return ::LibFred::InfoDomainByFqdn(fqdn).exec(ctx, "UTC").info_domain_data;
            }()}
    { }
    ::LibFred::InfoDomainData data;
};

enum class Share
{
    all,
    all_except_authinfo,
    by_discloseflags
};

template <typename ...Relationships>
void check(LibFred::OperationContext& ctx,
           const SessionData& session,
           const Contact& contact,
           Share expected_share_level)
{
    const auto cmp = [&]()
    {
        switch (expected_share_level)
        {
            case Share::all:
                return check_equal;
            case Share::all_except_authinfo:
                return check_equal_but_no_authinfopw;
            case Share::by_discloseflags:
                return check_equal_except_authinfo_respect_discloseflags;
        }
        throw std::logic_error{"unexpected Share value"};
    }();
    cmp(::Epp::Contact::info_contact(
            ctx,
            contact.data.handle,
            InfoContactConfigData<Relationships...>{},
            session),
        contact.data);
}

template <typename ...Relationships>
void check(LibFred::OperationContext& ctx,
           const SessionData& session,
           const char* authinfopw,
           const Contact& contact,
           Share expected_share_level)
{
    const auto cmp = [&]()
    {
        switch (expected_share_level)
        {
            case Share::all:
                return check_equal;
            case Share::all_except_authinfo:
                return check_equal_but_no_authinfopw;
            case Share::by_discloseflags:
                return check_equal_except_authinfo_respect_discloseflags;
        }
        throw std::logic_error{"unexpected Share value"};
    }();
    cmp(::Epp::Contact::info_contact(
            ctx,
            contact.data.handle,
            InfoContactConfigData<Relationships...>{authinfopw},
            session),
        contact.data);
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(info_contact_data_filter_test, autorollbacking_context)
{
    const Registrar registrar_a{ctx, "REG-TEST-A"};
    const Registrar registrar_b{ctx, "REG-TEST-B"};
    const SessionData session_a{registrar_a.data.id};
    const SessionData session_b{registrar_b.data.id};
    const Contact contact_a{ctx, registrar_a, "CONTACT-TEST-A"};
    const Contact contact_b{ctx, registrar_b, "CONTACT-TEST-B"};
    const Contact contact_b0{ctx, registrar_b, "CONTACT-TEST-B-0"};
    const Domain domain_a_owner_a{ctx, registrar_a, "domain-a-owner-a.cz", contact_a};
    const Domain domain_b_owner_a{ctx, registrar_b, "domain-b-owner-a.cz", contact_a};
    const Domain domain_b_owner_b{ctx, registrar_b, "domain-b-owner-b.cz", contact_b};
    const Domain domain_b_owner_b_admin_a{ctx, registrar_b, "domain-b-owner-b-admin-a.cz", contact_b, contact_a};
    check<ContactRegistrarRelationship::SponsoringRegistrar>(
                ctx,
                session_a,
                contact_a,
                Share::all);
    check<ContactRegistrarRelationship::SponsoringRegistrar>(
                ctx,
                session_b,
                contact_a,
                Share::by_discloseflags);
    check<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(
                ctx,
                session_a,
                contact_a,
                Share::all);
    check<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>(
                ctx,
                session_b,
                contact_a,
                Share::all_except_authinfo);
    check<ContactRegistrarRelationship::OtherRelationship>(
                ctx,
                session_b,
                contact_a,
                Share::by_discloseflags);
    check<ContactRegistrarRelationship::AuthorizedRegistrar,
          ContactRegistrarRelationship::SponsoringRegistrar,
          ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact,
          ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(
                ctx,
                session_a,
                contact_b0,
                Share::by_discloseflags);
    check<ContactRegistrarRelationship::OtherRelationship>(
                ctx,
                session_a,
                contact_b0,
                Share::all_except_authinfo);
    check<ContactRegistrarRelationship::AuthorizedRegistrar>(
                ctx,
                session_a,
                contact_b0.data.authinfopw.c_str(),
                contact_b0,
                Share::all_except_authinfo);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact/InfoContact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

}//namespace Test
