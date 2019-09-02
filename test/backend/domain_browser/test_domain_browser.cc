/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "src/deprecated/libfred/object_state/object_state_name.hh"
#include "libfred/object_state/object_has_state.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"

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

#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_domainbrowser_args.hh"

#include "src/backend/domain_browser/domain_browser.hh"

#include <boost/lexical_cast.hpp>

#include <string>
#include <utility>
#include <vector>
#include <map>

/**
 *  @file
 *  test domain browser
 */

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestDomainBrowser)

const std::string server_name = "test-domain-browser";

struct domain_browser_impl_instance_fixture
{
    std::string update_registrar_handle;
    unsigned int domain_list_limit;
    unsigned int nsset_list_limit;
    unsigned int keyset_list_limit;
    unsigned int contact_list_limit;
    Fred::Backend::DomainBrowser::DomainBrowser impl;

    domain_browser_impl_instance_fixture()
    : update_registrar_handle(CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleMojeIdArgs>()->registrar_handle)//MojeId registrar used for updates in domain browser
    , domain_list_limit(CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->domain_list_limit)//domain list chunk size
    , nsset_list_limit(CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->nsset_list_limit)//nsset list chunk size
    , keyset_list_limit(CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->keyset_list_limit)//keyset list chunk size
    , contact_list_limit(CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->contact_list_limit)//contact list chunk size

    , impl(server_name, update_registrar_handle, domain_list_limit, nsset_list_limit, keyset_list_limit, contact_list_limit)
    {}
};


struct user_contact_handle_fixture
{
    std::string xmark;
    std::string user_contact_handle;

    user_contact_handle_fixture()
    :xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , user_contact_handle(std::string("USER-CONTACT-HANDLE-")+xmark)
    {
        BOOST_TEST_MESSAGE(user_contact_handle);
    }
    ~user_contact_handle_fixture(){}
};



struct user_contact_fixture
: user_contact_handle_fixture
{
    ::LibFred::InfoContactOutput user_contact_info;

    user_contact_fixture()
    {
        ::LibFred::OperationContextCreator ctx;

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(user_contact_handle,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->registrar_handle)//MojeId registrar
            .set_name(std::string("USER-CONTACT-HANDLE NAME")+xmark)
            .set_place(place)
            .exec(ctx);

        user_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

        ctx.commit_transaction();//commit fixture
    }
    ~user_contact_fixture(){}
};

struct mojeid_user_contact_fixture
: user_contact_fixture
{
    mojeid_user_contact_fixture()
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::StatusList states;
        states.insert(::LibFred::ObjectState::MOJEID_CONTACT);
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_TEST_MESSAGE(::LibFred::ObjectState::MOJEID_CONTACT);
    }
    ~mojeid_user_contact_fixture(){}
};

struct test_registrar_fixture
{
    std::string xmark;
    std::string test_registrar_handle;

