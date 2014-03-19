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

#include <boost/lexical_cast.hpp>

#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar_diff.h"
#include "src/fredlib/registrar/info_registrar_impl.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"

#include "random_data_generator.h"

#include "cfg/handle_mojeid_args.h"

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

        Fred::CreateContact(user_contact_handle,
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle)//MojeID registrar
            .set_name(std::string("USER-CONTACT-HANDLE NAME")+xmark)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .exec(ctx);

        user_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx);

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
    admin_contact_fixture()
    : test_contact_handle(std::string("TEST-ADMIN-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        Fred::CreateContact(test_contact_handle,test_registrar_handle).set_organization(std::string("TEST-ORGANIZATION")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);
        ctx.commit_transaction();//commit fixture
    }
    ~admin_contact_fixture()
    {}
};

struct nsset_fixture
: virtual admin_contact_fixture
{
    std::string test_nsset_handle;
    nsset_fixture()
    : test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        Fred::CreateNsset(test_nsset_handle, test_registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
            ).exec(ctx);

        Fred::InfoNssetOutput nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

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
    std::string test_keyset_handle;
    keyset_fixture()
    : test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        Fred::CreateKeyset(test_keyset_handle, test_registrar_handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact_fixture::test_contact_handle))
        .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

        Fred::InfoKeysetOutput keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);

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
{};

