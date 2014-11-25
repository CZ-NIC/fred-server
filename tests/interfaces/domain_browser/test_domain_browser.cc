/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#include <string>
#include <utility>
#include <vector>
#include <map>

#include <boost/lexical_cast.hpp>

#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar_diff.h"
#include "src/fredlib/registrar/info_registrar_impl.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"
#include "util/map_at.h"

#include "random_data_generator.h"

#include "cfg/handle_mojeid_args.h"
#include "cfg/handle_domainbrowser_args.h"

#include "src/domain_browser/domain_browser.h"

/**
 *  @file
 *  test domain browser
 */

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
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
    Registry::DomainBrowserImpl::DomainBrowser impl;

    domain_browser_impl_instance_fixture()
    : update_registrar_handle(CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle)//MojeID registrar used for updates in domain browser
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
    :xmark(RandomDataGenerator().xnumstring(6))
    , user_contact_handle(std::string("USER-CONTACT-HANDLE-")+xmark)
    {
        BOOST_MESSAGE(user_contact_handle);
    }
    ~user_contact_handle_fixture(){}
};



struct user_contact_fixture
: user_contact_handle_fixture
{
    Fred::InfoContactOutput user_contact_info;

    user_contact_fixture()
    {
        Fred::OperationContext ctx;

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(user_contact_handle,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle)//MojeID registrar
            .set_name(std::string("USER-CONTACT-HANDLE NAME")+xmark)
            .set_place(place)
            .exec(ctx);

        user_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

        ctx.commit_transaction();//commit fixture
    }
    ~user_contact_fixture(){}
};

struct mojeid_user_contact_fixture
: user_contact_fixture
{
    mojeid_user_contact_fixture()
    {
        Fred::OperationContext ctx;
        Fred::StatusList states;
        states.insert(Fred::ObjectState::MOJEID_CONTACT);
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_MESSAGE(Fred::ObjectState::MOJEID_CONTACT);
    }
    ~mojeid_user_contact_fixture(){}
};

struct test_registrar_fixture
{
    std::string xmark;
    std::string test_registrar_handle;