    test_registrar_fixture()
    :xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , test_registrar_handle(std::string("TEST-REGISTRAR-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;

        ::LibFred::CreateRegistrar(test_registrar_handle)
            .set_name(std::string("TEST-REGISTRAR NAME")+xmark)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_TEST_MESSAGE(test_registrar_handle);
    }
    ~test_registrar_fixture()
    {}
};

struct admin_contact_fixture
: virtual test_registrar_fixture
{
    std::string test_contact_handle;
    ::LibFred::InfoContactOutput test_contact_info;
    admin_contact_fixture()
    : test_contact_handle(std::string("TEST-ADMIN-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(test_contact_handle,test_registrar_handle).set_organization(std::string("TEST-ORGANIZATION")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        test_contact_info = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        ctx.commit_transaction();//commit fixture
    }
    ~admin_contact_fixture()
    {}
};

struct nsset_fixture
: virtual admin_contact_fixture
{
    ::LibFred::InfoNssetOutput nsset_info;
    std::string test_nsset_handle;
    nsset_fixture()
    : test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateNsset(test_nsset_handle, test_registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle))
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
            (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                (boost::asio::ip::address::from_string("127.0.0.3"))
                (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
            (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                (boost::asio::ip::address::from_string("127.0.0.4"))
                (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
            ).exec(ctx);

        nsset_info = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

        ::LibFred::CreateObjectStateRequestId(nsset_info.info_nsset_data.id,
            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        ::LibFred::PerformObjectStateRequest().set_object_id(nsset_info.info_nsset_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~nsset_fixture()
    {}
};

struct keyset_fixture
: virtual admin_contact_fixture
{
    ::LibFred::InfoKeysetOutput keyset_info;
    std::string test_keyset_handle;
    keyset_fixture()
    : test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateKeyset(test_keyset_handle, test_registrar_handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle))
        .set_dns_keys(Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

        keyset_info = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

        ::LibFred::CreateObjectStateRequestId(keyset_info.info_keyset_data.id,
            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        ::LibFred::PerformObjectStateRequest().set_object_id(keyset_info.info_keyset_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~keyset_fixture()
    {}
};


BOOST_AUTO_TEST_SUITE(getRegistrarDetail)

struct get_registrar_fixture
: mojeid_user_contact_fixture
  , test_registrar_fixture
  , domain_browser_impl_instance_fixture
{};

/**
 * test call getRegistrarDetail
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail, get_registrar_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoRegistrarOutput registrar_info = ::LibFred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    Fred::Backend::DomainBrowser::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_info.info_contact_data.id,
            test_registrar_handle);

    BOOST_CHECK(rd.id == registrar_info.info_registrar_data.id);
    BOOST_CHECK(rd.handle == registrar_info.info_registrar_data.handle);
    BOOST_CHECK(rd.name == registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK(rd.phone == registrar_info.info_registrar_data.telephone.get_value_or_default());
    BOOST_CHECK(rd.fax == registrar_info.info_registrar_data.fax.get_value_or_default());
    BOOST_CHECK(rd.url == registrar_info.info_registrar_data.url.get_value_or_default());
    BOOST_CHECK(rd.address == (registrar_info.info_registrar_data.street1.get_value_or_default()+ ", "
        + registrar_info.info_registrar_data.postalcode.get_value_or_default() + " "
        + registrar_info.info_registrar_data.city.get_value_or_default()));
    BOOST_TEST_MESSAGE(rd.address);
}

struct get_registrar_detail_no_user_fixture
: user_contact_handle_fixture
  , test_registrar_fixture
  , domain_browser_impl_instance_fixture
{};

/**
 * test getRegistrarDetail no contact
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_no_user, get_registrar_detail_no_user_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoRegistrarOutput registrar_info = ::LibFred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        Fred::Backend::DomainBrowser::RegistrarDetail rd = impl.getRegistrarDetail(0, test_registrar_handle);
        BOOST_ERROR("unreported missing user contact");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}



struct get_registrar_detail_not_mojeid_user_fixture
: user_contact_fixture
  , test_registrar_fixture
  , domain_browser_impl_instance_fixture
{};

/**
 * test getRegistrarDetail not mojeid contact
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_not_mojeid_user, get_registrar_detail_not_mojeid_user_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoRegistrarOutput registrar_info = ::LibFred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        Fred::Backend::DomainBrowser::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_info.info_contact_data.id,
                test_registrar_handle);
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct get_registrar_detail_no_registrar_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test getRegistrarDetail no registrar
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_no_registrar, get_registrar_detail_no_registrar_fixture)
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_info.info_contact_data.id,
                "NO-NO-REGISTRAR-HANDLE");
        BOOST_ERROR("unreported missing registrar");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getRegistrarDetail

BOOST_AUTO_TEST_SUITE(getContactDetail)

struct test_contact_fixture
: test_registrar_fixture
{
    std::string test_contact_handle;

    test_contact_fixture()
    :test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;

        BOOST_CHECK(!test_registrar_handle.empty());//expecting existing registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(test_contact_handle,test_registrar_handle)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_fixture()
    {}
};


struct get_my_contact_detail_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test call getContactDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_my_contact_detail, get_my_contact_detail_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoContactOutput my_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoRegistrarOutput sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle(my_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    Fred::Backend::DomainBrowser::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
            my_contact_info.info_contact_data.id);
    const ::LibFred::Contact::PlaceAddress mci_place = my_contact_info.info_contact_data.place.get_value_or_default();
    const Nullable<::LibFred::Contact::PlaceAddress> mci_mailing = optional_map_at<Nullable>(my_contact_info.info_contact_data.addresses,::LibFred::ContactAddressType::MAILING);

    BOOST_CHECK(cd.id == my_contact_info.info_contact_data.id);
    BOOST_CHECK(cd.handle == my_contact_info.info_contact_data.handle);
    BOOST_CHECK(cd.roid == my_contact_info.info_contact_data.roid);
    BOOST_CHECK(cd.sponsoring_registrar.id == sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK(cd.sponsoring_registrar.handle == sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK(cd.sponsoring_registrar.name == sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK(cd.creation_time == my_contact_info.info_contact_data.creation_time);
    BOOST_CHECK(cd.update_time.get_value_or_default() == my_contact_info.info_contact_data.update_time.get_value_or_default());
    BOOST_CHECK(cd.transfer_time.get_value_or_default() == my_contact_info.info_contact_data.transfer_time.get_value_or_default());
    BOOST_CHECK(cd.authinfopw == my_contact_info.info_contact_data.authinfopw);
    BOOST_CHECK(cd.name.get_value_or_default() == my_contact_info.info_contact_data.name.get_value_or_default());
    BOOST_CHECK(cd.organization.get_value_or_default() == my_contact_info.info_contact_data.organization.get_value_or_default());

    BOOST_CHECK(cd.permanent_address == mci_place);
    BOOST_CHECK(cd.mailing_address == mci_mailing);

    BOOST_CHECK(cd.telephone.get_value_or_default() == my_contact_info.info_contact_data.telephone.get_value_or_default());
    BOOST_CHECK(cd.fax.get_value_or_default() == my_contact_info.info_contact_data.fax.get_value_or_default());
    BOOST_CHECK(cd.email.get_value_or_default() == my_contact_info.info_contact_data.email.get_value_or_default());
    BOOST_CHECK(cd.notifyemail.get_value_or_default() == my_contact_info.info_contact_data.notifyemail.get_value_or_default());
    BOOST_CHECK(cd.vat.get_value_or_default() == my_contact_info.info_contact_data.vat.get_value_or_default());
    BOOST_CHECK(cd.ssntype.get_value_or_default() == my_contact_info.info_contact_data.ssntype.get_value_or_default());
    BOOST_CHECK(cd.ssn.get_value_or_default() == my_contact_info.info_contact_data.ssn.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.name == my_contact_info.info_contact_data.disclosename);
    BOOST_CHECK(cd.disclose_flags.organization == my_contact_info.info_contact_data.discloseorganization);
    BOOST_CHECK(cd.disclose_flags.email == my_contact_info.info_contact_data.discloseemail);
    BOOST_CHECK(cd.disclose_flags.address == my_contact_info.info_contact_data.discloseaddress);
    BOOST_CHECK(cd.disclose_flags.telephone == my_contact_info.info_contact_data.disclosetelephone);
    BOOST_CHECK(cd.disclose_flags.fax == my_contact_info.info_contact_data.disclosefax);
    BOOST_CHECK(cd.disclose_flags.ident == my_contact_info.info_contact_data.discloseident);
    BOOST_CHECK(cd.disclose_flags.vat == my_contact_info.info_contact_data.disclosevat);
    BOOST_CHECK(cd.disclose_flags.notify_email == my_contact_info.info_contact_data.disclosenotifyemail);
    BOOST_CHECK(std::find(cd.state_codes.begin(), cd.state_codes.end(),"mojeidContact")!= cd.state_codes.end());
    BOOST_CHECK(cd.is_owner == true);
}

struct get_contact_fixture
: mojeid_user_contact_fixture
, test_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test call getContactDetail with public data
*/
BOOST_FIXTURE_TEST_CASE(get_contact_detail, get_contact_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoContactOutput test_contact_info = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoRegistrarOutput sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle(test_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    Fred::Backend::DomainBrowser::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
            test_contact_info.info_contact_data.id);
    const ::LibFred::Contact::PlaceAddress tci_place = test_contact_info.info_contact_data.place.get_value_or_default();
    const Nullable<::LibFred::Contact::PlaceAddress> tci_mailing = optional_map_at<Nullable>(test_contact_info.info_contact_data.addresses,::LibFred::ContactAddressType::MAILING);

    BOOST_CHECK(cd.id == test_contact_info.info_contact_data.id);
    BOOST_CHECK(cd.handle == test_contact_info.info_contact_data.handle);
    BOOST_CHECK(cd.roid == test_contact_info.info_contact_data.roid);
    BOOST_CHECK(cd.sponsoring_registrar.id == sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK(cd.sponsoring_registrar.handle == sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK(cd.sponsoring_registrar.name == sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK(cd.creation_time == test_contact_info.info_contact_data.creation_time);
    BOOST_CHECK(cd.update_time.get_value_or_default() == test_contact_info.info_contact_data.update_time.get_value_or_default());
    BOOST_CHECK(cd.transfer_time.get_value_or_default() == test_contact_info.info_contact_data.transfer_time.get_value_or_default());
    BOOST_CHECK(cd.authinfopw == "********");
    BOOST_CHECK(cd.name.get_value_or_default() == test_contact_info.info_contact_data.name.get_value_or_default());
    BOOST_CHECK(cd.organization.get_value_or_default() == test_contact_info.info_contact_data.organization.get_value_or_default());

    BOOST_CHECK(cd.permanent_address == tci_place);
    BOOST_CHECK(cd.mailing_address == tci_mailing);

    BOOST_CHECK(cd.telephone.get_value_or_default() == test_contact_info.info_contact_data.telephone.get_value_or_default());
    BOOST_CHECK(cd.fax.get_value_or_default() == test_contact_info.info_contact_data.fax.get_value_or_default());
    BOOST_CHECK(cd.email.get_value_or_default() == test_contact_info.info_contact_data.email.get_value_or_default());
    BOOST_CHECK(cd.notifyemail.get_value_or_default() == test_contact_info.info_contact_data.notifyemail.get_value_or_default());
    BOOST_CHECK(cd.vat.get_value_or_default() == test_contact_info.info_contact_data.vat.get_value_or_default());
    BOOST_CHECK(cd.ssntype.get_value_or_default() == test_contact_info.info_contact_data.ssntype.get_value_or_default());
    BOOST_CHECK(cd.ssn.get_value_or_default() == test_contact_info.info_contact_data.ssn.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.name == test_contact_info.info_contact_data.disclosename);
    BOOST_CHECK(cd.disclose_flags.organization == test_contact_info.info_contact_data.discloseorganization);
    BOOST_CHECK(cd.disclose_flags.email == test_contact_info.info_contact_data.discloseemail);
    BOOST_CHECK(cd.disclose_flags.address == test_contact_info.info_contact_data.discloseaddress);
    BOOST_CHECK(cd.disclose_flags.telephone == test_contact_info.info_contact_data.disclosetelephone);
    BOOST_CHECK(cd.disclose_flags.fax == test_contact_info.info_contact_data.disclosefax);
    BOOST_CHECK(cd.disclose_flags.ident == test_contact_info.info_contact_data.discloseident);
    BOOST_CHECK(cd.disclose_flags.vat == test_contact_info.info_contact_data.disclosevat);
    BOOST_CHECK(cd.disclose_flags.notify_email == test_contact_info.info_contact_data.disclosenotifyemail);
    BOOST_CHECK(std::find(cd.state_codes.begin(), cd.state_codes.end(),"mojeidContact") == cd.state_codes.end());
    BOOST_CHECK(cd.is_owner == false);
}


struct get_contact_detail_no_user_fixture
: test_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test getContactDetail no user contact
*/
BOOST_FIXTURE_TEST_CASE(get_contact_detail_no_user, get_contact_detail_no_user_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoContactOutput test_contact_info = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        Fred::Backend::DomainBrowser::ContactDetail cd = impl.getContactDetail(0,test_contact_info.info_contact_data.id);

        BOOST_ERROR("unreported missing user contact");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct get_contact_detail_not_mojeid_user_fixture
: user_contact_fixture
  , test_contact_fixture
  , domain_browser_impl_instance_fixture
{};

/**
 * test getContactDetail not mojeid user contact
 */
BOOST_FIXTURE_TEST_CASE(get_contact_detail_not_mojeid_user, get_contact_detail_not_mojeid_user_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoContactOutput test_contact_info = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        Fred::Backend::DomainBrowser::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
                test_contact_info.info_contact_data.id);
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct get_contact_detail_no_test_contact_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test getContactDetail no contact
 */
BOOST_FIXTURE_TEST_CASE(get_contact_detail_no_test_contact, get_contact_detail_no_test_contact_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::ContactDetail d = impl.getContactDetail(user_contact_info.info_contact_data.id,0);
        BOOST_ERROR("unreported missing test contact");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//getContactDetail

BOOST_AUTO_TEST_SUITE(getDomainDetail)

struct registrant_contact_fixture
: virtual test_registrar_fixture
{
    std::string test_contact_handle;
    registrant_contact_fixture()
    :test_contact_handle(std::string("TEST-REGISTRANT-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(test_contact_handle,test_registrar_handle)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        ctx.commit_transaction();//commit fixture
    }
    ~registrant_contact_fixture()
    {}
};


struct get_my_domain_fixture
: mojeid_user_contact_fixture
  , nsset_fixture
  , keyset_fixture
  , domain_browser_impl_instance_fixture
{
    std::string test_fqdn;
    get_my_domain_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    {

        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateDomain(test_fqdn//const std::string& fqdn
                    , test_registrar_handle//const std::string& registrar
                    , user_contact_handle//const std::string& registrant
                    , Optional<std::string>("testpasswd")//const Optional<std::string>& authinfo
                    , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                    , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                    , Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)//const std::vector<std::string>& admin_contacts
                    , boost::gregorian::day_clock::local_day()+boost::gregorian::months(12)//const Optional<boost::gregorian::date>& expiration_date
                    , Optional<boost::gregorian::date>()
                    , Optional<bool>()
                    , 0//const Optional<unsigned long long> logd_request_id
                    ).exec(ctx);


        ::LibFred::InfoDomainOutput domain_info = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

        ::LibFred::CreateObjectStateRequestId(domain_info.info_domain_data.id,
            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        ::LibFred::PerformObjectStateRequest().set_object_id(domain_info.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }

    ~get_my_domain_fixture()
    {}
};

struct get_domain_fixture
: mojeid_user_contact_fixture
  , registrant_contact_fixture
  , nsset_fixture
  , keyset_fixture
  , domain_browser_impl_instance_fixture
{
    std::string test_fqdn;
    get_domain_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    {

        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateDomain(test_fqdn//const std::string& fqdn
                    , test_registrar_handle//const std::string& registrar
                    , registrant_contact_fixture::test_contact_handle//const std::string& registrant
                    , Optional<std::string>("testpasswd")//const Optional<std::string>& authinfo
                    , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                    , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                    , Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)//const std::vector<std::string>& admin_contacts
                    , boost::gregorian::day_clock::local_day()+boost::gregorian::months(12)//const Optional<boost::gregorian::date>& expiration_date
                    , Optional<boost::gregorian::date>()
                    , Optional<bool>()
                    , 0//const Optional<unsigned long long> logd_request_id
                    ).exec(ctx);
        ctx.commit_transaction();//commit fixture
    }

    ~get_domain_fixture()
    {}
};

/**
 * test call getDomainDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_my_domain_detail, get_my_domain_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoDomainOutput my_domain_info = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoRegistrarOutput sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle(my_domain_info.info_domain_data.sponsoring_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    ::LibFred::InfoNssetOutput nsset_info = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoKeysetOutput keyset_info = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    ::LibFred::InfoContactOutput admin_contact_info = ::LibFred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    Fred::Backend::DomainBrowser::DomainDetail d = impl.getDomainDetail(user_contact_info.info_contact_data.id,
            my_domain_info.info_domain_data.id);

    BOOST_CHECK(d.id == my_domain_info.info_domain_data.id);
    BOOST_CHECK(d.fqdn == my_domain_info.info_domain_data.fqdn);
    BOOST_CHECK(d.roid == my_domain_info.info_domain_data.roid);
    BOOST_CHECK(d.sponsoring_registrar.id == sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK(d.sponsoring_registrar.handle == sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK(d.sponsoring_registrar.name == sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK(d.creation_time == my_domain_info.info_domain_data.creation_time);
    BOOST_CHECK(d.update_time.get_value_or_default() == my_domain_info.info_domain_data.update_time.get_value_or_default());
    BOOST_CHECK(d.authinfopw == my_domain_info.info_domain_data.authinfopw);
    BOOST_CHECK(d.registrant.id == user_contact_info.info_contact_data.id);
    BOOST_CHECK(d.registrant.handle == user_contact_info.info_contact_data.handle);
    BOOST_CHECK(d.registrant.name == (user_contact_info.info_contact_data.organization.get_value_or_default().empty()
            ? user_contact_info.info_contact_data.name.get_value_or_default()
            : user_contact_info.info_contact_data.organization.get_value_or_default()));
    BOOST_CHECK(d.expiration_date == my_domain_info.info_domain_data.expiration_date);
    BOOST_CHECK(d.enum_domain_validation.get_value_or_default() == my_domain_info.info_domain_data.enum_domain_validation.get_value_or_default());
    BOOST_CHECK(d.nsset.id == nsset_info.info_nsset_data.id);
    BOOST_CHECK(d.nsset.handle == nsset_info.info_nsset_data.handle);
    BOOST_CHECK(d.keyset.id == keyset_info.info_keyset_data.id);
    BOOST_CHECK(d.keyset.handle == keyset_info.info_keyset_data.handle);
    BOOST_CHECK(d.admins.at(0).id == admin_contact_info.info_contact_data.id);
    BOOST_CHECK(d.admins.at(0).handle == admin_contact_info.info_contact_data.handle);
    BOOST_CHECK(d.admins.at(0).name == (admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
        ? admin_contact_info.info_contact_data.name.get_value_or_default()
        : admin_contact_info.info_contact_data.organization.get_value_or_default()));
    BOOST_CHECK(d.admins.size() == 1);
    BOOST_CHECK(std::find(d.state_codes.begin(), d.state_codes.end(),::LibFred::ObjectState::SERVER_BLOCKED)!= d.state_codes.end());
    BOOST_CHECK(d.is_owner == true);
}

struct get_domain_detail_no_domain_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test getDomainDetail no domain
 */
BOOST_FIXTURE_TEST_CASE(get_domain_detail_no_domain, get_domain_detail_no_domain_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::DomainDetail d = impl.getDomainDetail(user_contact_info.info_contact_data.id,0);
        BOOST_ERROR("unreported missing test domain");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getDomainDetail

BOOST_AUTO_TEST_SUITE(getNssetDetail)

struct get_nsset_fixture
: mojeid_user_contact_fixture
  , nsset_fixture
  , domain_browser_impl_instance_fixture
{};

/**
 * test call getNssetDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_nsset_detail, get_nsset_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoNssetOutput nsset_info = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoRegistrarOutput sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle(nsset_info.info_nsset_data.sponsoring_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoContactOutput admin_contact_info = ::LibFred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    Fred::Backend::DomainBrowser::NssetDetail n = impl.getNssetDetail(user_contact_info.info_contact_data.id,
            nsset_info.info_nsset_data.id);

    BOOST_CHECK(n.id == nsset_info.info_nsset_data.id);
    BOOST_CHECK(n.handle == nsset_info.info_nsset_data.handle);
    BOOST_CHECK(n.roid == nsset_info.info_nsset_data.roid);
    BOOST_CHECK(n.sponsoring_registrar.id == sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK(n.sponsoring_registrar.handle == sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK(n.sponsoring_registrar.name == sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK(n.creation_time == nsset_info.info_nsset_data.creation_time);
    BOOST_CHECK(n.transfer_time.get_value_or_default() == nsset_info.info_nsset_data.transfer_time.get_value_or_default());
    BOOST_CHECK(n.update_time.get_value_or_default() == nsset_info.info_nsset_data.update_time.get_value_or_default());

    BOOST_CHECK(n.create_registrar.id == sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK(n.create_registrar.handle == sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK(n.create_registrar.name == sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());

    BOOST_CHECK(n.update_registrar.id == 0);
    BOOST_CHECK(n.update_registrar.handle == "");
    BOOST_CHECK(n.update_registrar.name == "");

    BOOST_CHECK(n.authinfopw == "********");

    BOOST_CHECK(n.admins.at(0).id == admin_contact_info.info_contact_data.id);
    BOOST_CHECK(n.admins.at(0).handle == admin_contact_info.info_contact_data.handle);
    BOOST_CHECK(n.admins.at(0).name == (admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
        ? admin_contact_info.info_contact_data.name.get_value_or_default()
        : admin_contact_info.info_contact_data.organization.get_value_or_default()));
    BOOST_CHECK(n.admins.size() == 1);

    BOOST_CHECK(n.hosts.at(0).get_fqdn().compare("a.ns.nic.cz") == 0);
    BOOST_CHECK(n.hosts.at(0).get_inet_addr().at(0).to_string().compare("127.0.0.3") == 0);
    BOOST_CHECK(n.hosts.at(0).get_inet_addr().at(1).to_string().compare("127.1.1.3") == 0);
    BOOST_CHECK(n.hosts.at(1).get_fqdn().compare("b.ns.nic.cz") == 0);
    BOOST_CHECK(n.hosts.at(1).get_inet_addr().at(0).to_string().compare("127.0.0.4") == 0);
    BOOST_CHECK(n.hosts.at(1).get_inet_addr().at(1).to_string().compare("127.1.1.4") == 0);

    BOOST_CHECK(std::find(n.state_codes.begin(), n.state_codes.end(),::LibFred::ObjectState::SERVER_DELETE_PROHIBITED)!= n.state_codes.end());
    BOOST_CHECK(n.report_level == 0);

    BOOST_CHECK(n.is_owner == false);

}

struct get_nsset_detail_no_nsset_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test getNssetDetail no nsset
 */
BOOST_FIXTURE_TEST_CASE(get_nsset_detail_no_nsset, get_nsset_detail_no_nsset_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::NssetDetail d = impl.getNssetDetail(user_contact_info.info_contact_data.id,0);
        BOOST_ERROR("unreported missing test nsset");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//getNssetDetail

BOOST_AUTO_TEST_SUITE(getKeysetDetail)

struct get_keyset_fixture
: mojeid_user_contact_fixture
  , keyset_fixture
  , domain_browser_impl_instance_fixture
{};

/**
 * test call getKeysetDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_keyset_detail, get_keyset_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoKeysetOutput keyset_info = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoRegistrarOutput sponsoring_registrar_info = ::LibFred::InfoRegistrarByHandle(keyset_info.info_keyset_data.sponsoring_registrar_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    ::LibFred::InfoContactOutput admin_contact_info = ::LibFred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

    Fred::Backend::DomainBrowser::KeysetDetail k = impl.getKeysetDetail(user_contact_info.info_contact_data.id,
            keyset_info.info_keyset_data.id);

    BOOST_CHECK(k.id == keyset_info.info_keyset_data.id);
    BOOST_CHECK(k.handle == keyset_info.info_keyset_data.handle);
    BOOST_CHECK(k.roid == keyset_info.info_keyset_data.roid);
    BOOST_CHECK(k.sponsoring_registrar.id == sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK(k.sponsoring_registrar.handle == sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK(k.sponsoring_registrar.name == sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());
    BOOST_CHECK(k.creation_time == keyset_info.info_keyset_data.creation_time);
    BOOST_CHECK(k.transfer_time.get_value_or_default() == keyset_info.info_keyset_data.transfer_time.get_value_or_default());
    BOOST_CHECK(k.update_time.get_value_or_default() == keyset_info.info_keyset_data.update_time.get_value_or_default());

    BOOST_CHECK(k.create_registrar.id == sponsoring_registrar_info.info_registrar_data.id);
    BOOST_CHECK(k.create_registrar.handle == sponsoring_registrar_info.info_registrar_data.handle);
    BOOST_CHECK(k.create_registrar.name == sponsoring_registrar_info.info_registrar_data.name.get_value_or_default());

    BOOST_CHECK(k.update_registrar.id == 0);
    BOOST_CHECK(k.update_registrar.handle == "");
    BOOST_CHECK(k.update_registrar.name == "");

    BOOST_CHECK(k.authinfopw == "********");

    BOOST_CHECK(k.admins.at(0).id == admin_contact_info.info_contact_data.id);
    BOOST_CHECK(k.admins.at(0).handle == admin_contact_info.info_contact_data.handle);
    BOOST_CHECK(k.admins.at(0).name == (admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
        ? admin_contact_info.info_contact_data.name.get_value_or_default()
        : admin_contact_info.info_contact_data.organization.get_value_or_default()));
    BOOST_CHECK(k.admins.size() == 1);

    BOOST_CHECK(k.dnskeys.size() == 1);
    BOOST_CHECK(k.dnskeys.at(0).flags == 257);
    BOOST_CHECK(k.dnskeys.at(0).protocol == 3);
    BOOST_CHECK(k.dnskeys.at(0).alg == 5);
    BOOST_CHECK(k.dnskeys.at(0).key.compare("AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8") == 0);

    BOOST_CHECK(std::find(k.state_codes.begin(), k.state_codes.end(),::LibFred::ObjectState::SERVER_DELETE_PROHIBITED)!= k.state_codes.end());

    BOOST_CHECK(k.is_owner == false);

}

struct get_keyset_detail_no_keyset_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test getKeysetDetail no keyset
 */
BOOST_FIXTURE_TEST_CASE(get_keyset_detail_no_keyset, get_keyset_detail_no_keyset_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::KeysetDetail d = impl.getKeysetDetail(user_contact_info.info_contact_data.id,0);
        BOOST_ERROR("unreported missing test keyset");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//getKeysetDetail

BOOST_AUTO_TEST_SUITE(setContactDiscloseFlags)

struct set_contact_disclose_flags_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test call setContactDiscloseFlags with private data
*/
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags, set_contact_disclose_flags_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::StatusList states;
        states.insert(::LibFred::ObjectState::IDENTIFIED_CONTACT);
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::OperationContextCreator ctx;

    Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 42);

    ::LibFred::InfoContactOutput my_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosename);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseorganization);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseemail);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseaddress);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosetelephone);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosefax);
    BOOST_CHECK(my_contact_info.info_contact_data.discloseident);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosevat);
    BOOST_CHECK(my_contact_info.info_contact_data.disclosenotifyemail);
    BOOST_CHECK(!my_contact_info.logd_request_id.isnull() && my_contact_info.logd_request_id.get_value() == 42);
}


struct set_validated_contact_disclose_flags_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test call setContactDiscloseFlags with private data and validated contact
*/
BOOST_FIXTURE_TEST_CASE(set_validated_contact_disclose_flags, set_validated_contact_disclose_flags_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::StatusList states;
        states.insert(::LibFred::ObjectState::VALIDATED_CONTACT);
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);

    ::LibFred::InfoContactOutput my_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
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

struct set_contact_disclose_flags_user_not_in_mojeid_fixture
: user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test setContactDiscloseFlags non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_user_not_in_mojeid, set_contact_disclose_flags_user_not_in_mojeid_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported missing user");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


struct set_contact_disclose_flags_user_not_identified_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test setContactDiscloseFlags non-identified user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_user_not_identified, set_contact_disclose_flags_user_not_identified_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported missing user identification");
    }
    catch( const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


struct set_contact_disclose_flags_contact_blocked_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test setContactDiscloseFlags blocked user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_contact_blocked, set_contact_disclose_flags_contact_blocked_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::IDENTIFIED_CONTACT)(::LibFred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported blocked user contact");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectBlocked& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct set_contact_disclose_flags_hide_organization_address_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setContactDiscloseFlags hide address of organization
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_hide_organization_address, set_contact_disclose_flags_hide_organization_address_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::IDENTIFIED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ::LibFred::UpdateContactById(user_contact_info.info_contact_data.id
            , CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->registrar_handle)
        .set_organization("TestOrganization").exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;
        Fred::Backend::DomainBrowser::ContactDiscloseFlagsToSet set_flags;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported hide address of organization");
    }
    catch( const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//setContactDiscloseFlags

BOOST_AUTO_TEST_SUITE(setContactAuthInfo)


struct set_contact_authinfo_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test call setContactAuthInfo with private data
*/
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo, set_contact_authinfo_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::StatusList states;
        states.insert(::LibFred::ObjectState::IDENTIFIED_CONTACT);
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(impl.setContactAuthInfo(
        user_contact_info.info_contact_data.id,"newauthinfo", 42));

    ::LibFred::InfoContactOutput my_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    BOOST_CHECK(my_contact_info.info_contact_data.authinfopw.compare("newauthinfo")==0);
    BOOST_CHECK(!my_contact_info.logd_request_id.isnull() && my_contact_info.logd_request_id.get_value() == 42);
}

struct set_validated_contact_authinfo_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test call setContactAuthInfo with private data and validated contact
*/
BOOST_FIXTURE_TEST_CASE(set_validated_contact_authinfo, set_validated_contact_authinfo_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::StatusList states;
        states.insert(::LibFred::ObjectState::VALIDATED_CONTACT);
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(impl.setContactAuthInfo(
        user_contact_info.info_contact_data.id,"newauthinfo", 0));

    ::LibFred::InfoContactOutput my_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
    BOOST_CHECK(my_contact_info.info_contact_data.authinfopw.compare("newauthinfo")==0);
}


struct set_contact_authinfo_user_not_in_mojeid_fixture
: user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setContactAuthInfo non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_user_not_in_mojeid, set_contact_authinfo_user_not_in_mojeid_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,"newauthinfo", 0);
        BOOST_ERROR("unreported missing user");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


struct set_contact_authinfo_user_not_identified_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setContactAuthInfo non-identified user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_user_not_identified, set_contact_authinfo_user_not_identified_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,"newauthinfo", 0);
        BOOST_ERROR("unreported missing user identification");
    }
    catch( const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct set_contact_authinfo_contact_blocked_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setContactAuthInfo blocked user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_contact_blocked, set_contact_authinfo_contact_blocked_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::IDENTIFIED_CONTACT)(::LibFred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,"newauthinfo", 0);
        BOOST_ERROR("unreported blocked user contact");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectBlocked& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


struct set_contact_authinfo_the_same_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setContactAuthInfo the same authinfo
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_the_same, set_contact_authinfo_the_same_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::IDENTIFIED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.authinfopw, 0) == false);
    }
}