/**
 * test call getRegistrarDetail
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail, get_registrar_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);
    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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
{};

/**
 * test getRegistrarDetail no contact
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_no_user, get_registrar_detail_no_user_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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
{};

/**
 * test getRegistrarDetail not mojeid contact
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_not_mojeid_user, get_registrar_detail_not_mojeid_user_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoRegistrarOutput registrar_info = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test getRegistrarDetail no registrar
*/
BOOST_FIXTURE_TEST_CASE(get_registrar_detail_no_registrar, mojeid_user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

        Fred::CreateContact(test_contact_handle,test_registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_fixture()
    {}
};

/**
 * test call getContactDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_my_contact_detail, mojeid_user_contact_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(my_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx);

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
    Registry::DomainBrowserImpl::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
            my_contact_info.info_contact_data.id, "CS");

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
    BOOST_CHECK(cd.street1.get_value_or_default() == my_contact_info.info_contact_data.street1.get_value_or_default());
    BOOST_CHECK(cd.street2.get_value_or_default() == my_contact_info.info_contact_data.street2.get_value_or_default());
    BOOST_CHECK(cd.street3.get_value_or_default() == my_contact_info.info_contact_data.street3.get_value_or_default());
    BOOST_CHECK(cd.city.get_value_or_default() == my_contact_info.info_contact_data.city.get_value_or_default());
    BOOST_CHECK(cd.stateorprovince.get_value_or_default() == my_contact_info.info_contact_data.stateorprovince.get_value_or_default());
    BOOST_CHECK(cd.postalcode.get_value_or_default() == my_contact_info.info_contact_data.postalcode.get_value_or_default());
    BOOST_CHECK(cd.country.get_value_or_default() == my_contact_info.info_contact_data.country.get_value_or_default());
    BOOST_CHECK(cd.telephone.get_value_or_default() == my_contact_info.info_contact_data.telephone.get_value_or_default());
    BOOST_CHECK(cd.fax.get_value_or_default() == my_contact_info.info_contact_data.fax.get_value_or_default());
    BOOST_CHECK(cd.email.get_value_or_default() == my_contact_info.info_contact_data.email.get_value_or_default());
    BOOST_CHECK(cd.notifyemail.get_value_or_default() == my_contact_info.info_contact_data.notifyemail.get_value_or_default());
    BOOST_CHECK(cd.vat.get_value_or_default() == my_contact_info.info_contact_data.vat.get_value_or_default());
    BOOST_CHECK(cd.ssntype.get_value_or_default() == my_contact_info.info_contact_data.ssntype.get_value_or_default());
    BOOST_CHECK(cd.ssn.get_value_or_default() == my_contact_info.info_contact_data.ssn.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.name == my_contact_info.info_contact_data.disclosename.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.organization == my_contact_info.info_contact_data.discloseorganization.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.email == my_contact_info.info_contact_data.discloseemail.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.address == my_contact_info.info_contact_data.discloseaddress.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.telephone == my_contact_info.info_contact_data.disclosetelephone.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.fax == my_contact_info.info_contact_data.disclosefax.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.ident == my_contact_info.info_contact_data.discloseident.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.vat == my_contact_info.info_contact_data.disclosevat.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.notify_email == my_contact_info.info_contact_data.disclosenotifyemail.get_value_or_default());
    BOOST_CHECK(cd.states.find_first_of("MojeID contact") != std::string::npos);
    BOOST_CHECK(cd.state_codes.find_first_of("mojeidContact") != std::string::npos);
    BOOST_CHECK(cd.is_owner == true);

    BOOST_MESSAGE(cd.states);
    BOOST_MESSAGE(cd.state_codes);
}

struct get_contact_fixture
: mojeid_user_contact_fixture
, test_contact_fixture
{};


/**
 * test call getContactDetail with public data
*/
BOOST_FIXTURE_TEST_CASE(get_contact_detail, get_contact_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput test_contact_info = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(test_contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx);

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
    Registry::DomainBrowserImpl::ContactDetail cd = impl.getContactDetail(user_contact_info.info_contact_data.id,
            test_contact_info.info_contact_data.id, "CS");

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
    BOOST_CHECK(cd.street1.get_value_or_default() == test_contact_info.info_contact_data.street1.get_value_or_default());
    BOOST_CHECK(cd.street2.get_value_or_default() == test_contact_info.info_contact_data.street2.get_value_or_default());
    BOOST_CHECK(cd.street3.get_value_or_default() == test_contact_info.info_contact_data.street3.get_value_or_default());
    BOOST_CHECK(cd.city.get_value_or_default() == test_contact_info.info_contact_data.city.get_value_or_default());
    BOOST_CHECK(cd.stateorprovince.get_value_or_default() == test_contact_info.info_contact_data.stateorprovince.get_value_or_default());
    BOOST_CHECK(cd.postalcode.get_value_or_default() == test_contact_info.info_contact_data.postalcode.get_value_or_default());
    BOOST_CHECK(cd.country.get_value_or_default() == test_contact_info.info_contact_data.country.get_value_or_default());
    BOOST_CHECK(cd.telephone.get_value_or_default() == test_contact_info.info_contact_data.telephone.get_value_or_default());
    BOOST_CHECK(cd.fax.get_value_or_default() == test_contact_info.info_contact_data.fax.get_value_or_default());
    BOOST_CHECK(cd.email.get_value_or_default() == test_contact_info.info_contact_data.email.get_value_or_default());
    BOOST_CHECK(cd.notifyemail.get_value_or_default() == test_contact_info.info_contact_data.notifyemail.get_value_or_default());
    BOOST_CHECK(cd.vat.get_value_or_default() == test_contact_info.info_contact_data.vat.get_value_or_default());
    BOOST_CHECK(cd.ssntype.get_value_or_default() == test_contact_info.info_contact_data.ssntype.get_value_or_default());
    BOOST_CHECK(cd.ssn.get_value_or_default() == test_contact_info.info_contact_data.ssn.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.name == test_contact_info.info_contact_data.disclosename.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.organization == test_contact_info.info_contact_data.discloseorganization.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.email == test_contact_info.info_contact_data.discloseemail.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.address == test_contact_info.info_contact_data.discloseaddress.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.telephone == test_contact_info.info_contact_data.disclosetelephone.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.fax == test_contact_info.info_contact_data.disclosefax.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.ident == test_contact_info.info_contact_data.discloseident.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.vat == test_contact_info.info_contact_data.disclosevat.get_value_or_default());
    BOOST_CHECK(cd.disclose_flags.notify_email == test_contact_info.info_contact_data.disclosenotifyemail.get_value_or_default());
    BOOST_CHECK(cd.states.find_first_of("MojeID contact") == std::string::npos);
    BOOST_CHECK(cd.state_codes.find_first_of("mojeidContact") == std::string::npos);
    BOOST_CHECK(cd.is_owner == false);

    BOOST_MESSAGE(cd.states);
    BOOST_MESSAGE(cd.state_codes);
}

/**
 * test getContactDetail no user contact
*/
BOOST_FIXTURE_TEST_CASE(get_contact_detail_no_user, test_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoContactOutput test_contact_info = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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
{};

/**
 * test getContactDetail not mojeid user contact
 */
BOOST_FIXTURE_TEST_CASE(get_contact_detail_not_mojeid_user, get_contact_detail_not_mojeid_user_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Fred::InfoContactOutput test_contact_info = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test getContactDetail no contact
 */
BOOST_FIXTURE_TEST_CASE(get_contact_detail_no_test_contact, mojeid_user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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
        Fred::CreateContact(test_contact_handle,test_registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
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


        Fred::InfoDomainOutput domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx);

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
    Fred::InfoDomainOutput my_domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(my_domain_info.info_domain_data.sponsoring_registrar_handle).exec(ctx);

    Fred::InfoNssetOutput nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    Fred::InfoKeysetOutput keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);

    Fred::InfoContactOutput admin_contact_info = Fred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx);

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test getDomainDetail no domain
 */
BOOST_FIXTURE_TEST_CASE(get_domain_detail_no_domain, mojeid_user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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
{};

/**
 * test call getNssetDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_nsset_detail, get_nsset_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOutput nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(nsset_info.info_nsset_data.sponsoring_registrar_handle).exec(ctx);
    Fred::InfoContactOutput admin_contact_info = Fred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx);

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test getNssetDetail no nsset
 */
BOOST_FIXTURE_TEST_CASE(get_nsset_detail_no_nsset, mojeid_user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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
{};

/**
 * test call getKeysetDetail with private data
*/
BOOST_FIXTURE_TEST_CASE(get_keyset_detail, get_keyset_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoKeysetOutput keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoRegistrarOutput sponsoring_registrar_info = Fred::InfoRegistrarByHandle(keyset_info.info_keyset_data.sponsoring_registrar_handle).exec(ctx);
    Fred::InfoContactOutput admin_contact_info = Fred::InfoContactByHandle(admin_contact_fixture::test_contact_handle).exec(ctx);

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test getKeysetDetail no keyset
 */
BOOST_FIXTURE_TEST_CASE(get_keyset_detail_no_keyset, mojeid_user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test call setContactDiscloseFlags with private data
*/
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags, mojeid_user_contact_fixture )
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

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
    Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx);
    BOOST_CHECK(!my_contact_info.info_contact_data.disclosename.get_value_or_default());
    BOOST_CHECK(!my_contact_info.info_contact_data.discloseorganization.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.discloseemail.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.discloseaddress.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosetelephone.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosefax.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.discloseident.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosevat.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosenotifyemail.get_value_or_default());
}

/**
 * test call setContactDiscloseFlags with private data and validated contact
*/
BOOST_FIXTURE_TEST_CASE(set_validated_contact_disclose_flags, mojeid_user_contact_fixture )
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

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
    Registry::DomainBrowserImpl::ContactDiscloseFlagsToSet set_flags;
    set_flags.address = true;
    set_flags.email = true;
    set_flags.fax = true;
    set_flags.ident = true;
    set_flags.notify_email = true;
    set_flags.telephone = true;
    set_flags.vat = true;
    impl.setContactDiscloseFlags(user_contact_info.info_contact_data.id,set_flags, 0);

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx);
    BOOST_CHECK(!my_contact_info.info_contact_data.disclosename.get_value_or_default());
    BOOST_CHECK(!my_contact_info.info_contact_data.discloseorganization.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.discloseemail.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.discloseaddress.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosetelephone.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosefax.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.discloseident.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosevat.get_value_or_default());
    BOOST_CHECK(my_contact_info.info_contact_data.disclosenotifyemail.get_value_or_default());
}


/**
 * test setContactDiscloseFlags non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_user_not_in_mojeid, user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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


/**
 * test setContactDiscloseFlags non-identified user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_user_not_identified, mojeid_user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setContactDiscloseFlags blocked user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_contact_blocked, mojeid_user_contact_fixture )
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setContactDiscloseFlags hide address of organization
 */
BOOST_FIXTURE_TEST_CASE(set_contact_disclose_flags_hide_organization_address, mojeid_user_contact_fixture )
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test call setContactAuthInfo with private data
*/
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo, mojeid_user_contact_fixture )
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

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);

    BOOST_CHECK(impl.setContactAuthInfo(
        user_contact_info.info_contact_data.id,
        user_contact_info.info_contact_data.id,"newauthinfo", 0));

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx);
    BOOST_CHECK(my_contact_info.info_contact_data.authinfopw.compare("newauthinfo")==0);
}

