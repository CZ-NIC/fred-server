/*
 * Copyright (C) 2008-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/domain_browser/domain_browser.hh"

#include "libfred/object/check_authinfo.hh"
#include "libfred/object_state/object_has_state.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/contact_state.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/domain_state.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/keyset_state.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/nsset_state.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"
#include "libfred/registrar/info_registrar_impl.hh"
#include "libfred/opexception.hh"
#include "libfred/opcontext.hh"

#include "util/util.hh"
#include "util/map_at.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"

#include "src/deprecated/libfred/object_state/object_state_name.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_domainbrowser_args.hh"

#include "test/poc/parallel-tests/fixtures/contact.hh"
#include "test/poc/parallel-tests/fixtures/domain.hh"
#include "test/poc/parallel-tests/fixtures/has_fresh_database.hh"
#include "test/poc/parallel-tests/fixtures/keyset.hh"
#include "test/poc/parallel-tests/fixtures/nsset.hh"
#include "test/poc/parallel-tests/fixtures/operation_context.hh"
#include "test/poc/parallel-tests/fixtures/registrar.hh"
#include "test/poc/parallel-tests/fixtures/zone.hh"
#include "test/poc/parallel-tests/fixtures/zone_access.hh"

#include <boost/lexical_cast.hpp>

#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/**
 *  @file
 *  test domain browser
 */

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestDomainBrowser)

class MojeidRegistrar : public Test::Registrar
{
public:
    explicit MojeidRegistrar(::LibFred::OperationContext& ctx)
        : Test::Registrar{ctx, Test::Setter::registrar(LibFred::CreateRegistrar{get_handle()})}
    {
        BOOST_TEST_MESSAGE(data.handle);
    }
private:
    static std::string get_handle()
    {
        return CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->registrar_handle;
    }
};

struct DomainBrowserImplInstanceFixture : Fred::Backend::DomainBrowser::DomainBrowser
{
    explicit DomainBrowserImplInstanceFixture(const MojeidRegistrar& reg_mojeid)//MojeId registrar used for updates in domain browser
        : Fred::Backend::DomainBrowser::DomainBrowser{
                "test-domain-browser",
                reg_mojeid.data.handle,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->domain_list_limit,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->nsset_list_limit,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->keyset_list_limit,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->contact_list_limit}
    { }
    explicit DomainBrowserImplInstanceFixture(
            Test::Commit,//help to commit transaction with prepared test objects
            const MojeidRegistrar& reg_mojeid)
        : DomainBrowserImplInstanceFixture{reg_mojeid}
    { }
};