struct set_contact_authinfo_not_owner_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

struct set_contact_authinfo_too_long_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setContactAuthInfo too long
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_too_long, set_contact_authinfo_too_long_fixture )
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::IDENTIFIED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaa", 0);
        BOOST_ERROR("unreported authinfo too long");
    }
    catch( const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//setContactAuthInfo

BOOST_AUTO_TEST_SUITE(setObjectBlockStatus)

struct registrant_domain_fixture
: mojeid_user_contact_fixture
  , nsset_fixture
  , keyset_fixture
{
    std::string test_fqdn;
    ::LibFred::InfoDomainOutput domain_info;

    registrant_domain_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    {

        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateDomain(test_fqdn//const std::string& fqdn
                    , test_registrar_handle//const std::string& registrar
                    , user_contact_handle//const std::string& registrant
                    , Optional<std::string>("testpasswd")//const Optional<std::string>& authinfo
                    , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                    , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                    , Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)//const std::vector<std::string>& admin_contacts
                    , boost::gregorian::day_clock::local_day()+boost::gregorian::months(12)//const Optional<boost::gregorian::date>& expiration_date
                    , Optional<boost::gregorian::date>()
                    , Optional<bool>()
                    , 0//const Optional<unsigned long long> logd_request_id
                    ).exec(ctx);

        domain_info = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        ctx.commit_transaction();//commit fixture
    }

    ~registrant_domain_fixture()
    {}
};

