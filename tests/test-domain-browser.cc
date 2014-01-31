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

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/object_state_name.h"
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
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
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

BOOST_AUTO_TEST_SUITE(getRegistrarDetail)

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
    Registry::DomainBrowserImpl::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_handle, test_registrar_handle);

    BOOST_CHECK(rd.id == registrar_info.info_registrar_data.id);
    BOOST_CHECK(rd.handle == registrar_info.info_registrar_data.handle);
    BOOST_CHECK(rd.name == registrar_info.info_registrar_data.name.get_value());
    BOOST_CHECK(rd.phone == registrar_info.info_registrar_data.telephone.get_value());
    BOOST_CHECK(rd.fax == registrar_info.info_registrar_data.fax.get_value());
    BOOST_CHECK(rd.url == registrar_info.info_registrar_data.url.get_value());
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
        Registry::DomainBrowserImpl::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_handle, test_registrar_handle);
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_handle());
        BOOST_CHECK(ex.get_unknown_contact_handle() == user_contact_handle);
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
        Registry::DomainBrowserImpl::RegistrarDetail rd = impl.getRegistrarDetail(user_contact_handle, test_registrar_handle);
        BOOST_ERROR("unreported mojeidContact state");
    }
    catch( const Registry::DomainBrowserImpl::UserNotExists& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_handle());
        BOOST_CHECK(ex.get_unknown_contact_handle() == user_contact_handle);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//getRegistrarDetail

BOOST_AUTO_TEST_SUITE_END();//TestDomainBrowser
