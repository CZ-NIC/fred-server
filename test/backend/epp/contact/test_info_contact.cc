/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/contact/impl/contact_data_share_policy_rules.hh"

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
            ::Epp::Password{},
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
            ::Epp::Password{},
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
                    ::Epp::Password{},
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
                    ::Epp::Password{},
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

template <>
std::string relationship_name<ContactRegistrarRelationship::SystemRegistrar>()
{
    return "system_registrar";
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
auto make_contact_data_share_policy_rules()
{
    std::vector<std::string> relationships;
    AddRelationships<Relationships...>::into(relationships);
    boost::program_options::variables_map vm;
    vm.insert(std::make_pair("rifd::info_contact.show_private_data_to", boost::program_options::variable_value{relationships, false}));
    return ::Epp::Contact::Impl::get_contact_data_share_policy_rules(
                ::Epp::Contact::ConfigDataFilter{}.template set_all_values<::Epp::Contact::Impl::InfoContact>(vm));
}

template <typename ...Relationships>
struct InfoContactConfigData : ::Epp::Contact::InfoContactConfigData
{
    InfoContactConfigData()
        : ::Epp::Contact::InfoContactConfigData{
              false,
              make_contact_data_share_policy_rules<Relationships...>()}
    { }
};

struct Contact
{
    Contact(::LibFred::OperationContext& ctx,
            const LibFred::InfoRegistrarData& registrar_data,
            const std::string& contact_handle)
        : data{
            [&]()
            {
                ::LibFred::CreateContact{
                        contact_handle,
                        registrar_data.handle,
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
    Contact(::LibFred::OperationContext& ctx,
            const Registrar& registrar,
            const std::string& contact_handle)
        : Contact{ctx, registrar.data, contact_handle}
    { }
    Contact(::LibFred::OperationContext& ctx,
            const SystemRegistrar& registrar,
            const std::string& contact_handle)
        : Contact{ctx, registrar.data, contact_handle}
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
    const auto authinfo_before = ::Epp::Password{LibFred::InfoContactById{contact.data.id}.exec(ctx).info_contact_data.authinfopw};
    cmp(::Epp::Contact::info_contact(
            ctx,
            contact.data.handle,
            InfoContactConfigData<Relationships...>{},
            ::Epp::Password{},
            session),
        contact.data);
    const auto authinfo_after = ::Epp::Password{LibFred::InfoContactById{contact.data.id}.exec(ctx).info_contact_data.authinfopw};
    BOOST_REQUIRE((authinfo_before.is_empty() && authinfo_after.is_empty()) ||
                  (!authinfo_before.is_empty() && !authinfo_after.is_empty()));
    BOOST_CHECK((authinfo_before.is_empty() && authinfo_after.is_empty()) ||
                (authinfo_before == authinfo_after));
}

template <typename ...Relationships>
void check(LibFred::OperationContext& ctx,
           const SessionData& session,
           const ::Epp::Password& authinfopw,
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
    const auto authinfo_before = ::Epp::Password{LibFred::InfoContactById{contact.data.id}.exec(ctx).info_contact_data.authinfopw};
    BOOST_REQUIRE(!authinfo_before.is_empty());
    const auto authorized = authinfo_before == authinfopw;
    cmp(::Epp::Contact::info_contact(
            ctx,
            contact.data.handle,
            InfoContactConfigData<Relationships...>{},
            authinfopw,
            session),
        contact.data);
    const auto authinfo_after = ::Epp::Password{LibFred::InfoContactById{contact.data.id}.exec(ctx).info_contact_data.authinfopw};
    if (authorized)
    {
        BOOST_CHECK(authinfopw != authinfo_after);
    }
    else
    {
        BOOST_CHECK(authinfo_before == authinfo_after);
    }
}

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(contact_data_share_policy_rules_test, autorollbacking_context)
{
    const Registrar registrar_a{ctx, "REG-TEST-A"};
    const Registrar registrar_b{ctx, "REG-TEST-B"};
    const SystemRegistrar sys_registrar{ctx};
    const SessionData session_a{registrar_a.data.id};
    const SessionData session_b{registrar_b.data.id};
    const SessionData sysreg_session{sys_registrar.data.id};
    const Contact contact_a{ctx, registrar_a, "CONTACT-TEST-A"};
    const Contact contact_b{ctx, registrar_b, "CONTACT-TEST-B"};
    const Contact contact_b0{ctx, registrar_b, "CONTACT-TEST-B-0"};
    const Contact contact_sys{ctx, sys_registrar, "CONTACT-TEST-SYS"};
    const Domain domain_a_owner_a{ctx, registrar_a, "domain-a-owner-a.cz", contact_a};
    const Domain domain_b_owner_a{ctx, registrar_b, "domain-b-owner-a.cz", contact_a};
    const Domain domain_b_owner_b{ctx, registrar_b, "domain-b-owner-b.cz", contact_b};
    const Domain domain_b_owner_b_admin_a{ctx, registrar_b, "domain-b-owner-b-admin-a.cz", contact_b, contact_a};
    BOOST_TEST_MESSAGE("SponsoringRegistrar a-a");
    check<ContactRegistrarRelationship::SponsoringRegistrar>(
                ctx,
                session_a,
                contact_a,
                Share::all);
    BOOST_TEST_MESSAGE("SponsoringRegistrar b-a");
    check<ContactRegistrarRelationship::SponsoringRegistrar>(
                ctx,
                session_b,
                contact_a,
                Share::by_discloseflags);
    BOOST_TEST_MESSAGE("DomainHolder a-a");
    check<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(
                ctx,
                session_a,
                contact_a,
                Share::all);
    BOOST_TEST_MESSAGE("AdminContact b-a");
    check<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>(
                ctx,
                session_b,
                contact_a,
                Share::all_except_authinfo);
    BOOST_TEST_MESSAGE("OtherRelationship b-a");
    check<ContactRegistrarRelationship::OtherRelationship>(
                ctx,
                session_b,
                contact_a,
                Share::by_discloseflags);
    BOOST_TEST_MESSAGE("AuthorizedRegistrar + SponsoringRegistrar + AdminContact + DomainHolder a-b0");
    check<ContactRegistrarRelationship::AuthorizedRegistrar,
          ContactRegistrarRelationship::SponsoringRegistrar,
          ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact,
          ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(
                ctx,
                session_a,
                contact_b0,
                Share::by_discloseflags);
    BOOST_TEST_MESSAGE("OtherRelationship a-b0");
    check<ContactRegistrarRelationship::OtherRelationship>(
                ctx,
                session_a,
                contact_b0,
                Share::all_except_authinfo);
    BOOST_TEST_MESSAGE("AuthorizedRegistrar a-b0");
    check<ContactRegistrarRelationship::AuthorizedRegistrar>(
                ctx,
                session_a,
                ::Epp::Password{contact_b0.data.authinfopw},
                contact_b0,
                Share::all_except_authinfo);
    BOOST_TEST_MESSAGE("SystemRegistrar sys-a");
    check<ContactRegistrarRelationship::SystemRegistrar>(
                ctx,
                sysreg_session,
                contact_a,
                Share::all_except_authinfo);
    BOOST_TEST_MESSAGE("SystemRegistrar sys-b");
    check<ContactRegistrarRelationship::SystemRegistrar>(
                ctx,
                sysreg_session,
                contact_b,
                Share::all_except_authinfo);
    BOOST_TEST_MESSAGE("SystemRegistrar sys-b0");
    check<ContactRegistrarRelationship::SystemRegistrar>(
                ctx,
                sysreg_session,
                contact_b0,
                Share::all_except_authinfo);
    BOOST_TEST_MESSAGE("SystemRegistrar sys-sys");
    check<ContactRegistrarRelationship::SystemRegistrar>(
                ctx,
                sysreg_session,
                contact_sys,
                Share::all);
    BOOST_TEST_MESSAGE("SponsoringRegistrar + SystemRegistrar sys-sys");
    check<ContactRegistrarRelationship::SponsoringRegistrar,
          ContactRegistrarRelationship::SystemRegistrar>(
                ctx,
                sysreg_session,
                contact_sys,
                Share::all);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact/InfoContact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Contact
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

}//namespace Test