struct set_registrant_domain_object_block_status_fixture
: registrant_domain_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - domain with registrant, blocking transfer and update
 */

BOOST_FIXTURE_TEST_CASE(set_registrant_domain_object_block_status, set_registrant_domain_object_block_status_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(::LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}

struct set_registrant_domain_object_block_status_transfer_fixture
: registrant_domain_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - domain with registrant, blocking transfer
 */

BOOST_FIXTURE_TEST_CASE(set_registrant_domain_object_block_status_transfer, set_registrant_domain_object_block_status_transfer_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}


struct admin_domain_fixture
: mojeid_user_contact_fixture
  , nsset_fixture
  , keyset_fixture
{
    std::string test_fqdn;
    ::LibFred::InfoDomainOutput domain_info;

    admin_domain_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    {

        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateDomain(test_fqdn//const std::string& fqdn
                    , test_registrar_handle//const std::string& registrar
                    , admin_contact_fixture::test_contact_handle//const std::string& registrant
                    , Optional<std::string>("testpasswd")//const Optional<std::string>& authinfo
                    , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                    , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                    , Util::vector_of<std::string>(user_contact_handle)//const std::vector<std::string>& admin_contacts
                    , boost::gregorian::day_clock::local_day()+boost::gregorian::months(12)//const Optional<boost::gregorian::date>& expiration_date
                    , Optional<boost::gregorian::date>()
                    , Optional<bool>()
                    , 0//const Optional<unsigned long long> logd_request_id
                    ).exec(ctx);

        domain_info = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        ctx.commit_transaction();//commit fixture
    }

    ~admin_domain_fixture()
    {}
};

struct set_admin_domain_object_block_status_fixture
: admin_domain_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - domain with admin, blocking transfer and update
 */
BOOST_FIXTURE_TEST_CASE(set_admin_domain_object_block_status, set_admin_domain_object_block_status_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(::LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}


struct set_admin_domain_object_block_status_transfer_fixture
: admin_domain_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - domain with admin, blocking transfer
 */
BOOST_FIXTURE_TEST_CASE(set_admin_domain_object_block_status_transfer, set_admin_domain_object_block_status_transfer_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(domain_info.info_domain_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}


struct admin_nsset_fixture
: mojeid_user_contact_fixture
  , admin_contact_fixture
{
    std::string test_nsset_handle;
    ::LibFred::InfoNssetOutput nsset_info;

    admin_nsset_fixture()
    : test_nsset_handle(std::string("TEST-NSSET-HANDLE")+mojeid_user_contact_fixture::xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateNsset(test_nsset_handle, test_registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                    (boost::asio::ip::address::from_string("127.0.0.3"))
                    (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                    (boost::asio::ip::address::from_string("127.0.0.4"))
                    (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
            ).exec(ctx);

        nsset_info = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

        ::LibFred::CreateObjectStateRequestId(nsset_info.info_nsset_data.id,
            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        ::LibFred::PerformObjectStateRequest().set_object_id(nsset_info.info_nsset_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }

    ~admin_nsset_fixture()
    {}
};


struct set_admin_nsset_object_block_status_fixture
: admin_nsset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - nsset with admin, blocking transfer and update
 */
BOOST_FIXTURE_TEST_CASE(set_admin_nsset_object_block_status, set_admin_nsset_object_block_status_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(::LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}

struct set_admin_nsset_object_block_status_transfer_fixture
: admin_nsset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - nsset with admin, blocking transfer
 */
BOOST_FIXTURE_TEST_CASE(set_admin_nsset_object_block_status_transfer, set_admin_nsset_object_block_status_transfer_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}


struct admin_keyset_fixture
: mojeid_user_contact_fixture
  , admin_contact_fixture
{
    std::string test_keyset_handle;
    ::LibFred::InfoKeysetOutput keyset_info;

    admin_keyset_fixture()
    : test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+mojeid_user_contact_fixture::xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateKeyset(test_keyset_handle, test_registrar_handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
        .set_dns_keys(Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

        keyset_info = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

        ::LibFred::CreateObjectStateRequestId(keyset_info.info_keyset_data.id,
            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        ::LibFred::PerformObjectStateRequest().set_object_id(keyset_info.info_keyset_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }

    ~admin_keyset_fixture()
    {}
};

struct set_admin_keyset_object_block_status_fixture
: admin_keyset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - keyset with admin, blocking transfer and update
 */
BOOST_FIXTURE_TEST_CASE(set_admin_keyset_object_block_status, set_admin_keyset_object_block_status_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(::LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}

struct set_admin_keyset_object_block_status_transfer_fixture
: admin_keyset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - keyset with admin, blocking transfer
 */
BOOST_FIXTURE_TEST_CASE(set_admin_keyset_object_block_status_transfer, set_admin_keyset_object_block_status_transfer_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(::LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Fred::Backend::DomainBrowser::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        ::LibFred::OperationContextCreator ctx;
        BOOST_CHECK(!LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_transfer_prohibited).exec(ctx));
        BOOST_CHECK(!LibFred::ObjectHasState(keyset_info.info_keyset_data.id, ::LibFred::Object_State::server_update_prohibited).exec(ctx));
    }
}

struct set_contact_object_block_status_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - wrong object type contact
 */
BOOST_FIXTURE_TEST_CASE(set_contact_object_block_status, set_contact_object_block_status_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "contact", Util::vector_of<unsigned long long>(user_contact_info.info_contact_data.id),
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported objtype contact");
    }
    catch(const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct set_object_block_status_missing_user_validation_fixture
: admin_keyset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - missing user validation
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_missing_user_validation, set_object_block_status_missing_user_validation_fixture)
{
    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported missing user validation");
    }
    catch(const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct set_object_block_status_wrong_object_type_fixture
: admin_keyset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - wrong object type
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_wrong_object_type, set_object_block_status_wrong_object_type_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "wrongtype", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported wrong object type");
    }
    catch(const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct set_object_block_status_empty_input_fixture
: admin_keyset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - zero size of input
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_empty_input, set_object_block_status_empty_input_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    BOOST_CHECK(!impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", std::vector<unsigned long long>(),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out));
}

struct set_object_block_status_big_input_fixture
: admin_keyset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - input too big
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_big_input, set_object_block_status_big_input_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "keyset", std::vector<unsigned long long>(501,keyset_info.info_keyset_data.id),
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported big input");
    }
    catch(const Fred::Backend::DomainBrowser::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct set_object_block_wrong_object_id_fixture
: admin_keyset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - wrong object id
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_wrong_object_id, set_object_block_wrong_object_id_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id)(0u),
            Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported wrong object id");
    }
    catch(const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct set_object_block_blocked_object_fixture
: admin_nsset_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setObjectBlockStatus - blocked object
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_blocked_object, set_object_block_blocked_object_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(::LibFred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    {
        ::LibFred::OperationContextCreator ctx;
        if(!LibFred::ObjectHasState(nsset_info.info_nsset_data.id, ::LibFred::Object_State::server_blocked).exec(ctx))
        {
            ctx.get_conn().exec_params(
                "INSERT INTO object_state_request (object_id, state_id)"
                " VALUES ($1::integer, (SELECT id FROM enum_object_states"
                " WHERE name = $2::text)) RETURNING id",
                Database::query_param_list
                    (nsset_info.info_nsset_data.id)
                    (::LibFred::ObjectState::SERVER_BLOCKED));
            ::LibFred::PerformObjectStateRequest().set_object_id(nsset_info.info_nsset_data.id).exec(ctx);
            ctx.commit_transaction();
        }
    }

    std::vector<std::string> blocked_objects_out;
    BOOST_CHECK(!impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Fred::Backend::DomainBrowser::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out));
    BOOST_CHECK(blocked_objects_out.size() == 1);
    BOOST_CHECK(blocked_objects_out.at(0) == nsset_info.info_nsset_data.handle);
}

BOOST_AUTO_TEST_SUITE_END();//setObjectBlockStatus

BOOST_AUTO_TEST_SUITE(getDomainList)

struct get_my_domains_fixture
: mojeid_user_contact_fixture
  , nsset_fixture
  , keyset_fixture
  , domain_browser_impl_instance_fixture
{
    std::string test_fqdn;
    std::map<std::string,::LibFred::InfoDomainOutput> domain_info;
    boost::gregorian::date current_local_day;
    unsigned outzone_protection;
    unsigned registration_protection;

    get_my_domains_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    , current_local_day(boost::gregorian::day_clock::day_clock::local_day())
    , outzone_protection(0)
    , registration_protection(0)
    {

        ::LibFred::OperationContextCreator ctx;
        for(int i = 0; i < 10; ++i)
        {
            std::ostringstream fqdn;
            fqdn << "n"<<i<<test_fqdn;
            ::LibFred::CreateDomain(fqdn.str()//const std::string& fqdn
                , test_registrar_handle//const std::string& registrar
                , user_contact_handle//const std::string& registrant
                , Optional<std::string>("testpasswd")//const Optional<std::string>& authinfo
                , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                , Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)//const std::vector<std::string>& admin_contacts
                , boost::gregorian::day_clock::local_day()+boost::gregorian::months(12)//const Optional<boost::gregorian::date>& expiration_date
                , Optional<boost::gregorian::date>()
                , Optional<bool>()
                , 0//const Optional<unsigned long long> logd_request_id
                ).exec(ctx);

            domain_info[fqdn.str()]= ::LibFred::InfoDomainByFqdn(fqdn.str()).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

            if(i%2)
            {
                BOOST_TEST_MESSAGE(fqdn.str() + " blocked");
                ::LibFred::CreateObjectStateRequestId(map_at(domain_info,fqdn.str()).info_domain_data.id,
                    Util::set_of<std::string>(::LibFred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                ::LibFred::PerformObjectStateRequest().set_object_id(map_at(domain_info,fqdn.str()).info_domain_data.id).exec(ctx);
            }
        }

        Database::Result protection_result = ctx.get_conn().exec(
        "SELECT (SELECT val::integer FROM enum_parameters WHERE name = 'expiration_dns_protection_period') AS outzone_protection, "
        " (SELECT val::integer FROM enum_parameters WHERE name = 'expiration_registration_protection_period') AS registration_protection ");

        outzone_protection = static_cast<unsigned>(protection_result[0]["outzone_protection"]);
        registration_protection = static_cast<unsigned>(protection_result[0]["registration_protection"]);

        {
            std::ostringstream fqdn;
            fqdn << "n"<<1<<test_fqdn;
            ::LibFred::UpdateDomain(fqdn.str(), test_registrar_handle).set_domain_expiration(
                    current_local_day - boost::gregorian::days(1)).exec(ctx);

            domain_info[fqdn.str()]= ::LibFred::InfoDomainByFqdn(fqdn.str()).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        }

        {   std::ostringstream fqdn;
            fqdn << "n"<<2<<test_fqdn;
            ::LibFred::UpdateDomain(fqdn.str(), test_registrar_handle).set_domain_expiration(
                    current_local_day - boost::gregorian::days(outzone_protection+1)).exec(ctx);
            domain_info[fqdn.str()]= ::LibFred::InfoDomainByFqdn(fqdn.str()).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        }

        ctx.commit_transaction();//commit fixture
    }

    ~get_my_domains_fixture()
    {}
};

/**
 * test call getDomainList
*/
BOOST_FIXTURE_TEST_CASE(get_my_domain_list, get_my_domains_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::DomainList dl = impl.getDomainList(user_contact_info.info_contact_data.id, Optional<unsigned long long>(),
        Optional<unsigned long long>(), Optional<unsigned long long>(),0);
    std::vector<Fred::Backend::DomainBrowser::DomainListData> domain_list_out = dl.dld;

    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_code == "deleteCandidate");
    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(0).fqdn).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "expired") != domain_list_out.at(0).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "outzone") != domain_list_out.at(0).state_code.end());

    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_code == "outzone");
    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(1).fqdn).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "expired") != domain_list_out.at(1).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "serverBlocked") != domain_list_out.at(1).state_code.end());

    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_code == "expired");
    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(2).fqdn).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).id == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.id);
        BOOST_CHECK(domain_list_out.at(i).fqdn == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK(domain_list_out.at(i).user_role == "holder");
        BOOST_CHECK(domain_list_out.at(i).registrar_handle == test_registrar_handle);
        BOOST_CHECK(domain_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));

        if(i%2)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), "serverBlocked") != domain_list_out.at(i).state_code.end());
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked == false);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_contact, get_my_domains_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::DomainList dl =  impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(admin_contact_fixture::test_contact_info.info_contact_data.id),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),0);
    std::vector<Fred::Backend::DomainBrowser::DomainListData> domain_list_out = dl.dld;

    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_code == "deleteCandidate");
    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(0).fqdn).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "expired") != domain_list_out.at(0).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "outzone") != domain_list_out.at(0).state_code.end());

    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_code == "outzone");
    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(1).fqdn).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "expired") != domain_list_out.at(1).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "serverBlocked") != domain_list_out.at(1).state_code.end());

    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_code == "expired");
    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(2).fqdn).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).id == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.id);
        BOOST_CHECK(domain_list_out.at(i).fqdn == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK(domain_list_out.at(i).user_role == "admin");
        BOOST_CHECK(domain_list_out.at(i).registrar_handle == test_registrar_handle);
        BOOST_CHECK(domain_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));

        if(i%2)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), "serverBlocked") != domain_list_out.at(i).state_code.end());
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked == false);
        }
    }
}


BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_nsset, get_my_domains_fixture )
{
    //add user contact as nsset admin
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::UpdateNsset(test_nsset_handle, test_registrar_handle).add_tech_contact(user_contact_info.info_contact_data.handle).exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::DomainList dl =  impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(nsset_info.info_nsset_data.id),
            Optional<unsigned long long>(),0);
    std::vector<Fred::Backend::DomainBrowser::DomainListData> domain_list_out = dl.dld;

    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_code == "deleteCandidate");
    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(0).fqdn).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "expired") != domain_list_out.at(0).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "outzone") != domain_list_out.at(0).state_code.end());

    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_code == "outzone");
    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(1).fqdn).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "expired") != domain_list_out.at(1).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "serverBlocked") != domain_list_out.at(1).state_code.end());

    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_code == "expired");
    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(2).fqdn).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).id == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.id);
        BOOST_CHECK(domain_list_out.at(i).fqdn == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK(domain_list_out.at(i).user_role == "holder");
        BOOST_CHECK(domain_list_out.at(i).registrar_handle == test_registrar_handle);
        BOOST_CHECK(domain_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));

        if(i%2)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), "serverBlocked") != domain_list_out.at(i).state_code.end());
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked == false);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_keyset, get_my_domains_fixture )
{
    //add user contact as keyset admin
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::UpdateKeyset(test_keyset_handle, test_registrar_handle).add_tech_contact(user_contact_info.info_contact_data.handle).exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::DomainList dl = impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(keyset_info.info_keyset_data.id),0);
    std::vector<Fred::Backend::DomainBrowser::DomainListData> domain_list_out = dl.dld;

    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_code == "deleteCandidate");
    BOOST_CHECK(domain_list_out.at(0).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(0).fqdn).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "expired") != domain_list_out.at(0).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(0).state_code.begin(), domain_list_out.at(0).state_code.end(), "outzone") != domain_list_out.at(0).state_code.end());

    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_code == "outzone");
    BOOST_CHECK(domain_list_out.at(1).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(1).fqdn).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "expired") != domain_list_out.at(1).state_code.end());
    BOOST_CHECK(std::find(domain_list_out.at(1).state_code.begin(), domain_list_out.at(1).state_code.end(), "serverBlocked") != domain_list_out.at(1).state_code.end());

    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_code == "expired");
    BOOST_CHECK(domain_list_out.at(2).next_state.get_value().state_date == (map_at(domain_info,domain_list_out.at(2).fqdn).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).state_code.empty());
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).id == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.id);
        BOOST_CHECK(domain_list_out.at(i).fqdn == map_at(domain_info,domain_list_out.at(i).fqdn).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).have_keyset);
        BOOST_CHECK(domain_list_out.at(i).user_role == "holder");
        BOOST_CHECK(domain_list_out.at(i).registrar_handle == test_registrar_handle);
        BOOST_CHECK(domain_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));

        if(i%2)
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(domain_list_out.at(i).state_code.begin(), domain_list_out.at(i).state_code.end(), "serverBlocked") != domain_list_out.at(i).state_code.end());
        }
        else
        {
            //BOOST_TEST_MESSAGE(domain_list_out.at(i).is_server_blocked ? "is_server_blocked: true" : "is_server_blocked: false");
            BOOST_CHECK(domain_list_out.at(i).is_server_blocked == false);
        }
    }
}