/**
 * test call setContactAuthInfo with private data and validated contact
*/
BOOST_FIXTURE_TEST_CASE(set_validated_contact_authinfo, mojeid_user_contact_fixture )
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

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);

    BOOST_CHECK(impl.setContactAuthInfo(
        user_contact_info.info_contact_data.id,
        user_contact_info.info_contact_data.id,"newauthinfo", 0));

    Fred::InfoContactOutput my_contact_info = Fred::InfoContactByHandle(user_contact_handle).exec(ctx);
    BOOST_CHECK(my_contact_info.info_contact_data.authinfopw.compare("newauthinfo")==0);
}

/**
 * test setContactAuthInfo non-mojeid user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_user_not_in_mojeid, user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);

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


/**
 * test setContactAuthInfo non-identified user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_user_not_identified, mojeid_user_contact_fixture )
{
    try
    {
        Fred::OperationContext ctx;
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);

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

/**
 * test setContactAuthInfo blocked user
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_contact_blocked, mojeid_user_contact_fixture )
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setContactAuthInfo the same authinfo
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_the_same, mojeid_user_contact_fixture )
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
        BOOST_CHECK(impl.setContactAuthInfo(
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.id,
            user_contact_info.info_contact_data.authinfopw, 0) == false);
    }
}

/**
 * test setContactAuthInfo not owner
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_not_owner, mojeid_user_contact_fixture )
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setContactAuthInfo too long
 */