    test_registrar_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , test_registrar_handle(std::string("TEST-REGISTRAR-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;

        Fred::CreateRegistrar(test_registrar_handle).set_name(std::string("TEST-REGISTRAR NAME")+xmark)
            .set_name(std::string("TEST-REGISTRAR NAME")+xmark)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
        BOOST_MESSAGE(test_registrar_handle);
    }
    ~test_registrar_fixture()
    {}
};

struct admin_contact_fixture
: virtual test_registrar_fixture
{
    std::string test_contact_handle;
    Fred::InfoContactOutput test_contact_info;
    admin_contact_fixture()
    : test_contact_handle(std::string("TEST-ADMIN-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(test_contact_handle,test_registrar_handle).set_organization(std::string("TEST-ORGANIZATION")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        test_contact_info = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
        ctx.commit_transaction();//commit fixture
    }
    ~admin_contact_fixture()
    {}
};

struct nsset_fixture
: virtual admin_contact_fixture
{
    Fred::InfoNssetOutput nsset_info;
    std::string test_nsset_handle;
    nsset_fixture()
    : test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        Fred::CreateNsset(test_nsset_handle, test_registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                (boost::asio::ip::address::from_string("127.0.0.3"))
                (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                (boost::asio::ip::address::from_string("127.0.0.4"))
                (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
            ).exec(ctx);

        nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

        Fred::CreateObjectStateRequestId(nsset_info.info_nsset_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(nsset_info.info_nsset_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~nsset_fixture()
    {}
};

struct keyset_fixture
: virtual admin_contact_fixture
{
    Fred::InfoKeysetOutput keyset_info;
    std::string test_keyset_handle;
    keyset_fixture()
    : test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        Fred::CreateKeyset(test_keyset_handle, test_registrar_handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle))
        .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

        keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

        Fred::CreateObjectStateRequestId(keyset_info.info_keyset_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(keyset_info.info_keyset_data.id).exec(ctx);

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
    Fred::OperationContext ctx;
    Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Registry::DomainBrowserImpl::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_info.info_contact_data.id,
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
    BOOST_MESSAGE(rd.address);
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
        Fred::OperationContext ctx;
        Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
        Registry::DomainBrowserImpl::RegistrarDetail rd = impl.getRegistrarDetail(0, test_registrar_handle);
        BOOST_ERROR("unreported missing user contact");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
        Registry::DomainBrowserImpl::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_info.info_contact_data.id,
                test_registrar_handle);
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_info.info_contact_data.id,
                "NO-NO-REGISTRAR-HANDLE");
        BOOST_ERROR("unreported missing registrar");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;

        BOOST_CHECK(!test_registrar_handle.empty());//expecting existing registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(test_contact_handle,test_registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
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
    Fred::OperationContext ctx;
    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(my_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

    Registry::DomainBrowserImpl::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
            my_contact_info.info_contact_data.id, "CS");
    const Fred::Contact::PlaceAddress mci_place = my_contact_info.info_contact_data.place.get_value_or_default();

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
    BOOST_CHECK(cd.street1.get_value_or_default() == mci_place.street1);
    BOOST_CHECK(cd.street2.get_value_or_default() == mci_place.street2.get_value_or_default());
    BOOST_CHECK(cd.street3.get_value_or_default() == mci_place.street3.get_value_or_default());
    BOOST_CHECK(cd.city.get_value_or_default() == mci_place.city);
    BOOST_CHECK(cd.stateorprovince.get_value_or_default() == mci_place.stateorprovince.get_value_or_default());
    BOOST_CHECK(cd.postalcode.get_value_or_default() == mci_place.postalcode);
    BOOST_CHECK(cd.country.get_value_or_default() == mci_place.country);
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
    BOOST_CHECK(cd.states.find_first_of("MojeID contact") != std::string::npos);
    BOOST_CHECK(cd.state_codes.find_first_of("mojeidContact") != std::string::npos);
    BOOST_CHECK(cd.is_owner == true);

    BOOST_MESSAGE(cd.states);
    BOOST_MESSAGE(cd.state_codes);
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
    Fred::OperationContext ctx;
    Fred::InfoContactOutput test_contact_info = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(test_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

    Registry::DomainBrowserImpl::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
            test_contact_info.info_contact_data.id, "CS");
    const Fred::Contact::PlaceAddress tci_place = test_contact_info.info_contact_data.place.get_value_or_default();

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
    BOOST_CHECK(cd.street1.get_value_or_default() == tci_place.street1);
    BOOST_CHECK(cd.street2.get_value_or_default() == tci_place.street2.get_value_or_default());
    BOOST_CHECK(cd.street3.get_value_or_default() == tci_place.street3.get_value_or_default());
    BOOST_CHECK(cd.city.get_value_or_default() == tci_place.city);
    BOOST_CHECK(cd.stateorprovince.get_value_or_default() == tci_place.stateorprovince.get_value_or_default());
    BOOST_CHECK(cd.postalcode.get_value_or_default() == tci_place.postalcode);
    BOOST_CHECK(cd.country.get_value_or_default() == tci_place.country);
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
    BOOST_CHECK(cd.states.find_first_of("MojeID contact") == std::string::npos);
    BOOST_CHECK(cd.state_codes.find_first_of("mojeidContact") == std::string::npos);
    BOOST_CHECK(cd.is_owner == false);

    BOOST_MESSAGE(cd.states);
    BOOST_MESSAGE(cd.state_codes);
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
        Fred::OperationContext ctx;
        Fred::InfoContactOutput test_contact_info = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
        Registry::DomainBrowserImpl::ContactDetail cd = impl.getContactDetail(0,test_contact_info.info_contact_data.id, "CS");

        BOOST_ERROR("unreported missing user contact");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::InfoContactOutput test_contact_info = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
        Registry::DomainBrowserImpl::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
                test_contact_info.info_contact_data.id, "CS");
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::ContactDetail d = impl.getContactDetail(user_contact_info.info_contact_data.id,0, "CS");
        BOOST_ERROR("unreported missing test contact");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(test_contact_handle,test_registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
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

        Fred::OperationContext ctx;
        Fred::CreateDomain(test_fqdn//const std::string& fqdn
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


        Fred::InfoDomainOutput domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

        Fred::CreateObjectStateRequestId(domain_info.info_domain_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(domain_info.info_domain_data.id).exec(ctx);

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

        Fred::OperationContext ctx;
        Fred::CreateDomain(test_fqdn//const std::string& fqdn
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
    Fred::OperationContext ctx;
    Fred::InfoDomainOutput my_domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(my_domain_info.info_domain_data.sponsoring_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

    Fred::InfoNssetOutput nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoKeysetOutput keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

    Fred::InfoContactOutput admin_contact_info = Fred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

    Registry::DomainBrowserImpl::DomainDetail d = impl.getDomainDetail(user_contact_info.info_contact_data.id,
            my_domain_info.info_domain_data.id, "CS");

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
    BOOST_CHECK(d.states.compare("Doména je blokována") == 0);
    BOOST_CHECK(d.state_codes.compare(Fred::ObjectState::SERVER_BLOCKED) == 0);
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
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainDetail d = impl.getDomainDetail(user_contact_info.info_contact_data.id,0, "CS");
        BOOST_ERROR("unreported missing test domain");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
    Fred::OperationContext ctx;
    Fred::InfoNssetOutput nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(nsset_info.info_nsset_data.sponsoring_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoContactOutput admin_contact_info = Fred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

    Registry::DomainBrowserImpl::NssetDetail n = impl.getNssetDetail(user_contact_info.info_contact_data.id,
            nsset_info.info_nsset_data.id, "CS");

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

    BOOST_CHECK(n.hosts.at(0).fqdn.compare("a.ns.nic.cz") == 0);
    BOOST_CHECK(n.hosts.at(0).inet_addr.compare("127.0.0.3, 127.1.1.3") == 0);
    BOOST_CHECK(n.hosts.at(1).fqdn.compare("b.ns.nic.cz") == 0);
    BOOST_CHECK(n.hosts.at(1).inet_addr.compare("127.0.0.4, 127.1.1.4") == 0);

    BOOST_CHECK(n.states.compare("Není povoleno smazání") == 0);
    BOOST_CHECK(n.state_codes.compare(Fred::ObjectState::SERVER_DELETE_PROHIBITED) == 0);

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
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::NssetDetail d = impl.getNssetDetail(user_contact_info.info_contact_data.id,0, "CS");
        BOOST_ERROR("unreported missing test nsset");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
    Fred::OperationContext ctx;
    Fred::InfoKeysetOutput keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(keyset_info.info_keyset_data.sponsoring_registrar_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    Fred::InfoContactOutput admin_contact_info = Fred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

    Registry::DomainBrowserImpl::KeysetDetail k = impl.getKeysetDetail(user_contact_info.info_contact_data.id,
            keyset_info.info_keyset_data.id, "CS");

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

    BOOST_CHECK(k.states.compare("Není povoleno smazání") == 0);
    BOOST_CHECK(k.state_codes.compare(Fred::ObjectState::SERVER_DELETE_PROHIBITED) == 0);

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
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::KeysetDetail d = impl.getKeysetDetail(user_contact_info.info_contact_data.id,0, "CS");
        BOOST_ERROR("unreported missing test keyset");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::StatusList states;
        states.insert(Fred::ObjectState::IDENTIFIED_CONTACT);
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Fred::OperationContext ctx;

    Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 42);

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    BOOST_CHECK(!my_contact_info.info_contact_data.disclosename);
    BOOST_CHECK(!my_contact_info.info_contact_data.discloseorganization);
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
        Fred::OperationContext ctx;
        Fred::StatusList states;
        states.insert(Fred::ObjectState::VALIDATED_CONTACT);
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Fred::OperationContext ctx;
    Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
    BOOST_CHECK(!my_contact_info.info_contact_data.disclosename);
    BOOST_CHECK(!my_contact_info.info_contact_data.discloseorganization);
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
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported missing user");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported missing user identification");
    }
    catch( const Registry::DomainBrowserImpl::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::IDENTIFIED_CONTACT)(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
        set_flags.email = true;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported blocked user contact");
    }
    catch( const Registry::DomainBrowserImpl::ObjectBlocked& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::IDENTIFIED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        Fred::UpdateContactById(user_contact_info.info_contact_data.id
            , CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle)
        .set_organization("TestOrganization").exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
        impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);
        BOOST_ERROR("unreported hide address of organization");
    }
    catch( const Registry::DomainBrowserImpl::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::StatusList states;
        states.insert(Fred::ObjectState::IDENTIFIED_CONTACT);
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Fred::OperationContext ctx;
    BOOST_CHECK(impl.setContactAuthInfo(
        user_contact_info.info_contact_data.id,
        user_contact_info.info_contact_data.id,"newauthinfo", 42));

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
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
        Fred::OperationContext ctx;
        Fred::StatusList states;
        states.insert(Fred::ObjectState::VALIDATED_CONTACT);
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, states).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Fred::OperationContext ctx;
    BOOST_CHECK(impl.setContactAuthInfo(
        user_contact_info.info_contact_data.id,
        user_contact_info.info_contact_data.id,"newauthinfo", 0));

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
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
        Fred::OperationContext ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.id,"newauthinfo", 0);
        BOOST_ERROR("unreported missing user");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.id,"newauthinfo", 0);
        BOOST_ERROR("unreported missing user identification");
    }
    catch( const Registry::DomainBrowserImpl::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::IDENTIFIED_CONTACT)(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.id,"newauthinfo", 0);
        BOOST_ERROR("unreported blocked user contact");
    }
    catch( const Registry::DomainBrowserImpl::ObjectBlocked& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::IDENTIFIED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.authinfopw, 0) == false);
    }
}