struct get_domain_list_user_not_in_mojeid_fixture
: user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test getDomainList non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_user_not_in_mojeid, get_domain_list_user_not_in_mojeid_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),Optional<unsigned long long>()
            ,0);

        BOOST_ERROR("unreported missing user");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not owned nsset
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_nsset_user_not_nsset_admin, get_my_domains_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        std::vector<Fred::Backend::DomainBrowser::DomainListData> domain_list_out;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(nsset_info.info_nsset_data.id),
            Optional<unsigned long long>(),0);

        BOOST_ERROR("unreported missing nsset admin contact");
    }
    catch( const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not owned keyset
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_keyset_user_not_keyset_admin, get_my_domains_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        std::vector<Fred::Backend::DomainBrowser::DomainListData> domain_list_out;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(keyset_info.info_keyset_data.id),0);

        BOOST_ERROR("unreported missing keyset admin contact");
    }
    catch( const Fred::Backend::DomainBrowser::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not existing contact
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_not_existing_contact, get_my_domains_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        std::vector<Fred::Backend::DomainBrowser::DomainListData> domain_list_out;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(0),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),0);

        BOOST_ERROR("unreported missing contact");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getDomainList

BOOST_AUTO_TEST_SUITE(getNssetList)

struct get_my_nssets_fixture
: mojeid_user_contact_fixture
  , admin_contact_fixture
  , domain_browser_impl_instance_fixture
{
    std::string test_nsset_handle;
    std::map<std::string,::LibFred::InfoNssetOutput> nsset_info;

    get_my_nssets_fixture()
    : test_nsset_handle(std::string("TEST_NSSET_")+user_contact_handle_fixture::xmark+"_")
    {
        ::LibFred::OperationContextCreator ctx;
        for(int i = 0; i < 10; ++i)
        {
            std::ostringstream nsset_handle;
            nsset_handle << test_nsset_handle << i;

            ::LibFred::CreateNsset(nsset_handle.str(), test_registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
                .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                    (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("127.0.0.3"))
                        (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
                    (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("127.0.0.4"))
                        (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

            nsset_info[nsset_handle.str()]= ::LibFred::InfoNssetByHandle(nsset_handle.str()).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

            if(i%2)
            {
                BOOST_TEST_MESSAGE(nsset_handle.str() + " blocked");
                ::LibFred::LockObjectStateRequestLock(map_at(nsset_info,nsset_handle.str()).info_nsset_data.id).exec(ctx);
                ctx.get_conn().exec_params(
                    "INSERT INTO object_state_request (object_id, state_id)"
                    " VALUES ($1::integer, (SELECT id FROM enum_object_states"
                    " WHERE name = $2::text)) RETURNING id",
                    Database::query_param_list
                        (map_at(nsset_info,nsset_handle.str()).info_nsset_data.id)
                        (::LibFred::ObjectState::SERVER_BLOCKED));
                ::LibFred::PerformObjectStateRequest().set_object_id(map_at(nsset_info,nsset_handle.str()).info_nsset_data.id).exec(ctx);
            }
        }

        ctx.commit_transaction();//commit fixture
    }

    ~get_my_nssets_fixture()
    {}
};

/**
 * test call getNssetList
*/
BOOST_FIXTURE_TEST_CASE(get_my_nsset_list, get_my_nssets_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::NssetList nl = impl.getNssetList(user_contact_info.info_contact_data.id, Optional<unsigned long long>(),0);
    std::vector<Fred::Backend::DomainBrowser::NssetListData> nsset_list_out = nl.nld;

    for(unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        BOOST_CHECK(nsset_list_out.at(i).id == map_at(nsset_info,nsset_list_out.at(i).handle).info_nsset_data.id);
        BOOST_CHECK(nsset_list_out.at(i).handle == map_at(nsset_info,nsset_list_out.at(i).handle).info_nsset_data.handle);

        BOOST_CHECK(nsset_list_out.at(i).registrar_handle == test_registrar_handle);//registrar handle
        BOOST_CHECK(nsset_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_CHECK(nsset_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(nsset_list_out.at(i).state_code.begin(), nsset_list_out.at(i).state_code.end(), "serverBlocked") != nsset_list_out.at(i).state_code.end());
        }
        else
        {
            BOOST_CHECK(nsset_list_out.at(i).is_server_blocked == false);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_nsset_list_by_contact, get_my_nssets_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::NssetList nl = impl.getNssetList(user_contact_info.info_contact_data.id,
        Optional<unsigned long long>(admin_contact_fixture::test_contact_info.info_contact_data.id), 0);
    std::vector<Fred::Backend::DomainBrowser::NssetListData> nsset_list_out = nl.nld;
    for(unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        BOOST_CHECK(nsset_list_out.at(i).id == map_at(nsset_info,nsset_list_out.at(i).handle).info_nsset_data.id);
        BOOST_CHECK(nsset_list_out.at(i).handle == map_at(nsset_info,nsset_list_out.at(i).handle).info_nsset_data.handle);

        BOOST_CHECK(nsset_list_out.at(i).registrar_handle == test_registrar_handle);//registrar handle
        BOOST_CHECK(nsset_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_CHECK(nsset_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(nsset_list_out.at(i).state_code.begin(), nsset_list_out.at(i).state_code.end(), "serverBlocked") != nsset_list_out.at(i).state_code.end());
        }
        else
        {
            BOOST_CHECK(nsset_list_out.at(i).is_server_blocked == false);
        }
    }
}

/**
 * test getNssetList for not existing contact
 */
BOOST_FIXTURE_TEST_CASE(get_nsset_list_for_not_existing_contact, get_my_nssets_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.getNssetList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(0), 0);
        BOOST_ERROR("unreported missing contact");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//getNssetList

BOOST_AUTO_TEST_SUITE(getKeysetList)

struct get_my_keysets_fixture
: mojeid_user_contact_fixture
  , admin_contact_fixture
  , domain_browser_impl_instance_fixture
{
    std::string test_keyset_handle;
    std::map<std::string,::LibFred::InfoKeysetOutput> keyset_info;

    get_my_keysets_fixture()
    : test_keyset_handle(std::string("TEST_KEYSET_")+user_contact_handle_fixture::xmark+"_")
    {
        ::LibFred::OperationContextCreator ctx;
        for(int i = 0; i < 10; ++i)
        {
            std::ostringstream keyset_handle;
            keyset_handle << test_keyset_handle << i;

            ::LibFred::CreateKeyset(keyset_handle.str(), test_registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
                .set_dns_keys(Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

            keyset_info[keyset_handle.str()]= ::LibFred::InfoKeysetByHandle(keyset_handle.str()).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);

            if(i%2)
            {
                BOOST_TEST_MESSAGE(keyset_handle.str() + " blocked");
                ::LibFred::LockObjectStateRequestLock(map_at(keyset_info,keyset_handle.str()).info_keyset_data.id).exec(ctx);
                ctx.get_conn().exec_params(
                    "INSERT INTO object_state_request (object_id, state_id)"
                    " VALUES ($1::integer, (SELECT id FROM enum_object_states"
                    " WHERE name = $2::text)) RETURNING id",
                    Database::query_param_list
                        (map_at(keyset_info,keyset_handle.str()).info_keyset_data.id)
                        (::LibFred::ObjectState::SERVER_BLOCKED));
                ::LibFred::PerformObjectStateRequest().set_object_id(map_at(keyset_info,keyset_handle.str()).info_keyset_data.id).exec(ctx);
            }
        }

        ctx.commit_transaction();//commit fixture
    }

    ~get_my_keysets_fixture()
    {}
};

/**
 * test call getKeysetList
*/
BOOST_FIXTURE_TEST_CASE(get_my_keyset_list, get_my_keysets_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::KeysetList kl = impl.getKeysetList(user_contact_info.info_contact_data.id, Optional<unsigned long long>(),0);
    std::vector<Fred::Backend::DomainBrowser::KeysetListData> keyset_list_out = kl.kld;
    for(unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        BOOST_CHECK(keyset_list_out.at(i).id == map_at(keyset_info,keyset_list_out.at(i).handle).info_keyset_data.id);
        BOOST_CHECK(keyset_list_out.at(i).handle == map_at(keyset_info,keyset_list_out.at(i).handle).info_keyset_data.handle);

        BOOST_CHECK(keyset_list_out.at(i).registrar_handle == test_registrar_handle);//registrar handle
        BOOST_CHECK(keyset_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_CHECK(keyset_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(keyset_list_out.at(i).state_code.begin(), keyset_list_out.at(i).state_code.end(), "serverBlocked") != keyset_list_out.at(i).state_code.end());
        }
        else
        {
            BOOST_CHECK(keyset_list_out.at(i).is_server_blocked == false);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_list_by_contact, get_my_keysets_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::KeysetList kl = impl.getKeysetList(user_contact_info.info_contact_data.id,
        Optional<unsigned long long>(admin_contact_fixture::test_contact_info.info_contact_data.id),0);
    std::vector<Fred::Backend::DomainBrowser::KeysetListData> keyset_list_out = kl.kld;

    for(unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        BOOST_CHECK(keyset_list_out.at(i).id == map_at(keyset_info,keyset_list_out.at(i).handle).info_keyset_data.id);
        BOOST_CHECK(keyset_list_out.at(i).handle == map_at(keyset_info,keyset_list_out.at(i).handle).info_keyset_data.handle);

        BOOST_CHECK(keyset_list_out.at(i).registrar_handle == test_registrar_handle);//registrar handle
        BOOST_CHECK(keyset_list_out.at(i).registrar_name == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_CHECK(keyset_list_out.at(i).is_server_blocked);
            if(i > 2) BOOST_CHECK(std::find(keyset_list_out.at(i).state_code.begin(), keyset_list_out.at(i).state_code.end(), "serverBlocked") != keyset_list_out.at(i).state_code.end());
        }
        else
        {
            BOOST_CHECK(keyset_list_out.at(i).is_server_blocked == false);
        }
    }
}

/**
 * test getKeysetList for not existing contact
 */
BOOST_FIXTURE_TEST_CASE(get_keyset_list_for_not_existing_contact, get_my_keysets_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.getKeysetList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(0),0);

        BOOST_ERROR("unreported missing contact");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getKeysetList

BOOST_AUTO_TEST_SUITE(getPublicStatusDesc)

BOOST_FIXTURE_TEST_CASE(get_public_status_desc, domain_browser_impl_instance_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    std::vector<Fred::Backend::DomainBrowser::StatusDesc> status_desc_out = impl.getPublicStatusDesc("CS");

    for(unsigned long long i = 0 ; i < status_desc_out.size(); ++i)
    {
        BOOST_TEST_MESSAGE(std::string("code: ") << status_desc_out.at(i).state_code << std::string(" desc: ") << status_desc_out.at(i).state_desc);
    }
}

BOOST_AUTO_TEST_SUITE_END();//getPublicStatusDesc

BOOST_AUTO_TEST_SUITE(getObjectRegistryId)

struct get_my_contact_object_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};

BOOST_FIXTURE_TEST_CASE(get_object_id, get_my_contact_object_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(user_contact_info.info_contact_data.id == impl.getContactId(
        user_contact_info.info_contact_data.handle));
}

BOOST_FIXTURE_TEST_CASE(get_object_id_by_wrong_handle, get_my_contact_object_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.getContactId("_WRONG_HANDLE_");

        BOOST_ERROR("unreported wrong handle");
    }
    catch( const Fred::Backend::DomainBrowser::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getObjectRegistryId

struct merge_contacts_fixture
: virtual user_contact_handle_fixture
, virtual test_registrar_fixture
, domain_browser_impl_instance_fixture
{
    ::LibFred::InfoContactOutput user_contact_info;

    std::string test_contact_handle;
    std::map<std::string,::LibFred::InfoContactOutput> contact_info;
    std::set<unsigned long long> contact_merge_candidates_ids;

    merge_contacts_fixture()
    : test_contact_handle(std::string("TEST_CONTACT_")+user_contact_handle_fixture::xmark+"_")
    {
        { //destination / user contact
            ::LibFred::OperationContextCreator ctx;
            ::LibFred::Contact::PlaceAddress place;
            place.street1 = std::string("STR1") + user_contact_handle_fixture::xmark;
            place.city = "Praha";
            place.postalcode = "11150";
            place.country = "CZ";
            ::LibFred::CreateContact(user_contact_handle,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->registrar_handle)//MojeId registrar
                .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                .set_place(place)
                .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                .exec(ctx);
            user_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
            ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, Util::set_of<std::string>(::LibFred::ObjectState::MOJEID_CONTACT)).exec(ctx);
            ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
            ctx.commit_transaction();//commit fixture
        }

        //source contacts
        ::LibFred::OperationContextCreator ctx;
        for(int i = 0; i < 26; ++i)
        {
            std::ostringstream contact_handle;
            contact_handle << test_contact_handle << i;
            bool merge_candidate = false;
            ::LibFred::Contact::PlaceAddress place;
            place.street1 = std::string("STR1") + user_contact_handle_fixture::xmark;
            place.city = "Praha";
            place.postalcode = "11150";
            place.country = "CZ";

            switch(i)
            {
                case 0: //the same as dest. - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    ::LibFred::CreateNsset(std::string("TEST-NSSET-HANDLE")+user_contact_handle_fixture::xmark, test_registrar_handle)
                        .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                            (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                                (boost::asio::ip::address::from_string("127.0.0.3"))
                                (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
                            (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                                (boost::asio::ip::address::from_string("127.0.0.4"))
                                (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
                            )
                            .set_tech_contacts(Util::vector_of<std::string>(contact_handle.str()))
                            .exec(ctx);

                    ::LibFred::CreateKeyset(std::string("TEST-KEYSET-HANDLE")+user_contact_handle_fixture::xmark, test_registrar_handle)
                            .set_tech_contacts(Util::vector_of<std::string>(contact_handle.str()))
                            .exec(ctx);

                    ::LibFred::CreateDomain(
                            std::string("testdomainadminowner")+user_contact_handle_fixture::xmark+".cz" //const std::string& fqdn
                            , test_registrar_handle //const std::string& registrar
                            , contact_handle.str() //registrant
                            )
                    .set_admin_contacts(Util::vector_of<std::string>(contact_handle.str()))
                    .exec(ctx);

                    merge_candidate = true;
                    break;
                case 1: //the same as dest. except missing vat - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 2: //the same as dest. except missing ssntype and ssn - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 3: //the same as dest. except missing ssntype, make no sence but - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 4: //the same as dest. except missing ssn, make no sence but - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 5: //the same as dest. except spaces in vat - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat(" CZ1234567890 ").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 6: //the same as dest. except spaces in ssn - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn(" 123456 ")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 7: //the same as dest. except spaces in name - ok
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string(" USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark+" ")
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn(" 123456 ")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 8: //the same as dest. except spaces in city - ok
                    place.city = "  Praha  ";
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 9: //the same as dest. except mojeidContact state - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " mojeidContact");
                        unsigned long long contact_id = ::LibFred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        ::LibFred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(::LibFred::ObjectState::MOJEID_CONTACT)).exec(ctx);
                        ::LibFred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = false;
                }
                    break;
                case 10: //the same as dest. except serverBlocked state - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " serverBlocked");
                        unsigned long long contact_id = ::LibFred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        ::LibFred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                        ::LibFred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = false;
                }
                    break;
                case 11: //the same as dest. except serverDeleteProhibited state - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " serverDeleteProhibited");
                        unsigned long long contact_id = ::LibFred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        ::LibFred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
                        ::LibFred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = false;
                }
                    break;
                case 12: //the same as dest. except the name - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE DIFFERENTNAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different name");

                    merge_candidate = false;
                }
                    break;
                case 13: //the same as dest. except the organization - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_organization(std::string("USER-CONTACT-HANDLE ORG")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different organization");

                    merge_candidate = false;
                }
                    break;
                case 14: //the same as dest. except the street1 - nok
                {
                    place.street1 = std::string("DIFFERENTSTR1") + user_contact_handle_fixture::xmark;
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different street1");

                    merge_candidate = false;
                }
                    break;
                case 15: //the same as dest. except the street2 - nok
                {
                    place.street2 = std::string("DIFFERENTSTR2") + user_contact_handle_fixture::xmark;
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different street2");

                    merge_candidate = false;
                }
                    break;
                case 16: //the same as dest. except the street3 - nok
                {
                    place.street3 = std::string("DIFFERENTSTR3") + user_contact_handle_fixture::xmark;
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different street3");

                    merge_candidate = false;
                }
                    break;
                case 17: //the same as dest. except the city - nok
                {
                    place.city = "DifferentPraha";
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different city");

                    merge_candidate = false;
                }
                    break;
                case 18: //the same as dest. except the postalcode - nok
                {
                    place.postalcode = "11151";
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different postalcode");

                    merge_candidate = false;
                }
                    break;
                case 19: //the same as dest. except the stateorprovince - nok
                {
                    place.stateorprovince = "different";
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different stateorprovince");

                    merge_candidate = false;
                }
                    break;
                case 20: //the same as dest. except the country - nok
                {
                    place.country = "SK";
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different country");

                    merge_candidate = false;
                }
                    break;
                case 21: //the same as dest. except the email - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place).set_email("test@test.cz")
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different email");

                    merge_candidate = false;
                }
                    break;
                case 22: //the same as dest. except the vat - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("SK1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different vat");

                    merge_candidate = false;
                }
                    break;
                case 23: //the same as dest. except the ssn - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("223456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different ssn");

                    merge_candidate = false;
                }
                    break;
                case 24: //the same as dest. except the ssntype - nok
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("RC").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " different ssntype");

                    merge_candidate = false;
                }
                    break;
                case 25: //the same as dest. canceled not allowed status - ok (#11067)
                {
                    ::LibFred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_TEST_MESSAGE(contact_handle.str() + " serverBlocked");
                        unsigned long long contact_id = ::LibFred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        ::LibFred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                        ::LibFred::PerformObjectStateRequest(contact_id).exec(ctx);
                        ::LibFred::CancelObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(::LibFred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                        ::LibFred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = true;
                }
                    break;
            }

            contact_info[contact_handle.str()] = ::LibFred::InfoContactByHandle(contact_handle.str()).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
            if (merge_candidate)
            {
                contact_merge_candidates_ids.insert(contact_info.at(contact_handle.str()).info_contact_data.id);
            }

        }

        ctx.commit_transaction();//commit fixture
    }

    ~merge_contacts_fixture()
    {}
};


BOOST_AUTO_TEST_SUITE(getMergeContactCandidateList)


/**
 * test getMergeContactCandidateList, check candidate list contacts from fixture
*/
BOOST_FIXTURE_TEST_CASE(get_candidate_contact_list, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    Fred::Backend::DomainBrowser::MergeContactCandidateList mcl = impl.getMergeContactCandidateList(user_contact_info.info_contact_data.id, 0);
    std::vector<Fred::Backend::DomainBrowser::MergeContactCandidateData> contact_list_out = mcl.mccl;

    BOOST_CHECK(contact_list_out.size() == contact_merge_candidates_ids.size());

    std::set<unsigned long long> contact_list_out_ids;

    for(unsigned long long i = 0; i < contact_list_out.size(); ++i)
    {
        unsigned long long id = map_at(contact_info,contact_list_out.at(i).handle).info_contact_data.id;
        BOOST_CHECK(contact_list_out.at(i).id == id);
        BOOST_CHECK(contact_list_out.at(i).handle == map_at(contact_info,contact_list_out.at(i).handle).info_contact_data.handle);
        contact_list_out_ids.insert(id);

        if(i == 0)
        {
            BOOST_CHECK(contact_list_out.at(i).domain_count == 1);
            BOOST_CHECK(contact_list_out.at(i).nsset_count == 1);
            BOOST_CHECK(contact_list_out.at(i).keyset_count == 1);
        }
        else
        {
            BOOST_CHECK(contact_list_out.at(i).domain_count == 0);
            BOOST_CHECK(contact_list_out.at(i).nsset_count == 0);
            BOOST_CHECK(contact_list_out.at(i).keyset_count == 0);
        }

        BOOST_CHECK(contact_list_out.at(i).registrar_handle == std::string("TEST-REGISTRAR-HANDLE")+test_registrar_fixture::xmark);
        BOOST_CHECK(contact_list_out.at(i).registrar_name == std::string("TEST-REGISTRAR NAME")+test_registrar_fixture::xmark);
    }

    BOOST_CHECK(contact_list_out_ids == contact_merge_candidates_ids);
}

struct get_domain_list_user_not_in_mojeid_fixture
: user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * test getMergeContactCandidateList, non-mojeid user
*/

BOOST_FIXTURE_TEST_CASE(get_candidate_contact_list_user_not_in_mojeid, get_domain_list_user_not_in_mojeid_fixture )
{
    try
    {
        ::LibFred::OperationContextCreator ctx;
        impl.getMergeContactCandidateList(user_contact_info.info_contact_data.id, 0);

        BOOST_ERROR("unreported missing user");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//getMergeContactCandidateList


BOOST_AUTO_TEST_SUITE(mergeContacts)

/**
 * mergeContacts
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;

    impl.mergeContacts(user_contact_info.info_contact_data.id,
        Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"0").info_contact_data.id), 0);

    std::vector<::LibFred::InfoContactOutput> info = ::LibFred::InfoContactHistoryById(
        map_at(contact_info,test_contact_handle+"0").info_contact_data.id).exec(ctx);
    BOOST_CHECK(!info.back().info_contact_data.delete_time.isnull());

}

struct merge_contacts_user_not_in_mojeid_fixture
: user_contact_fixture
, domain_browser_impl_instance_fixture
{};

/**
 * mergeContacts, non-mojeid user
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_user_not_in_mojeid, merge_contacts_user_not_in_mojeid_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,Util::vector_of<unsigned long long>(0), 0);
        BOOST_ERROR("unreported missing user");
    }
    catch( const Fred::Backend::DomainBrowser::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


struct merge_contacts_no_src_contacts_fixture
: virtual user_contact_handle_fixture
, virtual test_registrar_fixture
, domain_browser_impl_instance_fixture
{
    ::LibFred::InfoContactOutput user_contact_info;

    std::string test_contact_handle;
    std::map<std::string,::LibFred::InfoContactOutput> contact_info;

    merge_contacts_no_src_contacts_fixture()
    : test_contact_handle(std::string("TEST_CONTACT_")+user_contact_handle_fixture::xmark+"_")
    {
        // user contact
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + user_contact_handle_fixture::xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(user_contact_handle,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIdArgs>()->registrar_handle)//MojeId registrar
            .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
            .set_place(place)
            .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
            .exec(ctx);
        user_contact_info = ::LibFred::InfoContactByHandle(user_contact_handle).exec(ctx, Fred::Backend::DomainBrowser::DomainBrowser::output_timezone);
        ::LibFred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, Util::set_of<std::string>(::LibFred::ObjectState::MOJEID_CONTACT)).exec(ctx);
        ::LibFred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();//commit fixture
    }
};

/**
 * mergeContacts no src contacts throws invalid contacts
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_no_src_contacts, merge_contacts_no_src_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,std::vector<unsigned long long>(), 0);
        BOOST_ERROR("unreported missing src contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is the same as dest contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_dst_contacts, merge_contacts_no_src_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,Util::vector_of<unsigned long long>(user_contact_info.info_contact_data.id), 0);
        BOOST_ERROR("unreported the same src and dest. contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is mojeid contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_mojeid_src_contact, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"9").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is blocked contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_blocked_src_contact, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"10").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is delete prohibited
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_delete_prohibited_src_contact, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"11").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in name
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_name, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"12").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in org
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_org, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"13").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in street1
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_street1, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"14").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in city
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_city, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"17").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in postalcode
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_postalcode, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"18").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in country
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_country, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"20").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in email
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_email, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"21").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in vat
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_vat, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"22").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in ssn
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_ssn, merge_contacts_fixture )
{
    ::LibFred::OperationContextCreator ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"23").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Fred::Backend::DomainBrowser::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//mergeContacts

BOOST_AUTO_TEST_SUITE_END();//TestDomainBrowser