BOOST_FIXTURE_TEST_CASE(set_contact_authinfo_too_long, mojeid_user_contact_fixture )
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

        domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx);
        ctx.commit_transaction();//commit fixture
    }

    ~registrant_domain_fixture()
    {}
};

/**
 * test setObjectBlockStatus - domain with registrant, blocking transfer and update
 */

BOOST_FIXTURE_TEST_CASE(set_registrant_domain_object_block_status, registrant_domain_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setObjectBlockStatus - domain with registrant, blocking transfer
 */

BOOST_FIXTURE_TEST_CASE(set_registrant_domain_object_block_status_transfer, registrant_domain_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

        domain_info = Fred::InfoDomainByHandle(test_fqdn).exec(ctx);
/*
        Fred::CreateObjectStateRequestId(domain_info.info_domain_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_BLOCKED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(domain_info.info_domain_data.id).exec(ctx);
*/
        ctx.commit_transaction();//commit fixture
    }

    ~admin_domain_fixture()
    {}
};

/**
 * test setObjectBlockStatus - domain with admin, blocking transfer and update
 */

BOOST_FIXTURE_TEST_CASE(set_admin_domain_object_block_status, admin_domain_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setObjectBlockStatus - domain with admin, blocking transfer
 */

BOOST_FIXTURE_TEST_CASE(set_admin_domain_object_block_status_transfer, admin_domain_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
            ).exec(ctx);

        nsset_info = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);

        Fred::CreateObjectStateRequestId(nsset_info.info_nsset_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(nsset_info.info_nsset_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }

    ~admin_nsset_fixture()
    {}
};

/**
 * test setObjectBlockStatus - nsset with admin, blocking transfer and update
 */

BOOST_FIXTURE_TEST_CASE(set_admin_nsset_object_block_status, admin_nsset_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setObjectBlockStatus - nsset with admin, blocking transfer
 */

BOOST_FIXTURE_TEST_CASE(set_admin_nsset_object_block_status_transfer, admin_nsset_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

        keyset_info = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);

        Fred::CreateObjectStateRequestId(keyset_info.info_keyset_data.id,
            Util::set_of<std::string>(Fred::ObjectState::SERVER_DELETE_PROHIBITED)).exec(ctx);
        Fred::PerformObjectStateRequest().set_object_id(keyset_info.info_keyset_data.id).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }

    ~admin_keyset_fixture()
    {}
};

/**
 * test setObjectBlockStatus - keyset with admin, blocking transfer and update
 */

BOOST_FIXTURE_TEST_CASE(set_admin_keyset_object_block_status, admin_keyset_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setObjectBlockStatus - keyset with admin, blocking transfer
 */

BOOST_FIXTURE_TEST_CASE(set_admin_keyset_object_block_status_transfer, admin_keyset_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateObjectStateRequestId(user_contact_info.info_contact_data.id
        , Util::set_of<std::string>(Fred::ObjectState::VALIDATED_CONTACT)).exec(ctx);
        Fred::PerformObjectStateRequest(user_contact_info.info_contact_data.id).exec(ctx);
        ctx.commit_transaction();
    }

    Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setObjectBlockStatus - wrong object type contact
 */
BOOST_FIXTURE_TEST_CASE(set_contact_object_block_status, mojeid_user_contact_fixture)
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setObjectBlockStatus - missing user validation
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_missing_user_validation, admin_keyset_fixture)
{
    try
    {
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

/**
 * test setObjectBlockStatus - wrong object type
 */
BOOST_FIXTURE_TEST_CASE(set_object_block_status_wrong_object_type, admin_keyset_fixture)
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
        Registry::DomainBrowserImpl::DomainBrowser impl(server_name);
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

BOOST_AUTO_TEST_SUITE_END();//setObjectBlockStatus

BOOST_AUTO_TEST_SUITE_END();//TestDomainBrowser