struct set_contact_authinfo_not_owner_fixture
: mojeid_user_contact_fixture
, domain_browser_impl_instance_fixture
{};
/**
 * test setContactAuthInfo not owner
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_not_owner, set_contact_authinfo_not_owner_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::IDENTIFIED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    user_contact_fixture another_contact;

    try
    {
        Fred::OperationContext ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            another_contact.user_contact_info.info_contact_data.id,"newauthinfo", 0);
        BOOST_ERROR("unreported non-owner user contact");
    }
    catch( const Registry::DomainBrowserImpl::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::IDENTIFIED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;
        impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.id,
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaa", 0);
        BOOST_ERROR("unreported authinfo too long");
    }
    catch( const Registry::DomainBrowserImpl::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
    Fred::InfoDomainOutput domain_info;

    registrant_domain_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    {

        Fred::OperationContext ctx;
        Fred::CreateDomain(test_fqdn//const std::string& fqdn
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

        domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }
}


struct admin_domain_fixture
: mojeid_user_contact_fixture
  , nsset_fixture
  , keyset_fixture
{
    std::string test_fqdn;
    Fred::InfoDomainOutput domain_info;

    admin_domain_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    {

        Fred::OperationContext ctx;
        Fred::CreateDomain(test_fqdn//const std::string& fqdn
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

        domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "domain", Util::vector_of<unsigned long long>(domain_info.info_domain_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(domain_info.info_domain_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }
}


struct admin_nsset_fixture
: mojeid_user_contact_fixture
  , admin_contact_fixture
{
    std::string test_nsset_handle;
    Fred::InfoNssetOutput nsset_info;

    admin_nsset_fixture()
    : test_nsset_handle(std::string("TEST-NSSET-HANDLE")+mojeid_user_contact_fixture::xmark)
    {
        Fred::OperationContext ctx;
        Fred::CreateNsset(test_nsset_handle, test_registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                    (boost::asio::ip::address::from_string("127.0.0.3"))
                    (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                    (boost::asio::ip::address::from_string("127.0.0.4"))
                    (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
            ).exec(ctx);

        nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

        Fred::CreateObjectStateRequestId(nsset_info.info_nsset_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(nsset_info.info_nsset_data.id).exec(ctx);

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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }
}


struct admin_keyset_fixture
: mojeid_user_contact_fixture
  , admin_contact_fixture
{
    std::string test_keyset_handle;
    Fred::InfoKeysetOutput keyset_info;

    admin_keyset_fixture()
    : test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+mojeid_user_contact_fixture::xmark)
    {
        Fred::OperationContext ctx;
        Fred::CreateKeyset(test_keyset_handle, test_registrar_handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
        .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

        keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

        Fred::CreateObjectStateRequestId(keyset_info.info_keyset_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(keyset_info.info_keyset_data.id).exec(ctx);

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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
    }

    impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
        Registry::DomainBrowserImpl::UNBLOCK_TRANSFER, blocked_objects_out);

    {
        Fred::OperationContext ctx;
        BOOST_CHECK(!Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx));
        BOOST_CHECK(!Fred::ObjectHasState(keyset_info.info_keyset_data.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "contact", Util::vector_of<unsigned long long>(user_contact_info.info_contact_data.id),
            Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported objtype contact");
    }
    catch(const Registry::DomainBrowserImpl::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
            Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported missing user validation");
    }
    catch(const Registry::DomainBrowserImpl::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "wrongtype", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id),
            Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported wrong object type");
    }
    catch(const Registry::DomainBrowserImpl::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    std::vector<std::string> blocked_objects_out;
    BOOST_CHECK(!impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "keyset", std::vector<unsigned long long>(),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "keyset", std::vector<unsigned long long>(501,keyset_info.info_keyset_data.id),
            Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported big input");
    }
    catch(const Registry::DomainBrowserImpl::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    try
    {
        std::vector<std::string> blocked_objects_out;
        impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
            "keyset", Util::vector_of<unsigned long long>(keyset_info.info_keyset_data.id)(0u),
            Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out);
        BOOST_ERROR("unreported wrong object id");
    }
    catch(const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    {
        Fred::OperationContext ctx;
        if(!Fred::ObjectHasState(nsset_info.info_nsset_data.id, Fred::ObjectState::SERVER_BLOCKED).exec(ctx))
        {
            ctx.get_conn().exec_params(
                "INSERT INTO object_state_request (object_id, state_id)"
                " VALUES ($1::integer, (SELECT id FROM enum_object_states"
                " WHERE name = $2::text)) RETURNING id",
                Database::query_param_list
                    (nsset_info.info_nsset_data.id)
                    (Fred::ObjectState::SERVER_BLOCKED));
            Fred::PerformObjectStateRequest().set_object_id(nsset_info.info_nsset_data.id).exec(ctx);
            ctx.commit_transaction();
        }
    }

    std::vector<std::string> blocked_objects_out;
    BOOST_CHECK(!impl.setObjectBlockStatus(user_contact_info.info_contact_data.id,
        "nsset", Util::vector_of<unsigned long long>(nsset_info.info_nsset_data.id),
        Registry::DomainBrowserImpl::BLOCK_TRANSFER_AND_UPDATE, blocked_objects_out));
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
    std::map<std::string,Fred::InfoDomainOutput> domain_info;
    boost::gregorian::date current_local_day;
    unsigned outzone_protection;
    unsigned registration_protection;

    get_my_domains_fixture()
    : test_fqdn(std::string("test")+test_registrar_fixture::xmark+".cz")
    , current_local_day(boost::gregorian::day_clock::day_clock::local_day())
    , outzone_protection(0)
    , registration_protection(0)
    {

        Fred::OperationContext ctx;
        for(int i = 0; i < 10; ++i)
        {
            std::ostringstream fqdn;
            fqdn << "n"<<i<<test_fqdn;
            Fred::CreateDomain(fqdn.str()//const std::string& fqdn
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

            domain_info[fqdn.str()]= Fred::InfoDomainByHandle(fqdn.str()).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

            if(i%2)
            {
                BOOST_MESSAGE(fqdn.str() + " blocked");
                Fred::CreateObjectStateRequestId(map_at(domain_info,fqdn.str()).info_domain_data.id,
                    Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                Fred::PerformObjectStateRequest().set_object_id(map_at(domain_info,fqdn.str()).info_domain_data.id).exec(ctx);
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
            Fred::UpdateDomain(fqdn.str(), test_registrar_handle).set_domain_expiration(
                    current_local_day - boost::gregorian::days(1)).exec(ctx);

            domain_info[fqdn.str()]= Fred::InfoDomainByHandle(fqdn.str()).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
        }

        {   std::ostringstream fqdn;
            fqdn << "n"<<2<<test_fqdn;
            Fred::UpdateDomain(fqdn.str(), test_registrar_handle).set_domain_expiration(
                    current_local_day - boost::gregorian::days(outzone_protection+1)).exec(ctx);
            domain_info[fqdn.str()]= Fred::InfoDomainByHandle(fqdn.str()).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
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
    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > domain_list_out;
    bool limit_exceeded = impl.getDomainList(user_contact_info.info_contact_data.id, Optional<unsigned long long>(),
            Optional<unsigned long long>(), Optional<unsigned long long>(),"CS",0,domain_list_out);

    std::ostringstream list_out;
    list_out << "domain_list_out: \n";

    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < domain_list_out.at(i).size(); ++j)
        {
            list_out << " " <<domain_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);


    BOOST_CHECK(domain_list_out.at(0).at(3) == "deleteCandidate");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(0).at(4)) == (map_at(domain_info,domain_list_out.at(0).at(1)).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména není generována do zóny") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(1).at(3) == "outzone");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(1).at(4)) == (map_at(domain_info,domain_list_out.at(1).at(1)).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je blokována") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(2).at(3) == "expired");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(2).at(4)) == (map_at(domain_info,domain_list_out.at(2).at(1)).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).at(9) == "");
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.id));
        BOOST_CHECK(domain_list_out.at(i).at(1) == map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).at(5) == "t");//have keyset
        BOOST_CHECK(domain_list_out.at(i).at(6) == "holder");//role
        BOOST_CHECK(domain_list_out.at(i).at(7) == test_registrar_handle);//registrar handle
        BOOST_CHECK(domain_list_out.at(i).at(8) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "t");
            if(i > 2) BOOST_CHECK(domain_list_out.at(i).at(9) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "f");
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_contact, get_my_domains_fixture )
{
    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > domain_list_out;
    bool limit_exceeded = impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(admin_contact_fixture::test_contact_info.info_contact_data.id),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),"CS",0,domain_list_out);

    std::ostringstream list_out;
    list_out << "domain_list_out: \n";

    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < domain_list_out.at(i).size(); ++j)
        {
            list_out << " " <<domain_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);


    BOOST_CHECK(domain_list_out.at(0).at(3) == "deleteCandidate");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(0).at(4)) == (map_at(domain_info,domain_list_out.at(0).at(1)).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména není generována do zóny") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(1).at(3) == "outzone");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(1).at(4)) == (map_at(domain_info,domain_list_out.at(1).at(1)).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je blokována") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(2).at(3) == "expired");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(2).at(4)) == (map_at(domain_info,domain_list_out.at(2).at(1)).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).at(9) == "");
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.id));
        BOOST_CHECK(domain_list_out.at(i).at(1) == map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).at(5) == "t");//have keyset
        BOOST_CHECK(domain_list_out.at(i).at(6) == "admin");//role
        BOOST_CHECK(domain_list_out.at(i).at(7) == test_registrar_handle);//registrar handle
        BOOST_CHECK(domain_list_out.at(i).at(8) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "t");
            if(i > 2) BOOST_CHECK(domain_list_out.at(i).at(9) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "f");
        }
    }
}


BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_nsset, get_my_domains_fixture )
{
    //add user contact as nsset admin
    {
        Fred::OperationContext ctx;
        Fred::UpdateNsset(test_nsset_handle, test_registrar_handle).add_tech_contact(user_contact_info.info_contact_data.handle).exec(ctx);
        ctx.commit_transaction();
    }

    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > domain_list_out;
    bool limit_exceeded = impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(nsset_info.info_nsset_data.id),
            Optional<unsigned long long>(),"CS",0,domain_list_out);

    std::ostringstream list_out;
    list_out << "domain_list_out: \n";

    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < domain_list_out.at(i).size(); ++j)
        {
            list_out << " " <<domain_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);


    BOOST_CHECK(domain_list_out.at(0).at(3) == "deleteCandidate");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(0).at(4)) == (map_at(domain_info,domain_list_out.at(0).at(1)).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména není generována do zóny") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(1).at(3) == "outzone");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(1).at(4)) == (map_at(domain_info,domain_list_out.at(1).at(1)).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je blokována") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(2).at(3) == "expired");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(2).at(4)) == (map_at(domain_info,domain_list_out.at(2).at(1)).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).at(9) == "");
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.id));
        BOOST_CHECK(domain_list_out.at(i).at(1) == map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).at(5) == "t");//have keyset
        BOOST_CHECK(domain_list_out.at(i).at(6) == "holder");//role
        BOOST_CHECK(domain_list_out.at(i).at(7) == test_registrar_handle);//registrar handle
        BOOST_CHECK(domain_list_out.at(i).at(8) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "t");
            if(i > 2) BOOST_CHECK(domain_list_out.at(i).at(9) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "f");
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_my_domain_list_by_keyset, get_my_domains_fixture )
{
    //add user contact as keyset admin
    {
        Fred::OperationContext ctx;
        Fred::UpdateKeyset(test_keyset_handle, test_registrar_handle).add_tech_contact(user_contact_info.info_contact_data.handle).exec(ctx);
        ctx.commit_transaction();
    }

    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > domain_list_out;
    bool limit_exceeded = impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(keyset_info.info_keyset_data.id),"CS",0,domain_list_out);

    std::ostringstream list_out;
    list_out << "domain_list_out: \n";

    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < domain_list_out.at(i).size(); ++j)
        {
            list_out << " " <<domain_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);

    BOOST_CHECK(domain_list_out.at(0).at(3) == "deleteCandidate");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(0).at(4)) == (map_at(domain_info,domain_list_out.at(0).at(1)).info_domain_data.expiration_date + boost::gregorian::days(registration_protection)));

    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(0).at(9).find("Doména není generována do zóny") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(1).at(3) == "outzone");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(1).at(4)) == (map_at(domain_info,domain_list_out.at(1).at(1)).info_domain_data.expiration_date + boost::gregorian::days(outzone_protection)));

    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je po expiraci") != std::string::npos);
    BOOST_CHECK(domain_list_out.at(1).at(9).find("Doména je blokována") != std::string::npos);

    BOOST_CHECK(domain_list_out.at(2).at(3) == "expired");
    BOOST_CHECK(boost::gregorian::from_simple_string(domain_list_out.at(2).at(4)) == (map_at(domain_info,domain_list_out.at(2).at(1)).info_domain_data.expiration_date));
    BOOST_CHECK(domain_list_out.at(2).at(9) == "");
    for(unsigned long long i = 0; i < domain_list_out.size(); ++i)
    {
        BOOST_CHECK(domain_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.id));
        BOOST_CHECK(domain_list_out.at(i).at(1) == map_at(domain_info,domain_list_out.at(i).at(1)).info_domain_data.fqdn);

        BOOST_CHECK(domain_list_out.at(i).at(5) == "t");//have keyset
        BOOST_CHECK(domain_list_out.at(i).at(6) == "holder");//role
        BOOST_CHECK(domain_list_out.at(i).at(7) == test_registrar_handle);//registrar handle
        BOOST_CHECK(domain_list_out.at(i).at(8) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "t");
            if(i > 2) BOOST_CHECK(domain_list_out.at(i).at(9) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(domain_list_out.at(i).at(10));
            BOOST_CHECK(domain_list_out.at(i).at(10) == "f");
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
        Fred::OperationContext ctx;
        std::vector<std::vector<std::string> > domain_list_out;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),Optional<unsigned long long>()
            ,"CS",0,domain_list_out);

        BOOST_ERROR("unreported missing user");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not owned nsset
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_nsset_user_not_nsset_admin, get_my_domains_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        std::vector<std::vector<std::string> > domain_list_out;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(nsset_info.info_nsset_data.id),
            Optional<unsigned long long>(),"CS",0,domain_list_out);

        BOOST_ERROR("unreported missing nsset admin contact");
    }
    catch( const Registry::DomainBrowserImpl::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not owned keyset
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_keyset_user_not_keyset_admin, get_my_domains_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        std::vector<std::vector<std::string> > domain_list_out;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(keyset_info.info_keyset_data.id),"CS",0,domain_list_out);

        BOOST_ERROR("unreported missing keyset admin contact");
    }
    catch( const Registry::DomainBrowserImpl::AccessDenied& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * test getDomainList for not existing contact
 */