namespace {

std::string make_extension(int index, bool lower = false)
{
    auto extension = std::string{};
    if (index == 0)
    {
        return extension;
    }
    static constexpr auto number_of_letters = 'Z' - 'A' + 1;
    if (index <= number_of_letters)
    {
        extension.append(1, char((lower ? 'a' : 'A') + index - 1));
        return extension;
    }
    extension = std::to_string(index - number_of_letters - 1);
    return extension;
}

std::string make_handle(const char* prefix, const std::string& extension)
{
    auto handle = std::string{};
    if (extension.empty())
    {
        handle = prefix;
        return handle;
    }
    const auto prefix_length = std::strlen(prefix);
    handle.reserve(prefix_length + 1 + extension.length());
    handle.append(prefix, prefix_length);
    handle.append(1, '-');
    handle.append(extension);
    return handle;
}

template <typename Zone>
std::string make_fqdn(const char* prefix, int index)
{
    return Zone::fqdn(make_handle(prefix, make_extension(index, true)));
}

class UserContactProducer;

class MojeidContact : public Test::Contact
{
public:
    MojeidContact(MojeidContact&&) = default;
private:
    explicit MojeidContact(Test::Contact contact)
        : Test::Contact{std::move(contact)}
    { }
    friend class UserContactProducer;
};

class UserContactProducer
{
public:
    UserContactProducer() : index_{0} { }
    LibFred::CreateContact get_creator(const Test::Registrar& registrar)
    {
        const auto extension = make_extension(index_);
        ++index_;
        const auto handle = make_handle("USER-CONTACT-HANDLE", extension);
        return LibFred::CreateContact{handle, registrar.data.handle}
                    .set_name(make_handle("USER-CONTACT-HANDLE NAME", extension))
                    .set_place(
                            [&extension]()
                            {
                                ::LibFred::Contact::PlaceAddress place;
                                place.street1 = make_handle("STR1", extension);
                                place.city = "Praha";
                                place.postalcode = "11150";
                                place.country = "CZ";
                                return place;
                            }());
    }
    template <typename ...State>
    MojeidContact operator()(::LibFred::OperationContext& ctx, const MojeidRegistrar& registrar, State ...state)
    {
        return MojeidContact{Test::Contact{ctx, get_creator(registrar), state...}};
    }
    template <typename ...State>
    MojeidContact operator()(::LibFred::OperationContext& ctx, LibFred::CreateContact creator, State ...state)
    {
        return MojeidContact{Test::Contact{ctx, std::move(creator), state...}};
    }
private:
    int index_;
};

constexpr auto flag_mojeid_contact = LibFred::RegistrableObject::Contact::MojeidContact{};
constexpr auto flag_identified_contact = LibFred::RegistrableObject::Contact::IdentifiedContact{};
constexpr auto flag_server_blocked_contact = LibFred::RegistrableObject::Contact::ServerBlocked{};
constexpr auto flag_validated_contact = LibFred::RegistrableObject::Contact::ValidatedContact{};

class TestRegistrarProducer;

class TestRegistrar : public Test::Registrar
{
public:
    TestRegistrar(TestRegistrar&&) = default;
private:
    explicit TestRegistrar(Test::Registrar registrar)
        : Test::Registrar{std::move(registrar)}
    {
        BOOST_TEST_MESSAGE(data.handle);
    }
    friend class TestRegistrarProducer;
};

class TestRegistrarProducer
{
public:
    TestRegistrarProducer() : index_{0} { }
    TestRegistrar operator()(::LibFred::OperationContext& ctx)
    {
        const auto extension = make_extension(index_);
        ++index_;
        const auto handle = make_handle("TEST-REGISTRAR-HANDLE", extension);
        return TestRegistrar{Test::Registrar{
                ctx,
                LibFred::CreateRegistrar{handle}
                    .set_name(make_handle("TEST-REGISTRAR NAME", extension))
                    .set_street1(make_handle("STR1", extension))
                    .set_city("Praha")
                    .set_postalcode("11150")
                    .set_country("CZ")}};
    }
private:
    int index_;
};

class TestContactProducer
{
public:
    TestContactProducer() : index_{0} { }
    template <typename ...State>
    Test::Contact operator()(::LibFred::OperationContext& ctx, const TestRegistrar& registrar, State ...state)
    {
        const auto extension = make_extension(index_);
        ++index_;
        const auto handle = make_handle("TEST-CONTACT-HANDLE", extension);
        return Test::Contact{
                ctx,
                LibFred::CreateContact{handle, registrar.data.handle}
                    .set_name(make_handle("TEST-CONTACT NAME", extension))
                    .set_disclosename(true)
                    .set_place(
                            [&extension]()
                            {
                                ::LibFred::Contact::PlaceAddress place;
                                place.street1 = make_handle("STR1", extension);
                                place.city = "Praha";
                                place.postalcode = "11150";
                                place.country = "CZ";
                                return place;
                            }())
                    .set_discloseaddress(true),
                state...};
    }
private:
    int index_;
};

class AdminContactProducer;

class AdminContact : public Test::Contact
{
public:
    AdminContact(AdminContact&&) = default;
private:
    explicit AdminContact(Test::Contact contact)
        : Test::Contact{std::move(contact)}
    { }
    friend class AdminContactProducer;
};

class AdminContactProducer
{
public:
    AdminContactProducer() : index_{0} { }
    template <typename ...State>
    AdminContact operator()(::LibFred::OperationContext& ctx, const TestRegistrar& registrar, State ...state)
    {
        const auto extension = make_extension(index_);
        ++index_;
        const auto handle = make_handle("TEST-ADMIN-HANDLE", extension);
        return AdminContact{Test::Contact{
                ctx,
                LibFred::CreateContact{handle, registrar.data.handle}
                    .set_organization(make_handle("TEST-ORGANIZATION", extension))
                    .set_name(make_handle("TEST-CONTACT NAME", extension))
                    .set_disclosename(true)
                    .set_place(
                            [&extension]()
                            {
                                ::LibFred::Contact::PlaceAddress place;
                                place.street1 = make_handle("STR1", extension);
                                place.city = "Praha";
                                place.postalcode = "11150";
                                place.country = "CZ";
                                return place;
                            }())
                    .set_discloseaddress(true),
                state...}};
    }
private:
    int index_;
};

class NssetProducer
{
public:
    NssetProducer() : index_{0} { }
    template <typename ...State>
    Test::Nsset operator()(
            ::LibFred::OperationContext& ctx,
            const TestRegistrar& registrar,
            const std::vector<std::string>& admin_contacts,
            State ...state)
    {
        const auto extension = make_extension(index_);
        ++index_;
        const auto handle = make_handle("TEST-NSSET-HANDLE", extension);
        return Test::Nsset{
                ctx,
                LibFred::CreateNsset{handle, registrar.data.handle}
                    .set_tech_contacts(admin_contacts)
                    .set_dns_hosts({
                            ::LibFred::DnsHost{
                                    "a.ns.nic.cz",
                                    { boost::asio::ip::address::from_string("127.0.0.3"), boost::asio::ip::address::from_string("127.1.1.3") }}, //add_dns
                            ::LibFred::DnsHost{
                                    "b.ns.nic.cz",
                                    { boost::asio::ip::address::from_string("127.0.0.4"), boost::asio::ip::address::from_string("127.1.1.4") }}} //add_dns
                    ),
                LibFred::RegistrableObject::Nsset::ServerDeleteProhibited{},
                state...};
    }
private:
    int index_;
};

class KeysetProducer
{
public:
    KeysetProducer() : index_{0} { }
    template <typename ...State>
    Test::Keyset operator()(
            ::LibFred::OperationContext& ctx,
            const TestRegistrar& registrar,
            const std::vector<std::string>& admin_contacts,
            State ...state)
    {
        const auto extension = make_extension(index_);
        ++index_;
        const auto handle = make_handle("TEST-KEYSET-HANDLE", extension);
        return Test::Keyset{
                ctx,
                LibFred::CreateKeyset{handle, registrar.data.handle}
                    .set_tech_contacts(admin_contacts)
                    .set_dns_keys({ ::LibFred::DnsKey{257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"} }),
                LibFred::RegistrableObject::Keyset::ServerDeleteProhibited{},
                state...};
    }
private:
    int index_;
};

class MyDomainProducer
{
public:
    MyDomainProducer() : index_{0} { }
    ::LibFred::CreateDomain get_creator(
            const TestRegistrar& sponsoring_registrar,
            const MojeidContact& registrant,
            const AdminContact& admin_contact,
            const Test::Nsset& nsset,
            const Test::Keyset& keyset)
    {
        return ::LibFred::CreateDomain{
                make_fqdn<Test::CzZone>("test", index_++),
                sponsoring_registrar.data.handle,
                registrant.data.handle,
                std::string{},//const Optional<std::string>& authinfo
                nsset.data.handle,
                keyset.data.handle,
                { admin_contact.data.handle },
                boost::gregorian::day_clock::local_day() + boost::gregorian::months(12),//expiration_date
                Optional<boost::gregorian::date>{},
                Optional<bool>{},
                Optional<unsigned long long>{}};//logd_request_id
    }
    template <typename ...State>
    Test::Domain operator()(
            LibFred::OperationContext& ctx,
            const TestRegistrar& sponsoring_registrar,
            const MojeidContact& registrant,
            const AdminContact& admin_contact,
            const Test::Nsset& nsset,
            const Test::Keyset& keyset,
            State ...state)
    {
        return Test::Domain{
                ctx,
                get_creator(sponsoring_registrar, registrant, admin_contact, nsset, keyset),
                state...};
    }
private:
    int index_;
};

class AdminDomainProducer
{
public:
    AdminDomainProducer() : index_{0} { }
    ::LibFred::CreateDomain get_creator(
            const TestRegistrar& sponsoring_registrar,
            const AdminContact& registrant,
            const Test::Contact& admin_contact,
            const Test::Nsset& nsset,
            const Test::Keyset& keyset)
    {
        return ::LibFred::CreateDomain{
                make_fqdn<Test::CzZone>("test", index_++),
                sponsoring_registrar.data.handle,
                registrant.data.handle,
                {},//const Optional<std::string>& authinfo
                nsset.data.handle,
                keyset.data.handle,
                { admin_contact.data.handle },
                boost::gregorian::day_clock::local_day() + boost::gregorian::months(12),//expiration_date
                Optional<boost::gregorian::date>{},
                Optional<bool>{},
                Optional<unsigned long long>{}};//logd_request_id
    }
    template <typename ...State>
    Test::Domain operator()(
            LibFred::OperationContext& ctx,
            const TestRegistrar& sponsoring_registrar,
            const AdminContact& registrant,
            const Test::Contact& admin_contact,
            const Test::Nsset& nsset,
            const Test::Keyset& keyset,
            State ...state)
    {
        return Test::Domain{
                ctx,
                get_creator(sponsoring_registrar, registrant, admin_contact, nsset, keyset),
                state...};
    }
private:
    int index_;
};

struct Factory
    : UserContactProducer,
      TestRegistrarProducer,
      TestContactProducer,
      AdminContactProducer,
      NssetProducer,
      KeysetProducer,
      MyDomainProducer,
      AdminDomainProducer
{};

struct MainFixture : Test::HasFreshDatabase, Factory
{
    MainFixture()
        : Test::HasFreshDatabase{},
          Factory{},
          ctx{},
          reg_mojeid{ctx}
    { }
    Test::OperationContext ctx;
    MojeidRegistrar reg_mojeid;
};

}//namespace {anonymous}


BOOST_AUTO_TEST_SUITE(getRegistrarDetail)

struct GetRegistrarFixture : MainFixture
{
    GetRegistrarFixture()
        : MainFixture{},
          contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit(ctx), reg_mojeid}
    { }
    MojeidContact contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getRegistrarDetail
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail, GetRegistrarFixture)
{
    const auto registrar_info = ::LibFred::InfoRegistrarByHandle(reg_mojeid.data.handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto rd = dombr.getRegistrarDetail(contact.data.id, reg_mojeid.data.handle);

    BOOST_CHECK_EQUAL(rd.id, registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(rd.handle, registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(rd.name, registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(rd.phone, registrar_info.info_registrar_data.telephone.get_value_or_default());
    BOOST_CHECK_EQUAL(rd.fax, registrar_info.info_registrar_data.fax.get_value_or_default());
    BOOST_CHECK_EQUAL(rd.url, registrar_info.info_registrar_data.url.get_value_or_default());
    BOOST_CHECK_EQUAL(rd.address, (registrar_info.info_registrar_data.street1.get_value_or_default() + ", " +
                                   registrar_info.info_registrar_data.street2.get_value_or_default() + ", " +
                                   registrar_info.info_registrar_data.street3.get_value_or_default() + ", " +
                                   registrar_info.info_registrar_data.postalcode.get_value_or_default() + " " +
                                   registrar_info.info_registrar_data.city.get_value_or_default() + ", " +
                                   registrar_info.info_registrar_data.stateorprovince.get_value_or_default()));
    BOOST_TEST_MESSAGE(rd.address);
}

struct GetRegistrarDetailNoUserFixture : MainFixture
{
    GetRegistrarDetailNoUserFixture()
        : MainFixture{},
          registrar{TestRegistrarProducer::operator()(ctx)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    TestRegistrar registrar;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getRegistrarDetail no contact
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_no_user, GetRegistrarDetailNoUserFixture)
{
    try
    {
        const auto registrar_info = ::LibFred::InfoRegistrarByHandle(registrar.data.handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        const auto rd = dombr.getRegistrarDetail(0, registrar.data.handle);
        BOOST_ERROR("unreported missing user contact");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct GetRegistrarDetailNotMojeidUserFixture : MainFixture
{
    GetRegistrarDetailNotMojeidUserFixture()
        : MainFixture{},
          registrar{TestRegistrarProducer::operator()(ctx)},
          contact{UserContactProducer::operator()(ctx, reg_mojeid)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    TestRegistrar registrar;
    Test::Contact contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getRegistrarDetail not mojeid contact
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_not_mojeid_user_fixture, GetRegistrarDetailNotMojeidUserFixture)
{
    try
    {
        const auto registrar_info = ::LibFred::InfoRegistrarByHandle(registrar.data.handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        const auto rd = dombr.getRegistrarDetail(contact.data.id, registrar.data.handle);
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct GetRegistrarDetailNoRegistrarFixture : MainFixture
{
    GetRegistrarDetailNoRegistrarFixture()
        : MainFixture{},
          contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    MojeidContact contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getRegistrarDetail no registrar
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_no_registrar, GetRegistrarDetailNoRegistrarFixture)
{
    try
    {
        const auto rd = dombr.getRegistrarDetail(contact.data.id, "NO-NO-REGISTRAR-HANDLE");
        BOOST_ERROR("unreported missing registrar");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getRegistrarDetail


BOOST_AUTO_TEST_SUITE(getContactDetail)

struct GetMyContactDetailFixture : MainFixture
{
    GetMyContactDetailFixture()
        : MainFixture{},
          contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getContactDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_my_contact_detail, GetMyContactDetailFixture)
{
    const auto my_contact_info = ::LibFred::InfoContactByHandle(contact.data.handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle(my_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    const auto cd = dombr.getContactDetail(contact.data.id, my_contact_info.info_contact_data.id);
    const auto mci_place = my_contact_info.info_contact_data.place.get_value_or_default();
    const Nullable<::LibFred::Contact::PlaceAddress> mci_mailing = optional_map_at<Nullable>(my_contact_info.info_contact_data.addresses,::LibFred::ContactAddressType::MAILING);

    BOOST_CHECK_EQUAL(cd.id, my_contact_info.info_contact_data.id);
    BOOST_CHECK_EQUAL(cd.handle, my_contact_info.info_contact_data.handle);
    BOOST_CHECK_EQUAL(cd.roid, my_contact_info.info_contact_data.roid);
    BOOST_CHECK_EQUAL(cd.sponsoring_registrar.id, sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(cd.sponsoring_registrar.handle, sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(cd.sponsoring_registrar.name, sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.creation_time, my_contact_info.info_contact_data.creation_time);
    BOOST_CHECK_EQUAL(cd.update_time.get_value_or_default(), my_contact_info.info_contact_data.update_time.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.transfer_time.get_value_or_default(), my_contact_info.info_contact_data.transfer_time.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.name.get_value_or_default(), my_contact_info.info_contact_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.organization.get_value_or_default(), my_contact_info.info_contact_data.organization.get_value_or_default());

    BOOST_CHECK(cd.permanent_address == mci_place);
    BOOST_CHECK(cd.mailing_address == mci_mailing);

    BOOST_CHECK_EQUAL(cd.telephone.get_value_or_default(), my_contact_info.info_contact_data.telephone.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.fax.get_value_or_default(), my_contact_info.info_contact_data.fax.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.email.get_value_or_default(), my_contact_info.info_contact_data.email.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.notifyemail.get_value_or_default(), my_contact_info.info_contact_data.notifyemail.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.vat.get_value_or_default(), my_contact_info.info_contact_data.vat.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.ssntype.get_value_or_default(), my_contact_info.info_contact_data.ssntype.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.ssn.get_value_or_default(), my_contact_info.info_contact_data.ssn.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.disclose_flags.name, my_contact_info.info_contact_data.disclosename);
    BOOST_CHECK_EQUAL(cd.disclose_flags.organization, my_contact_info.info_contact_data.discloseorganization);
    BOOST_CHECK_EQUAL(cd.disclose_flags.email, my_contact_info.info_contact_data.discloseemail);
    BOOST_CHECK_EQUAL(cd.disclose_flags.address, my_contact_info.info_contact_data.discloseaddress);
    BOOST_CHECK_EQUAL(cd.disclose_flags.telephone, my_contact_info.info_contact_data.disclosetelephone);
    BOOST_CHECK_EQUAL(cd.disclose_flags.fax, my_contact_info.info_contact_data.disclosefax);
    BOOST_CHECK_EQUAL(cd.disclose_flags.ident, my_contact_info.info_contact_data.discloseident);
    BOOST_CHECK_EQUAL(cd.disclose_flags.vat, my_contact_info.info_contact_data.disclosevat);
    BOOST_CHECK_EQUAL(cd.disclose_flags.notify_email, my_contact_info.info_contact_data.disclosenotifyemail);
    BOOST_CHECK(std::any_of(cd.state_codes.begin(), cd.state_codes.end(), [](auto&& state) { return state == "mojeidContact"; }));
    BOOST_CHECK(cd.is_owner);
}

struct GetContactFixture : MainFixture
{
    GetContactFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          test_contact{TestContactProducer::operator()(ctx, registrar)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    TestRegistrar registrar;
    Test::Contact test_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getContactDetail with public data
*/
BOOST_FIXTURE_TEST_CASE(get_contact_detail, GetContactFixture)
{
    const auto test_contact_info = ::LibFred::InfoContactByHandle(test_contact.data.handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle(test_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    const auto cd = dombr.getContactDetail(mojeid_contact.data.id, test_contact.data.id);
    const auto tci_place = test_contact_info.info_contact_data.place.get_value_or_default();
    const Nullable<::LibFred::Contact::PlaceAddress> tci_mailing = optional_map_at<Nullable>(test_contact_info.info_contact_data.addresses,::LibFred::ContactAddressType::MAILING);

    BOOST_CHECK_EQUAL(cd.id, test_contact_info.info_contact_data.id);
    BOOST_CHECK_EQUAL(cd.handle, test_contact_info.info_contact_data.handle);
    BOOST_CHECK_EQUAL(cd.roid, test_contact_info.info_contact_data.roid);
    BOOST_CHECK_EQUAL(cd.sponsoring_registrar.id, sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(cd.sponsoring_registrar.handle, sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(cd.sponsoring_registrar.name, sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.creation_time, test_contact_info.info_contact_data.creation_time);
    BOOST_CHECK_EQUAL(cd.update_time.get_value_or_default(), test_contact_info.info_contact_data.update_time.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.transfer_time.get_value_or_default(), test_contact_info.info_contact_data.transfer_time.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.name.get_value_or_default(), test_contact_info.info_contact_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.organization.get_value_or_default(), test_contact_info.info_contact_data.organization.get_value_or_default());

    BOOST_CHECK(cd.permanent_address == tci_place);
    BOOST_CHECK(cd.mailing_address == tci_mailing);

    BOOST_CHECK_EQUAL(cd.telephone.get_value_or_default(), test_contact_info.info_contact_data.telephone.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.fax.get_value_or_default(), test_contact_info.info_contact_data.fax.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.email.get_value_or_default(), test_contact_info.info_contact_data.email.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.notifyemail.get_value_or_default(), test_contact_info.info_contact_data.notifyemail.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.vat.get_value_or_default(), test_contact_info.info_contact_data.vat.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.ssntype.get_value_or_default(), test_contact_info.info_contact_data.ssntype.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.ssn.get_value_or_default(), test_contact_info.info_contact_data.ssn.get_value_or_default());
    BOOST_CHECK_EQUAL(cd.disclose_flags.name, test_contact_info.info_contact_data.disclosename);
    BOOST_CHECK_EQUAL(cd.disclose_flags.organization, test_contact_info.info_contact_data.discloseorganization);
    BOOST_CHECK_EQUAL(cd.disclose_flags.email, test_contact_info.info_contact_data.discloseemail);
    BOOST_CHECK_EQUAL(cd.disclose_flags.address, test_contact_info.info_contact_data.discloseaddress);
    BOOST_CHECK_EQUAL(cd.disclose_flags.telephone, test_contact_info.info_contact_data.disclosetelephone);
    BOOST_CHECK_EQUAL(cd.disclose_flags.fax, test_contact_info.info_contact_data.disclosefax);
    BOOST_CHECK_EQUAL(cd.disclose_flags.ident, test_contact_info.info_contact_data.discloseident);
    BOOST_CHECK_EQUAL(cd.disclose_flags.vat, test_contact_info.info_contact_data.disclosevat);
    BOOST_CHECK_EQUAL(cd.disclose_flags.notify_email, test_contact_info.info_contact_data.disclosenotifyemail);
    BOOST_CHECK(std::all_of(cd.state_codes.begin(), cd.state_codes.end(), [](auto&& state) { return state != "mojeidContact"; }));
    BOOST_CHECK(!cd.is_owner);
}

struct GetContactDetailNoUserFixture : MainFixture
{
    GetContactDetailNoUserFixture()
        : MainFixture{},
          registrar{TestRegistrarProducer::operator()(ctx)},
          test_contact{TestContactProducer::operator()(ctx, registrar)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    TestRegistrar registrar;
    Test::Contact test_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getContactDetail no user contact
*/
BOOST_FIXTURE_TEST_CASE(get_contact_detail_no_user, GetContactDetailNoUserFixture)
{
    try
    {
        const auto test_contact_info = ::LibFred::InfoContactByHandle(test_contact.data.handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        const auto cd = dombr.getContactDetail(0, test_contact_info.info_contact_data.id);

        BOOST_ERROR("unreported missing user contact");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct GetContactDetailNotMojeidUserFixture : MainFixture
{
    GetContactDetailNotMojeidUserFixture()
        : MainFixture{},
          user_contact{UserContactProducer::operator()(ctx, reg_mojeid)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          test_contact{TestContactProducer::operator()(ctx, registrar)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    Test::Contact user_contact;
    TestRegistrar registrar;
    Test::Contact test_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getContactDetail not mojeid user contact
 */
BOOST_FIXTURE_TEST_CASE(get_contact_detail_not_mojeid_user, GetContactDetailNotMojeidUserFixture)
{
    try
    {
        const auto test_contact_info = ::LibFred::InfoContactByHandle(test_contact.data.handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        const auto cd = dombr.getContactDetail(user_contact.data.id, test_contact.data.id);
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct GetContactDetailNoTestContactFixture : MainFixture
{
    GetContactDetailNoTestContactFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getContactDetail no contact
 */
BOOST_FIXTURE_TEST_CASE(get_contact_detail_no_test_contact, GetContactDetailNoTestContactFixture)
{
    try
    {
        const auto d = dombr.getContactDetail(mojeid_contact.data.id, 0);
        BOOST_ERROR("unreported missing test contact");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getContactDetail


BOOST_AUTO_TEST_SUITE(getDomainDetail)

struct GetMyDomainFixture : MainFixture
{
    GetMyDomainFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nsset{NssetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          keyset{KeysetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          domain{MyDomainProducer::operator()(
                ctx, registrar, mojeid_contact, admin_contact, nsset, keyset, LibFred::RegistrableObject::Domain::ServerBlocked{})},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Nsset nsset;
    Test::Keyset keyset;
    Test::Domain domain;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getDomainDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_my_domain_detail, GetMyDomainFixture)
{
    const auto my_domain_info = ::LibFred::InfoDomainByFqdn{domain.data.fqdn}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle{my_domain_info.info_domain_data.sponsoring_registrar_handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    const auto nsset_info = ::LibFred::InfoNssetByHandle{nsset.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto keyset_info = ::LibFred::InfoKeysetByHandle{keyset.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    const auto admin_contact_info = ::LibFred::InfoContactByHandle{admin_contact.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    const auto d = dombr.getDomainDetail(mojeid_contact.data.id, my_domain_info.info_domain_data.id);

    BOOST_CHECK_EQUAL(d.id, my_domain_info.info_domain_data.id);
    BOOST_CHECK_EQUAL(d.fqdn, my_domain_info.info_domain_data.fqdn);
    BOOST_CHECK_EQUAL(d.roid, my_domain_info.info_domain_data.roid);
    BOOST_CHECK_EQUAL(d.sponsoring_registrar.id, sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(d.sponsoring_registrar.handle, sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(d.sponsoring_registrar.name, sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(d.creation_time, my_domain_info.info_domain_data.creation_time);
    BOOST_CHECK_EQUAL(d.update_time.get_value_or_default(), my_domain_info.info_domain_data.update_time.get_value_or_default());
    BOOST_CHECK_EQUAL(d.registrant.id, mojeid_contact.data.id);
    BOOST_CHECK_EQUAL(d.registrant.handle, mojeid_contact.data.handle);
    BOOST_CHECK_EQUAL(d.registrant.name, (mojeid_contact.data.organization.get_value_or_default().empty()
            ? mojeid_contact.data.name.get_value_or_default()
            : mojeid_contact.data.organization.get_value_or_default()));
    BOOST_CHECK_EQUAL(d.expiration_date, my_domain_info.info_domain_data.expiration_date);
    BOOST_CHECK_EQUAL(d.enum_domain_validation.get_value_or_default(), my_domain_info.info_domain_data.enum_domain_validation.get_value_or_default());
    BOOST_CHECK_EQUAL(d.nsset.id, nsset_info.info_nsset_data.id);
    BOOST_CHECK_EQUAL(d.nsset.handle, nsset_info.info_nsset_data.handle);
    BOOST_CHECK_EQUAL(d.keyset.id, keyset_info.info_keyset_data.id);
    BOOST_CHECK_EQUAL(d.keyset.handle, keyset_info.info_keyset_data.handle);
    BOOST_CHECK_EQUAL(d.admins.at(0).id, admin_contact_info.info_contact_data.id);
    BOOST_CHECK_EQUAL(d.admins.at(0).handle, admin_contact_info.info_contact_data.handle);
    BOOST_CHECK_EQUAL(d.admins.at(0).name, (admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
        ? admin_contact_info.info_contact_data.name.get_value_or_default()
        : admin_contact_info.info_contact_data.organization.get_value_or_default()));
    BOOST_CHECK_EQUAL(d.admins.size(), 1);
    BOOST_CHECK(std::any_of(d.state_codes.begin(), d.state_codes.end(), [](auto&& state) { return state == ::LibFred::RegistrableObject::Domain::ServerBlocked::name; }));
    BOOST_CHECK(d.is_owner);
}

struct GetDomainDetailNoDomainFixture : MainFixture
{
    GetDomainDetailNoDomainFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getDomainDetail no domain
 */
BOOST_FIXTURE_TEST_CASE(get_domain_detail_no_domain, GetDomainDetailNoDomainFixture)
{
    try
    {
        const auto d = dombr.getDomainDetail(mojeid_contact.data.id, 0);
        BOOST_ERROR("unreported missing test domain");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getDomainDetail


BOOST_AUTO_TEST_SUITE(getNssetDetail)

struct GetNssetFixture : MainFixture
{
    GetNssetFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nsset{NssetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Nsset nsset;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getNssetDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_nsset_detail, GetNssetFixture)
{
    const auto nsset_info = ::LibFred::InfoNssetByHandle{nsset.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle{nsset_info.info_nsset_data.sponsoring_registrar_handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto admin_contact_info = ::LibFred::InfoContactByHandle{admin_contact.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    const auto n = dombr.getNssetDetail(mojeid_contact.data.id, nsset_info.info_nsset_data.id);

    BOOST_CHECK_EQUAL(n.id, nsset_info.info_nsset_data.id);
    BOOST_CHECK_EQUAL(n.handle, nsset_info.info_nsset_data.handle);
    BOOST_CHECK_EQUAL(n.roid, nsset_info.info_nsset_data.roid);
    BOOST_CHECK_EQUAL(n.sponsoring_registrar.id, sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(n.sponsoring_registrar.handle, sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(n.sponsoring_registrar.name, sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(n.creation_time, nsset_info.info_nsset_data.creation_time);
    BOOST_CHECK_EQUAL(n.transfer_time.get_value_or_default(), nsset_info.info_nsset_data.transfer_time.get_value_or_default());
    BOOST_CHECK_EQUAL(n.update_time.get_value_or_default(), nsset_info.info_nsset_data.update_time.get_value_or_default());

    BOOST_CHECK_EQUAL(n.create_registrar.id, sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(n.create_registrar.handle, sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(n.create_registrar.name, sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());

    BOOST_CHECK_EQUAL(n.update_registrar.id, 0);
    BOOST_CHECK_EQUAL(n.update_registrar.handle, "");
    BOOST_CHECK_EQUAL(n.update_registrar.name, "");

    BOOST_CHECK_EQUAL(n.admins.at(0).id, admin_contact_info.info_contact_data.id);
    BOOST_CHECK_EQUAL(n.admins.at(0).handle, admin_contact_info.info_contact_data.handle);
    BOOST_CHECK_EQUAL(n.admins.at(0).name, (admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
        ? admin_contact_info.info_contact_data.name.get_value_or_default()
        : admin_contact_info.info_contact_data.organization.get_value_or_default()));
    BOOST_CHECK_EQUAL(n.admins.size(), 1);

    BOOST_CHECK_EQUAL(n.hosts.at(0).get_fqdn(), "a.ns.nic.cz");
    BOOST_CHECK_EQUAL(n.hosts.at(0).get_inet_addr().at(0).to_string(), "127.0.0.3");
    BOOST_CHECK_EQUAL(n.hosts.at(0).get_inet_addr().at(1).to_string(), "127.1.1.3");
    BOOST_CHECK_EQUAL(n.hosts.at(1).get_fqdn(), "b.ns.nic.cz");
    BOOST_CHECK_EQUAL(n.hosts.at(1).get_inet_addr().at(0).to_string(), "127.0.0.4");
    BOOST_CHECK_EQUAL(n.hosts.at(1).get_inet_addr().at(1).to_string(), "127.1.1.4");

    BOOST_CHECK(std::any_of(n.state_codes.begin(), n.state_codes.end(), [](auto&& state) { return state == ::LibFred::RegistrableObject::Nsset::ServerDeleteProhibited::name; }));
    BOOST_CHECK_EQUAL(n.report_level, 0);

    BOOST_CHECK(!n.is_owner);
}

struct GetNssetDetailNoNssetFixture : MainFixture
{
    GetNssetDetailNoNssetFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getNssetDetail no nsset
 */
BOOST_FIXTURE_TEST_CASE(get_nsset_detail_no_nsset, GetNssetDetailNoNssetFixture)
{
    try
    {
        const auto d = dombr.getNssetDetail(mojeid_contact.data.id,0);
        BOOST_ERROR("unreported missing test nsset");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getNssetDetail


BOOST_AUTO_TEST_SUITE(getKeysetDetail)

struct GetKeysetFixture : MainFixture
{
    GetKeysetFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          keyset{KeysetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Keyset keyset;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getKeysetDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_keyset_detail, GetKeysetFixture)
{
    const auto keyset_info = ::LibFred::InfoKeysetByHandle{keyset.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle{keyset_info.info_keyset_data.sponsoring_registrar_handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    const auto admin_contact_info = ::LibFred::InfoContactByHandle{admin_contact.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    const auto k = dombr.getKeysetDetail(mojeid_contact.data.id, keyset_info.info_keyset_data.id);

    BOOST_CHECK_EQUAL(k.id, keyset_info.info_keyset_data.id);
    BOOST_CHECK_EQUAL(k.handle, keyset_info.info_keyset_data.handle);
    BOOST_CHECK_EQUAL(k.roid, keyset_info.info_keyset_data.roid);
    BOOST_CHECK_EQUAL(k.sponsoring_registrar.id, sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(k.sponsoring_registrar.handle, sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(k.sponsoring_registrar.name, sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(k.creation_time, keyset_info.info_keyset_data.creation_time);
    BOOST_CHECK_EQUAL(k.transfer_time.get_value_or_default(), keyset_info.info_keyset_data.transfer_time.get_value_or_default());
    BOOST_CHECK_EQUAL(k.update_time.get_value_or_default(), keyset_info.info_keyset_data.update_time.get_value_or_default());

    BOOST_CHECK_EQUAL(k.create_registrar.id, sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK_EQUAL(k.create_registrar.handle, sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK_EQUAL(k.create_registrar.name, sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());

    BOOST_CHECK_EQUAL(k.update_registrar.id, 0);
    BOOST_CHECK_EQUAL(k.update_registrar.handle, "");
    BOOST_CHECK_EQUAL(k.update_registrar.name, "");

    BOOST_CHECK_EQUAL(k.admins.at(0).id, admin_contact_info.info_contact_data.id);
    BOOST_CHECK_EQUAL(k.admins.at(0).handle, admin_contact_info.info_contact_data.handle);
    BOOST_CHECK_EQUAL(k.admins.at(0).name, (admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
        ? admin_contact_info.info_contact_data.name.get_value_or_default()
        : admin_contact_info.info_contact_data.organization.get_value_or_default()));
    BOOST_CHECK_EQUAL(k.admins.size(), 1);

    BOOST_CHECK_EQUAL(k.dnskeys.size(), 1);
    BOOST_CHECK_EQUAL(k.dnskeys.at(0).flags, 257);
    BOOST_CHECK_EQUAL(k.dnskeys.at(0).protocol, 3);
    BOOST_CHECK_EQUAL(k.dnskeys.at(0).alg, 5);
    BOOST_CHECK_EQUAL(k.dnskeys.at(0).key, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8");

    BOOST_CHECK(std::any_of(k.state_codes.begin(), k.state_codes.end(), [](auto&& state) { return state == ::LibFred::RegistrableObject::Keyset::ServerDeleteProhibited::name; }));

    BOOST_CHECK(!k.is_owner);
}

struct GetKeysetDetailNoNssetFixture : MainFixture
{
    GetKeysetDetailNoNssetFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getKeysetDetail no keyset
 */
BOOST_FIXTURE_TEST_CASE(get_keyset_detail_no_keyset, GetKeysetDetailNoNssetFixture)
{
    try
    {
        const auto d = dombr.getKeysetDetail(mojeid_contact.data.id, 0);
        BOOST_ERROR("unreported missing test keyset");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getKeysetDetail


BOOST_AUTO_TEST_SUITE(setContactDiscloseFlags)

struct SetContactDiscloseFlagsFixture : MainFixture
{
    SetContactDiscloseFlagsFixture()
        : MainFixture{},
          identified_mojeid_contact{UserContactProducer::operator()(
                ctx, reg_mojeid, flag_mojeid_contact, flag_identified_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact identified_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call setContactDiscloseFlags with private data
*/
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags, SetContactDiscloseFlagsFixture)
{
    Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    dombr.setContactDiscloseFlags(identified_mojeid_contact.data.id, set_flags, 42);

    const auto my_contact_info = ::LibFred::InfoContactByHandle{identified_mojeid_contact.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosename);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseorganization);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseemail);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseaddress);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosetelephone);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosefax);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseident);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosevat);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosenotifyemail);
    BOOST_REQUIRE(!my_contact_info.logd_request_id.isnull());
    BOOST_CHECK_EQUAL(my_contact_info.logd_request_id.get_value(), 42);
}

struct SetValidatedContactDiscloseFlagsFixture : MainFixture
{
    SetValidatedContactDiscloseFlagsFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(
                        ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact validated_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call setContactDiscloseFlags with private data and validated contact
*/
BOOST_FIXTURE_TEST_CASE(set_validated_contact_disclose_flags, SetValidatedContactDiscloseFlagsFixture)
{
    Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    dombr.setContactDiscloseFlags(validated_mojeid_contact.data.id, set_flags, 0);

    const auto my_contact_info = ::LibFred::InfoContactByHandle{validated_mojeid_contact.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosename);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseorganization);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseemail);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseaddress);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosetelephone);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosefax);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseident);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosevat);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosenotifyemail);
}

struct SetContactDiscloseFlagsUserNotInMojeidFixture : MainFixture
{
    SetContactDiscloseFlagsUserNotInMojeidFixture()
        : MainFixture{},
          user_contact{UserContactProducer::operator()(ctx, reg_mojeid)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    Test::Contact user_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactDiscloseFlags non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_user_not_in_mojeid, SetContactDiscloseFlagsUserNotInMojeidFixture)
{
    try
    {
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        dombr.setContactDiscloseFlags(user_contact.data.id, set_flags, 0);
        BOOST_ERROR("unreported missing user");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetContactDiscloseFlagsUserNotIdentifiedFixture : MainFixture
{
    SetContactDiscloseFlagsUserNotIdentifiedFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactDiscloseFlags non-identified user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_user_not_identified, SetContactDiscloseFlagsUserNotIdentifiedFixture)
{
    try
    {
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        dombr.setContactDiscloseFlags(mojeid_contact.data.id, set_flags, 0);
        BOOST_ERROR("unreported missing user identification");
    }
    catch (const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetContactDiscloseFlagsContactBlockedFixture : MainFixture
{
    SetContactDiscloseFlagsContactBlockedFixture()
        : MainFixture{},
          blocked_mojeid_contact{
                UserContactProducer::operator()(
                        ctx, reg_mojeid, flag_mojeid_contact, flag_identified_contact, flag_server_blocked_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact blocked_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactDiscloseFlags blocked user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_contact_blocked, SetContactDiscloseFlagsContactBlockedFixture)
{
    try
    {
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        dombr.setContactDiscloseFlags(blocked_mojeid_contact.data.id, set_flags, 0);
        BOOST_ERROR("unreported blocked user contact");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectBlocked& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetContactDiscloseFlagsHideOrganizationAddressFixture : MainFixture
{
    SetContactDiscloseFlagsHideOrganizationAddressFixture()
        : MainFixture{},
          identified_mojeid_contact{
                UserContactProducer::operator()(
                        ctx,
                        MojeidCompanyCreator{}(reg_mojeid, *this),
                        flag_mojeid_contact,
                        flag_identified_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    struct MojeidCompanyCreator
    {
        LibFred::CreateContact operator()(const MojeidRegistrar& registrar, UserContactProducer& producer)
        {
            auto creator = producer.get_creator(registrar);
            creator.set_organization("TestOrganization");
            return creator;
        }
    };
    MojeidContact identified_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactDiscloseFlags hide address of organization
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_hide_organization_address, SetContactDiscloseFlagsHideOrganizationAddressFixture)
{
    try
    {
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        dombr.setContactDiscloseFlags(identified_mojeid_contact.data.id, set_flags, 0);
        BOOST_ERROR("unreported hide address of organization");
    }
    catch (const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/setContactDiscloseFlags


BOOST_AUTO_TEST_SUITE(setContactAuthInfo)

struct SetContactAuthinfoFixture : MainFixture
{
    SetContactAuthinfoFixture()
        : MainFixture{},
          identified_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_identified_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact identified_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call setContactAuthInfo with private data
*/
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo, SetContactAuthinfoFixture)
{
    BOOST_CHECK(dombr.setContactAuthInfo(identified_mojeid_contact.data.id, "newauthinfo", 42));

    const auto my_contact_info = ::LibFred::InfoContactByHandle{identified_mojeid_contact.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    BOOST_CHECK_LT(
            0,
            LibFred::Object::CheckAuthinfo{LibFred::Object::ObjectId{my_contact_info.info_contact_data.id}}
                    .exec(ctx, "newauthinfo", LibFred::Object::CheckAuthinfo::increment_usage));
}

struct SetValidatedContactAuthinfoFixture : MainFixture
{
    SetValidatedContactAuthinfoFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact validated_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call setContactAuthInfo with private data and validated contact
*/
BOOST_FIXTURE_TEST_CASE(set_validated_contact_authinfo, SetValidatedContactAuthinfoFixture)
{
    BOOST_CHECK(dombr.setContactAuthInfo(validated_mojeid_contact.data.id, "newauthinfo", 0));

    const auto my_contact_info = ::LibFred::InfoContactByHandle{validated_mojeid_contact.data.handle}.exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    BOOST_CHECK_LT(
            0,
            LibFred::Object::CheckAuthinfo{LibFred::Object::ObjectId{my_contact_info.info_contact_data.id}}
                    .exec(ctx, "newauthinfo", LibFred::Object::CheckAuthinfo::increment_usage));
}

struct SetContactAuthinfoUserNotInMojeidFixture : MainFixture
{
    SetContactAuthinfoUserNotInMojeidFixture()
        : MainFixture{},
          user_contact{UserContactProducer::operator()(ctx, reg_mojeid)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    Test::Contact user_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactAuthInfo non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_user_not_in_mojeid, SetContactAuthinfoUserNotInMojeidFixture)
{
    try
    {
        dombr.setContactAuthInfo(user_contact.data.id, "newauthinfo", 0);
        BOOST_ERROR("unreported missing user");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetContactAuthinfoUserNotIdentifiedFixture : MainFixture
{
    SetContactAuthinfoUserNotIdentifiedFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactAuthInfo non-identified user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_user_not_identified, SetContactAuthinfoUserNotIdentifiedFixture)
{
    try
    {
        dombr.setContactAuthInfo(mojeid_contact.data.id, "newauthinfo", 0);
        BOOST_ERROR("unreported missing user identification");
    }
    catch (const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetContactAuthinfoContactBlockedFixture : MainFixture
{
    SetContactAuthinfoContactBlockedFixture()
        : MainFixture{},
          blocked_mojeid_contact{
                UserContactProducer::operator()(
                        ctx,
                        reg_mojeid,
                        flag_mojeid_contact,
                        flag_identified_contact,
                        flag_server_blocked_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact blocked_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactAuthInfo blocked user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_contact_blocked, SetContactAuthinfoContactBlockedFixture)
{
    try
    {
        dombr.setContactAuthInfo(blocked_mojeid_contact.data.id, "newauthinfo", 0);
        BOOST_ERROR("unreported blocked user contact");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectBlocked& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetContactAuthinfoTooLongFixture : MainFixture
{
    SetContactAuthinfoTooLongFixture()
        : MainFixture{},
          identified_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_identified_contact)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact identified_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setContactAuthInfo too long
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_too_long, SetContactAuthinfoTooLongFixture)
{
    try
    {
        dombr.setContactAuthInfo(identified_mojeid_contact.data.id, std::string(329, 'a'), 0);
        BOOST_ERROR("unreported authinfo too long");
    }
    catch (const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/setContactAuthInfo


BOOST_AUTO_TEST_SUITE(setObjectBlockStatus)

struct RegistrantDomainFixture : MainFixture
{
    RegistrantDomainFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nsset{NssetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          keyset{KeysetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          domain{MyDomainProducer::operator()(ctx, registrar, validated_mojeid_contact, admin_contact, nsset, keyset)}
    { }
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Nsset nsset;
    Test::Keyset keyset;
    Test::Domain domain;
};

struct SetRegistrantDomainObjectBlockStatusFixture : RegistrantDomainFixture
{
    SetRegistrantDomainObjectBlockStatusFixture()
        : RegistrantDomainFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - domain with registrant, blocking transfer and update
 */

BOOST_FIXTURE_TEST_CASE(set_registrant_domain_object_block_status, SetRegistrantDomainObjectBlockStatusFixture)
{
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(::LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    commit(ctx);//to see changes performed by dombr call
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct SetRegistrantDomainObjectBlockStatusTransferFixture : RegistrantDomainFixture
{
    SetRegistrantDomainObjectBlockStatusTransferFixture()
        : RegistrantDomainFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - domain with registrant, blocking transfer
 */

BOOST_FIXTURE_TEST_CASE(set_registrant_domain_object_block_status_transfer, SetRegistrantDomainObjectBlockStatusTransferFixture)
{
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct AdminDomainFixture : MainFixture
{
    AdminDomainFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nsset{NssetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          keyset{KeysetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          domain{AdminDomainProducer::operator()(ctx, registrar, admin_contact, validated_mojeid_contact, nsset, keyset)}
    { }
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Nsset nsset;
    Test::Keyset keyset;
    Test::Domain domain;
};

struct SetAdminDomainObjectBlockStatusFixture : AdminDomainFixture
{
    SetAdminDomainObjectBlockStatusFixture()
        : AdminDomainFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - domain with admin, blocking transfer and update
 */
BOOST_FIXTURE_TEST_CASE(set_admin_domain_object_block_status, SetAdminDomainObjectBlockStatusFixture)
{
    ::LOGGER.info("set_admin_domain_object_block_status");
    LibFred::InfoDomainById{domain.data.id}.exec(ctx);
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(::LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct SetAdminDomainObjectBlockStatusTransferFixture : AdminDomainFixture
{
    SetAdminDomainObjectBlockStatusTransferFixture()
        : AdminDomainFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - domain with admin, blocking transfer
 */
BOOST_FIXTURE_TEST_CASE(set_admin_domain_object_block_status_transfer, SetAdminDomainObjectBlockStatusTransferFixture)
{
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(domain.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct AdminNssetFixture : MainFixture
{
    AdminNssetFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nsset{NssetProducer::operator()(
                ctx,
                registrar,
                { admin_contact.data.handle, validated_mojeid_contact.data.handle },
                LibFred::RegistrableObject::Nsset::ServerDeleteProhibited{})}
    { }
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Nsset nsset;
};

struct SetAdminNssetObjectBlockStatusFixture : AdminNssetFixture
{
    SetAdminNssetObjectBlockStatusFixture()
        : AdminNssetFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - nsset with admin, blocking transfer and update
 */
BOOST_FIXTURE_TEST_CASE(set_admin_nsset_object_block_status, SetAdminNssetObjectBlockStatusFixture)
{
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "nsset",
            { nsset.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(::LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "nsset",
            { nsset.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    BOOST_CHECK(!LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct SetAdminNssetObjectBlockStatusTransferFixture : AdminNssetFixture
{
    SetAdminNssetObjectBlockStatusTransferFixture()
        : AdminNssetFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - nsset with admin, blocking transfer
 */
BOOST_FIXTURE_TEST_CASE(set_admin_nsset_object_block_status_transfer, SetAdminNssetObjectBlockStatusTransferFixture)
{
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "nsset",
            { nsset.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "nsset",
            { nsset.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(!LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(nsset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct AdminKeysetFixture : MainFixture
{
    AdminKeysetFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          keyset{KeysetProducer::operator()(
                ctx,
                registrar,
                { admin_contact.data.handle, validated_mojeid_contact.data.handle },
                LibFred::RegistrableObject::Keyset::ServerDeleteProhibited{})}
    { }
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Keyset keyset;
};

struct SetAdminKeysetObjectBlockStatusFixture : AdminKeysetFixture
{
    SetAdminKeysetObjectBlockStatusFixture()
        : AdminKeysetFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - keyset with admin, blocking transfer and update
 */
BOOST_FIXTURE_TEST_CASE(set_admin_keyset_object_block_status, SetAdminKeysetObjectBlockStatusFixture)
{
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "keyset",
            { keyset.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(::LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "keyset",
            { keyset.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out);
    BOOST_CHECK(!LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct SetAdminKeysetObjectBlockStatusTransferFixture : AdminKeysetFixture
{
    SetAdminKeysetObjectBlockStatusTransferFixture()
        : AdminKeysetFixture{},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - keyset with admin, blocking transfer
 */
BOOST_FIXTURE_TEST_CASE(set_admin_keyset_object_block_status_transfer, SetAdminKeysetObjectBlockStatusTransferFixture)
{
    std::vector<std::string> blocked_objects_out;
    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "keyset",
            { keyset.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(::LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    commit(ctx);//unlock object state request lock

    dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "keyset",
            { keyset.data.id },
            Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER,
            blocked_objects_out);
    BOOST_CHECK(!LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
    BOOST_CHECK(!LibFred::ObjectHasState(keyset.data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
}

struct SetContactObjectBlockStatusFixture : MainFixture
{
    SetContactObjectBlockStatusFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    MojeidContact validated_mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - wrong object type contact
 */
BOOST_FIXTURE_TEST_CASE(set_contact_object_block_status, SetContactObjectBlockStatusFixture)
{
    try
    {
        std::vector<std::string> blocked_objects_out;
        dombr.setObjectBlockStatus(
                validated_mojeid_contact.data.id,
                "contact",
                { validated_mojeid_contact.data.id },
                Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
                blocked_objects_out);
        BOOST_ERROR("unreported objtype contact");
    }
    catch (const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetObjectBlockStatusMissingUserValidationFixture : MainFixture
{
    SetObjectBlockStatusMissingUserValidationFixture()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          keyset{KeysetProducer::operator()(
                ctx, registrar, { admin_contact.data.handle }, LibFred::RegistrableObject::Keyset::ServerDeleteProhibited{})},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Keyset keyset;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - missing user validation
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_missing_user_validation, SetObjectBlockStatusMissingUserValidationFixture)
{
    try
    {
        std::vector<std::string> blocked_objects_out;
        dombr.setObjectBlockStatus(
                mojeid_contact.data.id,
                "keyset",
                { keyset.data.id },
                Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
                blocked_objects_out);
        BOOST_ERROR("unreported missing user validation");
    }
    catch (const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetObjectBlockStatusWrongObjectTypeFixture : AdminKeysetFixture
{
    SetObjectBlockStatusWrongObjectTypeFixture()
        : AdminKeysetFixture{},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - wrong object type
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_wrong_object_type, SetObjectBlockStatusWrongObjectTypeFixture)
{
    try
    {
        std::vector<std::string> blocked_objects_out;
        dombr.setObjectBlockStatus(
                validated_mojeid_contact.data.id,
                "wrongtype",
                { keyset.data.id },
                Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
                blocked_objects_out);
        BOOST_ERROR("unreported wrong object type");
    }
    catch (const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetObjectBlockStatusEmptyInputFixture : AdminKeysetFixture
{
    SetObjectBlockStatusEmptyInputFixture()
        : AdminKeysetFixture{},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - zero size of input
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_empty_input, SetObjectBlockStatusEmptyInputFixture)
{
    std::vector<std::string> blocked_objects_out;
    BOOST_CHECK(!dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "keyset",
            {},
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out));
}

struct SetObjectBlockStatusBigInputFixture : AdminKeysetFixture
{
    SetObjectBlockStatusBigInputFixture()
        : AdminKeysetFixture{},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - input too big
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_big_input, SetObjectBlockStatusBigInputFixture)
{
    try
    {
        std::vector<std::string> blocked_objects_out;
        dombr.setObjectBlockStatus(
                validated_mojeid_contact.data.id,
                "keyset",
                std::vector<unsigned long long>(501, keyset.data.id),
                Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
                blocked_objects_out);
        BOOST_ERROR("unreported big input");
    }
    catch (const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetObjectBlockWrongObjectIdFixture : AdminKeysetFixture
{
    SetObjectBlockWrongObjectIdFixture()
        : AdminKeysetFixture{},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - wrong object id
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_wrong_object_id, SetObjectBlockWrongObjectIdFixture)
{
    try
    {
        std::vector<std::string> blocked_objects_out;
        dombr.setObjectBlockStatus(
                validated_mojeid_contact.data.id,
                "keyset",
                { keyset.data.id, 0u },
                Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
                blocked_objects_out);
        BOOST_ERROR("unreported wrong object id");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct SetObjectBlockBlockedObjectFixture : MainFixture
{
    SetObjectBlockBlockedObjectFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nsset{NssetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          keyset{KeysetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          domain{MyDomainProducer::operator()(
                ctx,
                registrar,
                validated_mojeid_contact,
                admin_contact,
                nsset,
                keyset,
                LibFred::RegistrableObject::Domain::ServerBlocked{})},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Nsset nsset;
    Test::Keyset keyset;
    Test::Domain domain;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test setObjectBlockStatus - blocked object
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_blocked_object, SetObjectBlockBlockedObjectFixture)
{
    std::vector<std::string> blocked_objects_out;
    BOOST_CHECK(!dombr.setObjectBlockStatus(
            validated_mojeid_contact.data.id,
            "domain",
            { domain.data.id },
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE,
            blocked_objects_out));
    BOOST_REQUIRE_EQUAL(blocked_objects_out.size(), 1);
    BOOST_CHECK_EQUAL(blocked_objects_out.at(0), domain.data.fqdn);
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/setObjectBlockStatus

BOOST_AUTO_TEST_SUITE(getDomainList)

struct GetMyDomainsFixture : MainFixture
{
    static decltype(auto) make_fqdn_domain_pair(Test::Domain domain)
    {
        auto pair = std::make_pair(std::string{""}, std::move(domain));
        pair.first = pair.second.data.fqdn;
        return pair;
    }
    explicit GetMyDomainsFixture(bool destroy_context = true)
        : MainFixture{},
          protection{get_protection()},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nsset{NssetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          keyset{KeysetProducer::operator()(ctx, registrar, { admin_contact.data.handle })},
          domains{make_fqdn_domain_pair(make_domain(false)),
                  make_fqdn_domain_pair(make_domain(
                        true,
                        [](LibFred::UpdateDomain& op)
                        {
                            const auto yesterday = boost::gregorian::day_clock::day_clock::local_day() -
                                                   boost::gregorian::days{1};
                            op.set_domain_expiration(yesterday);
                        })),
                  make_fqdn_domain_pair(make_domain(
                        false,
                        [this](LibFred::UpdateDomain& op)
                        {
                            const auto outzonedate = boost::gregorian::day_clock::day_clock::local_day() -
                                                     boost::gregorian::days{protection.outzone + 1};
                            op.set_domain_expiration(outzonedate);
                        })),
                  make_fqdn_domain_pair(make_domain(true)),
                  make_fqdn_domain_pair(make_domain(false)),
                  make_fqdn_domain_pair(make_domain(true)),
                  make_fqdn_domain_pair(make_domain(false)),
                  make_fqdn_domain_pair(make_domain(true)),
                  make_fqdn_domain_pair(make_domain(false)),
                  make_fqdn_domain_pair(make_domain(true))},
          dombr{(destroy_context ? Test::Commit{std::move(ctx)} : Test::Commit{ctx}), reg_mojeid}
    { }
    Test::Domain make_domain(bool blocked, std::function<void(LibFred::UpdateDomain&)> setter = nullptr)
    {
        auto creator = MyDomainProducer::get_creator(
                registrar,
                validated_mojeid_contact,
                admin_contact,
                nsset,
                keyset);
        auto domain = blocked ? Test::Domain{ctx, creator, LibFred::RegistrableObject::Domain::ServerBlocked{}}
                              : Test::Domain{ctx, creator};
        if (setter != nullptr)
        {
            LibFred::UpdateDomain op{domain.data.fqdn, registrar.data.handle};
            setter(op);
            op.exec(ctx);
            domain.data = LibFred::InfoDomainById{domain.data.id}.exec(ctx).info_domain_data;
        }
        return domain;
    }
    struct Protection
    {
        unsigned outzone;
        unsigned registration;
    };
    Protection get_protection()
    {
        const auto dbres = ctx.get_conn().exec(
                "SELECT (SELECT val::integer FROM enum_parameters WHERE name = 'expiration_dns_protection_period'), "
                       "(SELECT val::integer FROM enum_parameters WHERE name = 'expiration_registration_protection_period')");

        return Protection{static_cast<unsigned>(dbres[0][0]), static_cast<unsigned>(dbres[0][1])};
    }
    Protection protection;
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    Test::Nsset nsset;
    Test::Keyset keyset;
    std::map<std::string, Test::Domain> domains;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getDomainList
*/
BOOST_FIXTURE_TEST_CASE(get_my_domain_list, GetMyDomainsFixture)
{
    const auto dl = dombr.getDomainList(validated_mojeid_contact.data.id, Optional<unsigned long long>(),
            Optional<unsigned long long>(), Optional<unsigned long long>(), 0);
    const auto domain_list_out = dl.dld;

    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_code, "deleteCandidate");
    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_date, map_at(domains, domain_list_out.at(0).fqdn).data.expiration_date + boost::gregorian::days{protection.registration});

    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "outzone"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_code, "outzone");
    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_date, map_at(domains, domain_list_out.at(1).fqdn).data.expiration_date + boost::gregorian::days{protection.outzone});

    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_code, "expired");
    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_date, map_at(domains, domain_list_out.at(2).fqdn).data.expiration_date);
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for (unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(domain_list_out.at(i).id, map_at(domains, domain_list_out.at(i).fqdn).data.id);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).fqdn, map_at(domains, domain_list_out.at(i).fqdn).data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).user_role, "holder");
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        if ((i % 2) == 1)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if (2 < i)
            {
                BOOST_CHECK(std::any_of(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));
            }
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(!domain_list_out.at(i).is_server_blocked);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_contact, GetMyDomainsFixture)
{
    const auto dl = dombr.getDomainList(validated_mojeid_contact.data.id,
            admin_contact.data.id,
            Optional<unsigned long long>{},
            Optional<unsigned long long>{},
            0);
    const auto& domain_list_out = dl.dld;

    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_code, "deleteCandidate");
    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_date, map_at(domains, domain_list_out.at(0).fqdn).data.expiration_date + boost::gregorian::days{protection.registration});

    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "outzone"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_code, "outzone");
    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_date, map_at(domains, domain_list_out.at(1).fqdn).data.expiration_date + boost::gregorian::days{protection.outzone});

    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_code, "expired");
    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_date, map_at(domains, domain_list_out.at(2).fqdn).data.expiration_date);
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for (unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(domain_list_out.at(i).id, map_at(domains, domain_list_out.at(i).fqdn).data.id);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).fqdn, map_at(domains, domain_list_out.at(i).fqdn).data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK(domain_list_out.at(i).user_role == "admin");
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        if ((i % 2) == 1)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if (2 < i)
            {
                BOOST_CHECK(std::any_of(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));
            }
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(!domain_list_out.at(i).is_server_blocked);
        }
    }
}

struct GetMyDomainsWithContextFixture : GetMyDomainsFixture
{
    GetMyDomainsWithContextFixture() : GetMyDomainsFixture{false} { }
};

BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_nsset, GetMyDomainsWithContextFixture)
{
    //add user contact as nsset admin
    ::LibFred::UpdateNsset{nsset.data.handle, registrar.data.handle}.add_tech_contact(validated_mojeid_contact.data.handle).exec(ctx);
    commit(ctx);

    const auto dl = dombr.getDomainList(validated_mojeid_contact.data.id,
            Optional<unsigned long long>{},
            nsset.data.id,
            Optional<unsigned long long>{},
            0);
    const auto& domain_list_out = dl.dld;

    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_code, "deleteCandidate");
    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_date, map_at(domains, domain_list_out.at(0).fqdn).data.expiration_date + boost::gregorian::days{protection.registration});

    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "outzone"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_code, "outzone");
    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_date, map_at(domains, domain_list_out.at(1).fqdn).data.expiration_date + boost::gregorian::days{protection.outzone});

    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_code, "expired");
    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_date, map_at(domains, domain_list_out.at(2).fqdn).data.expiration_date);
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for (unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(domain_list_out.at(i).id, map_at(domains, domain_list_out.at(i).fqdn).data.id);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).fqdn, map_at(domains, domain_list_out.at(i).fqdn).data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).user_role, "holder");
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        if ((i % 2) == 1)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if (2 < i)
            {
                BOOST_CHECK(std::any_of(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));
            }
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(!domain_list_out.at(i).is_server_blocked);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_keyset, GetMyDomainsWithContextFixture)
{
    //add user contact as keyset admin
    ::LibFred::UpdateKeyset{keyset.data.handle, registrar.data.handle}.add_tech_contact(validated_mojeid_contact.data.handle).exec(ctx);
    commit(ctx);

    const auto dl = dombr.getDomainList(validated_mojeid_contact.data.id,
            Optional<unsigned long long>{},
            Optional<unsigned long long>{},
            keyset.data.id,
            0);
    const auto& domain_list_out = dl.dld;

    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_code, "deleteCandidate");
    BOOST_CHECK_EQUAL(domain_list_out.at(0).next_state.get_value().state_date, map_at(domains, domain_list_out.at(0).fqdn).data.expiration_date + boost::gregorian::days{protection.registration});

    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), [](auto&& state) { return state == "outzone"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_code, "outzone");
    BOOST_CHECK_EQUAL(domain_list_out.at(1).next_state.get_value().state_date, map_at(domains, domain_list_out.at(1).fqdn).data.expiration_date + boost::gregorian::days{protection.outzone});

    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "expired"; }));
    BOOST_CHECK(std::any_of(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));

    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_code, "expired");
    BOOST_CHECK_EQUAL(domain_list_out.at(2).next_state.get_value().state_date, map_at(domains, domain_list_out.at(2).fqdn).data.expiration_date);
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for (unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(domain_list_out.at(i).id, map_at(domains, domain_list_out.at(i).fqdn).data.id);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).fqdn, map_at(domains, domain_list_out.at(i).fqdn).data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).user_role, "holder");
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(domain_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        if ((i % 2) == 1)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if (2 < i)
            {
                BOOST_CHECK(std::any_of(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), [](auto&& state) { return state == "serverBlocked"; }));
            }
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(!domain_list_out.at(i).is_server_blocked);
        }
    }
}

struct GetDomainListUserNotInMojeidFixture : MainFixture
{
    GetDomainListUserNotInMojeidFixture()
        : MainFixture{},
          user_contact{UserContactProducer::operator()(ctx, reg_mojeid)},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    Test::Contact user_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getDomainList non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_user_not_in_mojeid, GetDomainListUserNotInMojeidFixture)
{
    try
    {
        dombr.getDomainList(user_contact.data.id,
                Optional<unsigned long long>{},
                Optional<unsigned long long>{},
                Optional<unsigned long long>{},
                0);

        BOOST_ERROR("unreported missing user");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not owned nsset
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_nsset_user_not_nsset_admin, GetMyDomainsFixture)
{
    try
    {
        dombr.getDomainList(validated_mojeid_contact.data.id,
                Optional<unsigned long long>{},
                nsset.data.id,
                Optional<unsigned long long>{},
                0);
        BOOST_ERROR("unreported missing nsset admin contact");
    }
    catch (const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not owned keyset
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_keyset_user_not_keyset_admin, GetMyDomainsFixture)
{
    try
    {
        dombr.getDomainList(validated_mojeid_contact.data.id,
                Optional<unsigned long long>{},
                Optional<unsigned long long>{},
                keyset.data.id,
                0);
        BOOST_ERROR("unreported missing keyset admin contact");
    }
    catch (const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not existing contact
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_not_existing_contact, GetMyDomainsFixture)
{
    try
    {
        dombr.getDomainList(validated_mojeid_contact.data.id,
                0,
                Optional<unsigned long long>{},
                Optional<unsigned long long>{},
                0);
        BOOST_ERROR("unreported missing contact");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getDomainList


BOOST_AUTO_TEST_SUITE(getNssetList)

struct GetMyNssetsFixture : MainFixture
{
    static decltype(auto) make_handle_nsset_pair(Test::Nsset nsset)
    {
        auto pair = std::make_pair(std::string{""}, std::move(nsset));
        pair.first = pair.second.data.handle;
        return pair;
    }
    explicit GetMyNssetsFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          nssets{make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset()),
                 make_handle_nsset_pair(make_nsset())},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    Test::Nsset make_nsset()
    {
        return NssetProducer::operator()(
                ctx,
                registrar,
                { admin_contact.data.handle, validated_mojeid_contact.data.handle});
    }
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    std::map<std::string, Test::Nsset> nssets;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getNssetList
*/
BOOST_FIXTURE_TEST_CASE(get_my_nsset_list, GetMyNssetsFixture)
{
    const auto nl = dombr.getNssetList(validated_mojeid_contact.data.id, Optional<unsigned long long>{}, 0);
    const auto& nsset_list_out = nl.nld;

    for (unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(nsset_list_out.at(i).id, map_at(nssets, nsset_list_out.at(i).handle).data.id);
        BOOST_CHECK_EQUAL(nsset_list_out.at(i).handle, map_at(nssets, nsset_list_out.at(i).handle).data.handle);

        BOOST_CHECK_EQUAL(nsset_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(nsset_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        BOOST_CHECK(!nsset_list_out.at(i).is_server_blocked);
    }
}

BOOST_FIXTURE_TEST_CASE(get_nsset_list_by_contact, GetMyNssetsFixture)
{
    const auto nl = dombr.getNssetList(validated_mojeid_contact.data.id, admin_contact.data.id, 0);
    const auto& nsset_list_out = nl.nld;
    for (unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(nsset_list_out.at(i).id, map_at(nssets, nsset_list_out.at(i).handle).data.id);
        BOOST_CHECK_EQUAL(nsset_list_out.at(i).handle, map_at(nssets, nsset_list_out.at(i).handle).data.handle);

        BOOST_CHECK_EQUAL(nsset_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(nsset_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        BOOST_CHECK(!nsset_list_out.at(i).is_server_blocked);
    }
}

/**
 * test getNssetList for not existing contact
 */
BOOST_FIXTURE_TEST_CASE(get_nsset_list_for_not_existing_contact, GetMyNssetsFixture)
{
    try
    {
        dombr.getNssetList(validated_mojeid_contact.data.id, 0, 0);
        BOOST_ERROR("unreported missing contact");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getNssetList


BOOST_AUTO_TEST_SUITE(getKeysetList)

struct GetMyKeysetsFixture : MainFixture
{
    static decltype(auto) make_handle_keyset_pair(Test::Keyset keyset)
    {
        auto pair = std::make_pair(std::string{""}, std::move(keyset));
        pair.first = pair.second.data.handle;
        return pair;
    }
    explicit GetMyKeysetsFixture()
        : MainFixture{},
          validated_mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact, flag_validated_contact)},
          registrar{TestRegistrarProducer::operator()(ctx)},
          admin_contact{AdminContactProducer::operator()(ctx, registrar)},
          keysets{make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset()),
                  make_handle_keyset_pair(make_keyset())},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    Test::Keyset make_keyset()
    {
        return KeysetProducer::operator()(
                ctx,
                registrar,
                { admin_contact.data.handle, validated_mojeid_contact.data.handle});
    }
    MojeidContact validated_mojeid_contact;
    TestRegistrar registrar;
    AdminContact admin_contact;
    std::map<std::string, Test::Keyset> keysets;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test call getKeysetList
*/
BOOST_FIXTURE_TEST_CASE(get_my_keyset_list, GetMyKeysetsFixture)
{
    const auto kl = dombr.getKeysetList(validated_mojeid_contact.data.id, Optional<unsigned long long>{}, 0);
    const auto& keyset_list_out = kl.kld;
    for (unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(keyset_list_out.at(i).id, map_at(keysets, keyset_list_out.at(i).handle).data.id);
        BOOST_CHECK_EQUAL(keyset_list_out.at(i).handle, map_at(keysets, keyset_list_out.at(i).handle).data.handle);

        BOOST_CHECK_EQUAL(keyset_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(keyset_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        BOOST_CHECK(!keyset_list_out.at(i).is_server_blocked);
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_list_by_contact, GetMyKeysetsFixture)
{
    const auto kl = dombr.getKeysetList(validated_mojeid_contact.data.id, admin_contact.data.id, 0);
    const auto& keyset_list_out = kl.kld;

    for (unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        BOOST_CHECK_EQUAL(keyset_list_out.at(i).id, map_at(keysets, keyset_list_out.at(i).handle).data.id);
        BOOST_CHECK_EQUAL(keyset_list_out.at(i).handle, map_at(keysets, keyset_list_out.at(i).handle).data.handle);

        BOOST_CHECK_EQUAL(keyset_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(keyset_list_out.at(i).registrar_name, boost::algorithm::replace_first_copy(registrar.data.handle, "-HANDLE", " NAME"));

        BOOST_CHECK(!keyset_list_out.at(i).is_server_blocked);
    }
}

/**
 * test getKeysetList for not existing contact
 */
BOOST_FIXTURE_TEST_CASE(get_keyset_list_for_not_existing_contact, GetMyKeysetsFixture)
{
    try
    {
        dombr.getKeysetList(validated_mojeid_contact.data.id, 0, 0);
        BOOST_ERROR("unreported missing contact");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getKeysetList


BOOST_AUTO_TEST_SUITE(getPublicStatusDesc)

struct GetPublicStatusDescFixture : MainFixture
{
    explicit GetPublicStatusDescFixture()
        : MainFixture{},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    DomainBrowserImplInstanceFixture dombr;
};

BOOST_FIXTURE_TEST_CASE(get_public_status_desc, GetPublicStatusDescFixture)
{
    const auto status_desc_out = dombr.getPublicStatusDesc("CS");
    BOOST_REQUIRE_LT(0, status_desc_out.size());
    for (unsigned long long i = 0 ; i < status_desc_out.size(); ++i)
    {
        BOOST_TEST_MESSAGE("code: " << status_desc_out.at(i).state_code << " desc: " << status_desc_out.at(i).state_desc);
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getPublicStatusDesc


BOOST_AUTO_TEST_SUITE(getObjectRegistryId)

struct GetMyContactObjectFixture : MainFixture
{
    GetMyContactObjectFixture()
        : MainFixture{},
          mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid, flag_mojeid_contact)},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

BOOST_FIXTURE_TEST_CASE(get_object_id, GetMyContactObjectFixture)
{
    BOOST_CHECK_EQUAL(mojeid_contact.data.id, dombr.getContactId(mojeid_contact.data.handle));
}

BOOST_FIXTURE_TEST_CASE(get_object_id_by_wrong_handle, GetMyContactObjectFixture)
{
    try
    {
        dombr.getContactId("_WRONG_HANDLE_");
        BOOST_ERROR("unreported wrong handle");
    }
    catch (const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getObjectRegistryId

class DomainProducer
{
public:
    DomainProducer() : index_{0} { }
    ::LibFred::CreateDomain get_creator(
            const Test::Registrar& sponsoring_registrar,
            const Test::Contact& registrant,
            const Test::Contact& admin_contact)
    {
        return ::LibFred::CreateDomain{
                make_fqdn<Test::CzZone>("testdomainadminowner", index_++),
                sponsoring_registrar.data.handle,
                registrant.data.handle,
                Optional<std::string>{},
                Optional<std::string>{},
                Optional<std::string>{},
                { admin_contact.data.handle },
                Optional<boost::gregorian::date>{},
                Optional<boost::gregorian::date>{},
                Optional<bool>{},
                Optional<unsigned long long>{}};
    }
    template <typename ...State>
    Test::Domain operator()(
            LibFred::OperationContext& ctx,
            const Test::Registrar& sponsoring_registrar,
            const Test::Contact& registrant,
            const Test::Contact& admin_contact,
            State ...state)
    {
        return Test::Domain{
                ctx,
                get_creator(sponsoring_registrar, registrant, admin_contact),
                state...};
    }
private:
    int index_;
};

struct MergeContactsFixture : MainFixture, DomainProducer
{
    MergeContactsFixture()
        : MainFixture{},
          mojeid_contact{make_mojeid_contact()},
          registrar{TestRegistrarProducer::operator()(ctx)},
          contact_0{make_contact_0()},
          contact_1{make_contact_1()},
          contact_2{make_contact_2()},
          contact_3{make_contact_3()},
          contact_4{make_contact_4()},
          contact_5{make_contact_5()},
          contact_6{make_contact_6()},
          contact_7{make_contact_7()},
          contact_8{make_contact_8()},
          contact_9{make_contact_9()},
          contact_10{make_contact_10()},
          contact_11{make_contact_11()},
          contact_12{make_contact_12()},
          contact_13{make_contact_13()},
          contact_14{make_contact_14()},
          contact_15{make_contact_15()},
          contact_16{make_contact_16()},
          contact_17{make_contact_17()},
          contact_18{make_contact_18()},
          contact_19{make_contact_19()},
          contact_20{make_contact_20()},
          contact_21{make_contact_21()},
          contact_22{make_contact_22()},
          contact_23{make_contact_23()},
          contact_24{make_contact_24()},
          contact_25{make_contact_25()},
          contact_merge_candidates_ids{contact_0.data.id,
                                       contact_1.data.id,
                                       contact_2.data.id,
                                       contact_3.data.id,
                                       contact_4.data.id,
                                       contact_5.data.id,
                                       contact_6.data.id,
                                       contact_7.data.id,
                                       contact_8.data.id,
                                       contact_25.data.id},
          dombr{Test::Commit{ctx}, reg_mojeid}
    { }
    MojeidContact make_mojeid_contact()
    {
        auto creator = UserContactProducer::get_creator(reg_mojeid);
        creator.set_vat("CZ1234567890")
               .set_ssntype("OP")
               .set_ssn("123456");
        return UserContactProducer::operator()(ctx, std::move(creator), flag_mojeid_contact, flag_validated_contact);
    }
    Test::Contact make_contact_0()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(mojeid_contact.data.name.get_value())
               .set_place(mojeid_contact.data.place.get_value())
               .set_vat(mojeid_contact.data.vat.get_value())
               .set_ssntype(mojeid_contact.data.ssntype.get_value())
               .set_ssn(mojeid_contact.data.ssn.get_value());
        auto contact = Test::Contact{ctx, std::move(creator)};
        NssetProducer::operator()(ctx, registrar, { contact.data.handle });
        KeysetProducer::operator()(ctx, registrar, { contact.data.handle });
        DomainProducer::operator()(ctx, registrar, contact, contact);
        return contact;
    }
    Test::Contact make_contact_1()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(contact_0.data.name.get_value())
               .set_place(contact_0.data.place.get_value())
               .set_ssntype(contact_0.data.ssntype.get_value())
               .set_ssn(contact_0.data.ssn.get_value());
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_2()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(contact_0.data.name.get_value())
               .set_place(contact_0.data.place.get_value())
               .set_vat(contact_0.data.vat.get_value());
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_3()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(contact_0.data.name.get_value())
               .set_place(contact_0.data.place.get_value())
               .set_vat(contact_0.data.vat.get_value())
               .set_ssn(contact_0.data.ssn.get_value());
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_4()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(contact_0.data.name.get_value())
               .set_place(contact_0.data.place.get_value())
               .set_vat(contact_0.data.vat.get_value())
               .set_ssntype(contact_0.data.ssntype.get_value());
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_5()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(contact_0.data.name.get_value())
               .set_place(contact_0.data.place.get_value())
               .set_vat(" " + contact_0.data.vat.get_value() + " ")
               .set_ssntype(contact_0.data.ssntype.get_value())
               .set_ssn(contact_0.data.ssn.get_value());
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_6()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(contact_0.data.name.get_value())
               .set_place(contact_0.data.place.get_value())
               .set_vat(contact_0.data.vat.get_value())
               .set_ssntype(contact_0.data.ssntype.get_value())
               .set_ssn(" " + contact_0.data.ssn.get_value() + " ");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_7()
    {
        auto creator = the_same_as_contact_0();
        creator.set_name(" " + contact_0.data.name.get_value() + " ");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_8()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.city = " " + place.city + " ";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_9()
    {
        return Test::Contact{ctx, the_same_as_contact_0(), flag_mojeid_contact};
    }
    Test::Contact make_contact_10()
    {
        return Test::Contact{ctx, the_same_as_contact_0(), flag_server_blocked_contact};
    }
    Test::Contact make_contact_11()
    {
        return Test::Contact{ctx, the_same_as_contact_0(), LibFred::RegistrableObject::Contact::ServerDeleteProhibited{}};
    }
    Test::Contact make_contact_12()
    {
        auto creator = the_same_as_contact_0();
        creator.set_name("USER-CONTACT-HANDLE DIFFERENTNAME");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_13()
    {
        auto creator = the_same_as_contact_0();
        creator.set_organization("USER-CONTACT-HANDLE ORG");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_14()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.street1 = "DIFFERENTSTR1";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_15()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.street2 = "DIFFERENTSTR2";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_16()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.street3 = "DIFFERENTSTR3";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_17()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.city = "DifferentPraha";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_18()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.postalcode = "11151";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_19()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.stateorprovince = "different";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_20()
    {
        auto creator = the_same_as_contact_0();
        auto place = contact_0.data.place.get_value();
        place.country = "SK";
        creator.set_place(place);
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_21()
    {
        auto creator = the_same_as_contact_0();
        creator.set_email("test@test.cz");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_22()
    {
        auto creator = the_same_as_contact_0();
        creator.set_vat("SK1234567890");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_23()
    {
        auto creator = the_same_as_contact_0();
        creator.set_ssn("223456");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_24()
    {
        auto creator = the_same_as_contact_0();
        creator.set_ssntype("RC");
        return Test::Contact{ctx, std::move(creator)};
    }
    Test::Contact make_contact_25()
    {
        auto contact = Test::Contact{ctx, the_same_as_contact_0(), flag_server_blocked_contact};
        ::LibFred::CancelObjectStateRequestId{contact.data.id, { flag_server_blocked_contact.name }}.exec(ctx);
        ::LibFred::PerformObjectStateRequest{contact.data.id}.exec(ctx);
        return contact;
    }
    LibFred::CreateContact the_same_as_contact_0()
    {
        auto creator = UserContactProducer::get_creator(registrar);
        creator.set_name(contact_0.data.name.get_value())
               .set_place(contact_0.data.place.get_value())
               .set_vat(contact_0.data.vat.get_value())
               .set_ssntype(contact_0.data.ssntype.get_value())
               .set_ssn(contact_0.data.ssn.get_value());
        return creator;
    }
    MojeidContact mojeid_contact;
    TestRegistrar registrar;
    Test::Contact contact_0;
    Test::Contact contact_1;
    Test::Contact contact_2;
    Test::Contact contact_3;
    Test::Contact contact_4;
    Test::Contact contact_5;
    Test::Contact contact_6;
    Test::Contact contact_7;
    Test::Contact contact_8;
    Test::Contact contact_9;
    Test::Contact contact_10;
    Test::Contact contact_11;
    Test::Contact contact_12;
    Test::Contact contact_13;
    Test::Contact contact_14;
    Test::Contact contact_15;
    Test::Contact contact_16;
    Test::Contact contact_17;
    Test::Contact contact_18;
    Test::Contact contact_19;
    Test::Contact contact_20;
    Test::Contact contact_21;
    Test::Contact contact_22;
    Test::Contact contact_23;
    Test::Contact contact_24;
    Test::Contact contact_25;
    std::set<unsigned long long> contact_merge_candidates_ids;
    DomainBrowserImplInstanceFixture dombr;
};

BOOST_AUTO_TEST_SUITE(getMergeContactCandidateList)

/**
 * test getMergeContactCandidateList, check candidate list contacts from fixture
*/
BOOST_FIXTURE_TEST_CASE(get_candidate_contact_list, MergeContactsFixture)
{
    const auto mcl = dombr.getMergeContactCandidateList(mojeid_contact.data.id, 0);
    const auto& contact_list_out = mcl.mccl;

    BOOST_CHECK_EQUAL(contact_list_out.size(), contact_merge_candidates_ids.size());

    std::set<unsigned long long> contact_list_out_ids;
    const auto handle_to_id = std::map<std::string, unsigned long long>{
            std::make_pair(contact_0.data.handle, contact_0.data.id),
            std::make_pair(contact_1.data.handle, contact_1.data.id),
            std::make_pair(contact_2.data.handle, contact_2.data.id),
            std::make_pair(contact_3.data.handle, contact_3.data.id),
            std::make_pair(contact_4.data.handle, contact_4.data.id),
            std::make_pair(contact_5.data.handle, contact_5.data.id),
            std::make_pair(contact_6.data.handle, contact_6.data.id),
            std::make_pair(contact_7.data.handle, contact_7.data.id),
            std::make_pair(contact_8.data.handle, contact_8.data.id),
            std::make_pair(contact_25.data.handle, contact_25.data.id)};

    for (unsigned long long i = 0; i < contact_list_out.size(); ++i)
    {
        const auto id = map_at(handle_to_id, contact_list_out.at(i).handle);
        BOOST_CHECK_EQUAL(contact_list_out.at(i).id, id);
        contact_list_out_ids.insert(id);

        if (i == 0)
        {
            BOOST_CHECK_EQUAL(contact_list_out.at(i).domain_count, 1);
            BOOST_CHECK_EQUAL(contact_list_out.at(i).nsset_count, 1);
            BOOST_CHECK_EQUAL(contact_list_out.at(i).keyset_count, 1);
        }
        else
        {
            BOOST_CHECK_EQUAL(contact_list_out.at(i).domain_count, 0);
            BOOST_CHECK_EQUAL(contact_list_out.at(i).nsset_count, 0);
            BOOST_CHECK_EQUAL(contact_list_out.at(i).keyset_count, 0);
        }

        BOOST_CHECK_EQUAL(contact_list_out.at(i).registrar_handle, registrar.data.handle);
        BOOST_CHECK_EQUAL(contact_list_out.at(i).registrar_name, registrar.data.name);
    }

    BOOST_CHECK(contact_list_out_ids == contact_merge_candidates_ids);
}

struct GetDomainListUserNotInMojeidFixture : MainFixture
{
    GetDomainListUserNotInMojeidFixture()
        : MainFixture{},
          mojeid_contact{
                UserContactProducer::operator()(ctx, reg_mojeid)},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * test getMergeContactCandidateList, non-mojeid user
*/
BOOST_FIXTURE_TEST_CASE(get_candidate_contact_list_user_not_in_mojeid, GetDomainListUserNotInMojeidFixture)
{
    try
    {
        dombr.getMergeContactCandidateList(mojeid_contact.data.id, 0);
        BOOST_ERROR("unreported missing user");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/getMergeContactCandidateList


BOOST_AUTO_TEST_SUITE(mergeContacts)

/**
 * mergeContacts
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts, MergeContactsFixture)
{
    dombr.mergeContacts(mojeid_contact.data.id, { contact_0.data.id }, 0);

    const auto info = ::LibFred::InfoContactHistoryById{contact_0.data.id}.exec(ctx);
    BOOST_REQUIRE_LT(0, info.size());
    BOOST_CHECK(!info.back().info_contact_data.delete_time.isnull());
}

/**
 * mergeContacts, all candidates
*/
BOOST_FIXTURE_TEST_CASE(merge_all_contacts, MergeContactsFixture)
{
    dombr.mergeContacts(
            mojeid_contact.data.id,
            std::vector<unsigned long long>(begin(contact_merge_candidates_ids), end(contact_merge_candidates_ids)),
            0);

    std::for_each(begin(contact_merge_candidates_ids), end(contact_merge_candidates_ids), [this](auto&& id)
    {
        const auto info = ::LibFred::InfoContactHistoryById{id}.exec(ctx);
        BOOST_REQUIRE_LT(0, info.size());
        BOOST_CHECK(!info.back().info_contact_data.delete_time.isnull());
    });
}

struct MergeContactsUserNotInMojeid : MainFixture
{
    MergeContactsUserNotInMojeid()
        : MainFixture{},
          mojeid_contact{UserContactProducer::operator()(ctx, reg_mojeid)},
          dombr{Test::Commit{std::move(ctx)}, reg_mojeid}
    { }
    MojeidContact mojeid_contact;
    DomainBrowserImplInstanceFixture dombr;
};

/**
 * mergeContacts, non-mojeid user
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_user_not_in_mojeid, MergeContactsUserNotInMojeid)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { 0 }, 0);
        BOOST_ERROR("unreported missing user");
    }
    catch (const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts no src contacts throws invalid contacts
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_no_src_contacts, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { }, 0);
        BOOST_ERROR("unreported missing src contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is the same as dest contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_dst_contacts, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { mojeid_contact.data.id }, 0);
        BOOST_ERROR("unreported the same src and dest. contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is mojeid contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_mojeid_src_contact, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_9.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is blocked contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_blocked_src_contact, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_10.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is delete prohibited
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_delete_prohibited_src_contact, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_11.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in name
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_name, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_12.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in org
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_org, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_13.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in street1
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_street1, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_14.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in city
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_city, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_17.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in postalcode
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_postalcode, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_18.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in country
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_country, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_20.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in email
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_email, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_21.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in vat
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_vat, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_22.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in ssn
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_ssn, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_23.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in ssntype
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_ssntype, MergeContactsFixture)
{
    try
    {
        dombr.mergeContacts(mojeid_contact.data.id, { contact_24.data.id }, 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch (const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser/mergeContacts
BOOST_AUTO_TEST_SUITE_END()//TestDomainBrowser