BOOST_FIXTURE_TEST_CASE(get_domain_list_for_not_existing_contact, get_my_domains_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        std::vector<std::vector<std::string> > domain_list_out;
        impl.getDomainList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(0),
            Optional<unsigned long long>(),
            Optional<unsigned long long>(),"CS",0,domain_list_out);

        BOOST_ERROR("unreported missing contact");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
    std::map<std::string,Fred::InfoNssetOutput> nsset_info;

    get_my_nssets_fixture()
    : test_nsset_handle(std::string("TEST_NSSET_")+user_contact_handle_fixture::xmark+"_")
    {
        Fred::OperationContext ctx;
        for(int i = 0; i < 10; ++i)
        {
            std::ostringstream nsset_handle;
            nsset_handle << test_nsset_handle << i;

            Fred::CreateNsset(nsset_handle.str(), test_registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
                .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                    (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("127.0.0.3"))
                        (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
                    (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                        (boost::asio::ip::address::from_string("127.0.0.4"))
                        (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

            nsset_info[nsset_handle.str()]= Fred::InfoNssetByHandle(nsset_handle.str()).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

            if(i%2)
            {
                BOOST_MESSAGE(nsset_handle.str() + " blocked");
                Fred::LockObjectStateRequestLock(map_at(nsset_info,nsset_handle.str()).info_nsset_data.id).exec(ctx);
                ctx.get_conn().exec_params(
                    "INSERT INTO object_state_request (object_id, state_id)"
                    " VALUES ($1::integer, (SELECT id FROM enum_object_states"
                    " WHERE name = $2::text)) RETURNING id",
                    Database::query_param_list
                        (map_at(nsset_info,nsset_handle.str()).info_nsset_data.id)
                        (Fred::ObjectState::SERVER_BLOCKED));
                Fred::PerformObjectStateRequest().set_object_id(map_at(nsset_info,nsset_handle.str()).info_nsset_data.id).exec(ctx);
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
    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > nsset_list_out;
    bool limit_exceeded = impl.getNssetList(user_contact_info.info_contact_data.id, Optional<unsigned long long>(),
        "CS",0,nsset_list_out);

    std::ostringstream list_out;
    list_out << "nsset_list_out: \n";

    for(unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < nsset_list_out.at(i).size(); ++j)
        {
            list_out << " " <<nsset_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);

    for(unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        BOOST_CHECK(nsset_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(nsset_info,nsset_list_out.at(i).at(1)).info_nsset_data.id));
        BOOST_CHECK(nsset_list_out.at(i).at(1) == map_at(nsset_info,nsset_list_out.at(i).at(1)).info_nsset_data.handle);

        BOOST_CHECK(nsset_list_out.at(i).at(3) == test_registrar_handle);//registrar handle
        BOOST_CHECK(nsset_list_out.at(i).at(4) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(nsset_list_out.at(i).at(7));
            BOOST_CHECK(nsset_list_out.at(i).at(7) == "t");
            if(i > 2) BOOST_CHECK(nsset_list_out.at(i).at(6) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(nsset_list_out.at(i).at(7));
            BOOST_CHECK(nsset_list_out.at(i).at(7) == "f");
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_nsset_list_by_contact, get_my_nssets_fixture )
{
    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > nsset_list_out;
    bool limit_exceeded = impl.getNssetList(user_contact_info.info_contact_data.id,
        Optional<unsigned long long>(admin_contact_fixture::test_contact_info.info_contact_data.id),
        "CS",0,nsset_list_out);

    std::ostringstream list_out;
    list_out << "nsset_list_out: \n";

    for(unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < nsset_list_out.at(i).size(); ++j)
        {
            list_out << " " <<nsset_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);

    for(unsigned long long i = 0; i < nsset_list_out.size(); ++i)
    {
        BOOST_CHECK(nsset_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(nsset_info,nsset_list_out.at(i).at(1)).info_nsset_data.id));
        BOOST_CHECK(nsset_list_out.at(i).at(1) == map_at(nsset_info,nsset_list_out.at(i).at(1)).info_nsset_data.handle);

        BOOST_CHECK(nsset_list_out.at(i).at(3) == test_registrar_handle);//registrar handle
        BOOST_CHECK(nsset_list_out.at(i).at(4) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(nsset_list_out.at(i).at(7));
            BOOST_CHECK(nsset_list_out.at(i).at(7) == "t");
            if(i > 2) BOOST_CHECK(nsset_list_out.at(i).at(6) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(nsset_list_out.at(i).at(7));
            BOOST_CHECK(nsset_list_out.at(i).at(7) == "f");
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
        Fred::OperationContext ctx;
        std::vector<std::vector<std::string> > nsset_list_out;
        impl.getNssetList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(0),
            "CS",0,nsset_list_out);

        BOOST_ERROR("unreported missing contact");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
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
    std::map<std::string,Fred::InfoKeysetOutput> keyset_info;

    get_my_keysets_fixture()
    : test_keyset_handle(std::string("TEST_KEYSET_")+user_contact_handle_fixture::xmark+"_")
    {
        Fred::OperationContext ctx;
        for(int i = 0; i < 10; ++i)
        {
            std::ostringstream keyset_handle;
            keyset_handle << test_keyset_handle << i;

            Fred::CreateKeyset(keyset_handle.str(), test_registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle)(user_contact_handle))
                .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

            keyset_info[keyset_handle.str()]= Fred::InfoKeysetByHandle(keyset_handle.str()).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);

            if(i%2)
            {
                BOOST_MESSAGE(keyset_handle.str() + " blocked");
                Fred::LockObjectStateRequestLock(map_at(keyset_info,keyset_handle.str()).info_keyset_data.id).exec(ctx);
                ctx.get_conn().exec_params(
                    "INSERT INTO object_state_request (object_id, state_id)"
                    " VALUES ($1::integer, (SELECT id FROM enum_object_states"
                    " WHERE name = $2::text)) RETURNING id",
                    Database::query_param_list
                        (map_at(keyset_info,keyset_handle.str()).info_keyset_data.id)
                        (Fred::ObjectState::SERVER_BLOCKED));
                Fred::PerformObjectStateRequest().set_object_id(map_at(keyset_info,keyset_handle.str()).info_keyset_data.id).exec(ctx);
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
    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > keyset_list_out;
    bool limit_exceeded = impl.getKeysetList(user_contact_info.info_contact_data.id, Optional<unsigned long long>(),
        "CS",0,keyset_list_out);

    std::ostringstream list_out;
    list_out << "keyset_list_out: \n";

    for(unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < keyset_list_out.at(i).size(); ++j)
        {
            list_out << " " <<keyset_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);

    for(unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        BOOST_CHECK(keyset_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(keyset_info,keyset_list_out.at(i).at(1)).info_keyset_data.id));
        BOOST_CHECK(keyset_list_out.at(i).at(1) == map_at(keyset_info,keyset_list_out.at(i).at(1)).info_keyset_data.handle);

        BOOST_CHECK(keyset_list_out.at(i).at(3) == test_registrar_handle);//registrar handle
        BOOST_CHECK(keyset_list_out.at(i).at(4) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(keyset_list_out.at(i).at(7));
            BOOST_CHECK(keyset_list_out.at(i).at(7) == "t");
            BOOST_CHECK(keyset_list_out.at(i).at(6) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(keyset_list_out.at(i).at(7));
            BOOST_CHECK(keyset_list_out.at(i).at(7) == "f");
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_list_by_contact, get_my_keysets_fixture )
{
    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > keyset_list_out;
    bool limit_exceeded = impl.getKeysetList(user_contact_info.info_contact_data.id,
        Optional<unsigned long long>(admin_contact_fixture::test_contact_info.info_contact_data.id),
        "CS",0,keyset_list_out);

    std::ostringstream list_out;
    list_out << "keyset_list_out: \n";

    for(unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < keyset_list_out.at(i).size(); ++j)
        {
            list_out << " " <<keyset_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);

    for(unsigned long long i = 0; i < keyset_list_out.size(); ++i)
    {
        BOOST_CHECK(keyset_list_out.at(i).at(0) == boost::lexical_cast<std::string>(map_at(keyset_info,keyset_list_out.at(i).at(1)).info_keyset_data.id));
        BOOST_CHECK(keyset_list_out.at(i).at(1) == map_at(keyset_info,keyset_list_out.at(i).at(1)).info_keyset_data.handle);

        BOOST_CHECK(keyset_list_out.at(i).at(3) == test_registrar_handle);//registrar handle
        BOOST_CHECK(keyset_list_out.at(i).at(4) == boost::algorithm::replace_first_copy(test_registrar_handle, "-HANDLE", " NAME"));//registrar name

        if(i%2)
        {
            BOOST_MESSAGE(keyset_list_out.at(i).at(7));
            BOOST_CHECK(keyset_list_out.at(i).at(7) == "t");
            BOOST_CHECK(keyset_list_out.at(i).at(6) == "Doména je blokována");
        }
        else
        {
            BOOST_MESSAGE(keyset_list_out.at(i).at(7));
            BOOST_CHECK(keyset_list_out.at(i).at(7) == "f");
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
        Fred::OperationContext ctx;
        std::vector<std::vector<std::string> > keyset_list_out;
        impl.getKeysetList(user_contact_info.info_contact_data.id,
            Optional<unsigned long long>(0),
            "CS",0,keyset_list_out);

        BOOST_ERROR("unreported missing contact");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getKeysetList

BOOST_AUTO_TEST_SUITE(getPublicStatusDesc)

BOOST_FIXTURE_TEST_CASE(get_public_sattus_desc, domain_browser_impl_instance_fixture)
{
    Fred::OperationContext ctx;
    std::vector<std::string> status_desc_out;
    impl.getPublicStatusDesc("CS",status_desc_out);
    for(std::vector<std::string>::const_iterator ci = status_desc_out.begin();
        ci!=status_desc_out.end(); ++ci)
    {
        BOOST_MESSAGE((*ci));
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
    Fred::OperationContext ctx;
    BOOST_CHECK(user_contact_info.info_contact_data.id == impl.getObjectRegistryId(
        "contact",user_contact_info.info_contact_data.handle));
}

BOOST_FIXTURE_TEST_CASE(get_object_id_by_wrong_handle, get_my_contact_object_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        impl.getObjectRegistryId("contact","_WRONG_HANDLE_");

        BOOST_ERROR("unreported wrong handle");
    }
    catch( const Registry::DomainBrowserImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_object_id_by_objtype, get_my_contact_object_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        impl.getObjectRegistryId("wrongtype","_WRONG_HANDLE_");

        BOOST_ERROR("unreported wrong object type");
    }
    catch( const Registry::DomainBrowserImpl::IncorrectUsage& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getObjectRegistryId

struct merge_contacts_fixture
: virtual user_contact_handle_fixture
, virtual test_registrar_fixture
, domain_browser_impl_instance_fixture
{
    Fred::InfoContactOutput user_contact_info;

    std::string test_contact_handle;
    std::map<std::string,Fred::InfoContactOutput> contact_info;
    std::set<unsigned long long> contact_merge_candidates_ids;

    merge_contacts_fixture()
    : test_contact_handle(std::string("TEST_CONTACT_")+user_contact_handle_fixture::xmark+"_")
    {
        { //destination / user contact
            Fred::OperationContext ctx;
            Fred::Contact::PlaceAddress place;
            place.street1 = std::string("STR1") + user_contact_handle_fixture::xmark;
            place.city = "Praha";
            place.postalcode = "11150";
            place.country = "CZ";
            Fred::CreateContact(user_contact_handle,
                CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle)//MojeID registrar
                .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                .set_place(place)
                .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                .exec(ctx);
            user_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
            Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, Util::set_of<std::string>(Fred::ObjectState::MOJEID_CONTACT)).exec(ctx);
            Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
            ctx.commit_transaction();//commit fixture
        }

        //source contacts
        Fred::OperationContext ctx;
        for(int i = 0; i < 26; ++i)
        {
            std::ostringstream contact_handle;
            contact_handle << test_contact_handle << i;
            bool merge_candidate = false;
            Fred::Contact::PlaceAddress place;
            place.street1 = std::string("STR1") + user_contact_handle_fixture::xmark;
            place.city = "Praha";
            place.postalcode = "11150";
            place.country = "CZ";

            switch(i)
            {
                case 0: //the same as dest. - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    Fred::CreateNsset(std::string("TEST-NSSET-HANDLE")+user_contact_handle_fixture::xmark, test_registrar_handle)
                        .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                                (boost::asio::ip::address::from_string("127.0.0.3"))
                                (boost::asio::ip::address::from_string("127.1.1.3")))) //add_dns
                            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<boost::asio::ip::address>
                                (boost::asio::ip::address::from_string("127.0.0.4"))
                                (boost::asio::ip::address::from_string("127.1.1.4")))) //add_dns
                            )
                            .set_tech_contacts(Util::vector_of<std::string>(contact_handle.str()))
                            .exec(ctx);

                    Fred::CreateKeyset(std::string("TEST-KEYSET-HANDLE")+user_contact_handle_fixture::xmark, test_registrar_handle)
                            .set_tech_contacts(Util::vector_of<std::string>(contact_handle.str()))
                            .exec(ctx);

                    Fred::CreateDomain(
                            std::string("testdomainadminowner")+user_contact_handle_fixture::xmark+".cz" //const std::string& fqdn
                            , test_registrar_handle //const std::string& registrar
                            , contact_handle.str() //registrant
                            )
                    .set_admin_contacts(Util::vector_of<std::string>(contact_handle.str()))
                    .exec(ctx);

                    merge_candidate = true;
                    break;
                case 1: //the same as dest. except missing vat - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 2: //the same as dest. except missing ssntype and ssn - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 3: //the same as dest. except missing ssntype, make no sence but - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 4: //the same as dest. except missing ssn, make no sence but - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 5: //the same as dest. except spaces in vat - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat(" CZ1234567890 ").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 6: //the same as dest. except spaces in ssn - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn(" 123456 ")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 7: //the same as dest. except spaces in name - ok
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string(" USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark+" ")
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn(" 123456 ")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 8: //the same as dest. except spaces in city - ok
                    place.city = "  Praha  ";
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);

                    merge_candidate = true;
                    break;
                case 9: //the same as dest. except mojeidContact state - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " mojeidContact");
                        unsigned long long contact_id = Fred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        Fred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(Fred::ObjectState::MOJEID_CONTACT)).exec(ctx);
                        Fred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = false;
                }
                    break;
                case 10: //the same as dest. except serverBlocked state - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " serverBlocked");
                        unsigned long long contact_id = Fred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        Fred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                        Fred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = false;
                }
                    break;
                case 11: //the same as dest. except serverDeleteProhibited state - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " serverDeleteProhibited");
                        unsigned long long contact_id = Fred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        Fred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
                        Fred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = false;
                }
                    break;
                case 12: //the same as dest. except the name - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE DIFFERENTNAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different name");

                    merge_candidate = false;
                }
                    break;
                case 13: //the same as dest. except the organization - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_organization(std::string("USER-CONTACT-HANDLE ORG")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different organization");

                    merge_candidate = false;
                }
                    break;
                case 14: //the same as dest. except the street1 - nok
                {
                    place.street1 = std::string("DIFFERENTSTR1") + user_contact_handle_fixture::xmark;
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different street1");

                    merge_candidate = false;
                }
                    break;
                case 15: //the same as dest. except the street2 - nok
                {
                    place.street2 = std::string("DIFFERENTSTR2") + user_contact_handle_fixture::xmark;
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different street2");

                    merge_candidate = false;
                }
                    break;
                case 16: //the same as dest. except the street3 - nok
                {
                    place.street3 = std::string("DIFFERENTSTR3") + user_contact_handle_fixture::xmark;
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different street3");

                    merge_candidate = false;
                }
                    break;
                case 17: //the same as dest. except the city - nok
                {
                    place.city = "DifferentPraha";
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different city");

                    merge_candidate = false;
                }
                    break;
                case 18: //the same as dest. except the postalcode - nok
                {
                    place.postalcode = "11151";
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different postalcode");

                    merge_candidate = false;
                }
                    break;
                case 19: //the same as dest. except the stateorprovince - nok
                {
                    place.stateorprovince = "different";
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different stateorprovince");

                    merge_candidate = false;
                }
                    break;
                case 20: //the same as dest. except the country - nok
                {
                    place.country = "SK";
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different country");

                    merge_candidate = false;
                }
                    break;
                case 21: //the same as dest. except the email - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place).set_email("test@test.cz")
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different email");

                    merge_candidate = false;
                }
                    break;
                case 22: //the same as dest. except the vat - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("SK1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different vat");

                    merge_candidate = false;
                }
                    break;
                case 23: //the same as dest. except the ssn - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("223456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different ssn");

                    merge_candidate = false;
                }
                    break;
                case 24: //the same as dest. except the ssntype - nok
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("RC").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " different ssntype");

                    merge_candidate = false;
                }
                    break;
                case 25: //the same as dest. canceled not allowed status - ok (#11067)
                {
                    Fred::CreateContact(contact_handle.str(),test_registrar_handle)
                        .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
                        .set_place(place)
                        .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
                        .exec(ctx);
                        BOOST_MESSAGE(contact_handle.str() + " serverBlocked");
                        unsigned long long contact_id = Fred::InfoContactByHandle(contact_handle.str()).exec(ctx).info_contact_data.id;
                        Fred::CreateObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                        Fred::PerformObjectStateRequest(contact_id).exec(ctx);
                        Fred::CancelObjectStateRequestId(contact_id,
                            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
                        Fred::PerformObjectStateRequest(contact_id).exec(ctx);

                    merge_candidate = true;
                }
                    break;
            }

            contact_info[contact_handle.str()] = Fred::InfoContactByHandle(contact_handle.str()).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
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
    Fred::OperationContext ctx;
    std::vector<std::vector<std::string> > contact_list_out;
    bool limit_exceeded = impl.getMergeContactCandidateList(user_contact_info.info_contact_data.id,
        0,contact_list_out);

    std::ostringstream list_out;
    list_out << "contact_list_out: \n";

    for(unsigned long long i = 0; i < contact_list_out.size(); ++i)
    {
        for(unsigned long long j = 0; j < contact_list_out.at(i).size(); ++j)
        {
            list_out << " " <<contact_list_out.at(i).at(j);
        }

        list_out << "\n";
    }
    BOOST_MESSAGE(list_out.str());
    BOOST_MESSAGE("limit_exceeded: " << limit_exceeded);

    BOOST_CHECK(contact_list_out.size() == contact_merge_candidates_ids.size());

    std::set<unsigned long long> contact_list_out_ids;

    for(unsigned long long i = 0; i < contact_list_out.size(); ++i)
    {
        unsigned long long id = map_at(contact_info,contact_list_out.at(i).at(1)).info_contact_data.id;
        BOOST_CHECK(contact_list_out.at(i).at(0) == boost::lexical_cast<std::string>(id));
        BOOST_CHECK(contact_list_out.at(i).at(1) == map_at(contact_info,contact_list_out.at(i).at(1)).info_contact_data.handle);
        contact_list_out_ids.insert(id);

        if(i == 0)
        {
            BOOST_CHECK(contact_list_out.at(i).at(2) == "1");
            BOOST_CHECK(contact_list_out.at(i).at(3) == "1");
            BOOST_CHECK(contact_list_out.at(i).at(4) == "1");
        }
        else
        {
            BOOST_CHECK(contact_list_out.at(i).at(2) == "0");
            BOOST_CHECK(contact_list_out.at(i).at(3) == "0");
            BOOST_CHECK(contact_list_out.at(i).at(4) == "0");
        }

        BOOST_CHECK(contact_list_out.at(i).at(5) == std::string("TEST-REGISTRAR-HANDLE")+test_registrar_fixture::xmark);
        BOOST_CHECK(contact_list_out.at(i).at(6) == std::string("TEST-REGISTRAR NAME")+test_registrar_fixture::xmark);
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
        Fred::OperationContext ctx;
        std::vector<std::vector<std::string> > contact_list_out;
        impl.getMergeContactCandidateList(user_contact_info.info_contact_data.id,
            0,contact_list_out);

        BOOST_ERROR("unreported missing user");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//getMergeContactCandidateList


BOOST_AUTO_TEST_SUITE(mergeContacts)

/**
 * mergeContacts
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts, merge_contacts_fixture )
{
    Fred::OperationContext ctx;

    impl.mergeContacts(user_contact_info.info_contact_data.id,
        Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"0").info_contact_data.id), 0);

    std::vector<Fred::InfoContactOutput> info = Fred::InfoContactHistoryById(
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
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,Util::vector_of<unsigned long long>(0), 0);
        BOOST_ERROR("unreported missing user");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}


struct merge_contacts_no_src_contacts_fixture
: virtual user_contact_handle_fixture
, virtual test_registrar_fixture
, domain_browser_impl_instance_fixture
{
    Fred::InfoContactOutput user_contact_info;

    std::string test_contact_handle;
    std::map<std::string,Fred::InfoContactOutput> contact_info;

    merge_contacts_no_src_contacts_fixture()
    : test_contact_handle(std::string("TEST_CONTACT_")+user_contact_handle_fixture::xmark+"_")
    {
        // user contact
        Fred::OperationContext ctx;
        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + user_contact_handle_fixture::xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(user_contact_handle,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle)//MojeID registrar
            .set_name(std::string("USER-CONTACT-HANDLE NAME")+user_contact_handle_fixture::xmark)
            .set_place(place)
            .set_vat("CZ1234567890").set_ssntype("OP").set_ssn("123456")
            .exec(ctx);
        user_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx, Registry::DomainBrowserImpl::DomainBrowser::output_timezone);
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id, Util::set_of<std::string>(Fred::ObjectState::MOJEID_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();//commit fixture
    }
};

/**
 * mergeContacts no src contacts throws invalid contacts
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_no_src_contacts, merge_contacts_no_src_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,std::vector<unsigned long long>(), 0);
        BOOST_ERROR("unreported missing src contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is the same as dest contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_dst_contacts, merge_contacts_no_src_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,Util::vector_of<unsigned long long>(user_contact_info.info_contact_data.id), 0);
        BOOST_ERROR("unreported the same src and dest. contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is mojeid contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_mojeid_src_contact, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"9").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is blocked contact
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_blocked_src_contact, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"10").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact is delete prohibited
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_delete_prohibited_src_contact, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"11").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in name
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_name, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"12").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in org
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_org, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"13").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in street1
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_street1, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
                Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"14").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in city
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_city, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"17").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in postalcode
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_postalcode, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"18").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in country
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_country, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"20").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in email
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_email, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"21").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in vat
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_vat, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"22").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/**
 * mergeContacts src contact differs in ssn
*/
BOOST_FIXTURE_TEST_CASE(merge_contacts_src_contact_differs_in_ssn, merge_contacts_fixture )
{
    Fred::OperationContext ctx;
    try
    {
        impl.mergeContacts(user_contact_info.info_contact_data.id,
            Util::vector_of<unsigned long long>(map_at(contact_info,test_contact_handle+"23").info_contact_data.id), 0);
        BOOST_ERROR("unreported invalid contacts");
    }
    catch( const Registry::DomainBrowserImpl::InvalidContacts& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END();//mergeContacts

BOOST_AUTO_TEST_SUITE_END();//TestDomainBrowser
