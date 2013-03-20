/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"
#include "fredlib/opexception.h"
#include "util/util.h"

#include "fredlib/contact_verification/contact.h"
#include "fredlib/object_states.h"
#include "contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestUpdateDomain)

const std::string server_name = "test-update-domain";

/**
 * test UpdateDomainException
 * test create and throw exception with special data
 */
BOOST_AUTO_TEST_CASE(update_domain_exception)
{
    //no parsable data - ok
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, "test")
    , Fred::OperationExceptionBase);

    //no data - error
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, "test ||  |")
    , Fred::OperationErrorBase);

    //error exception - ok
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException::OperationErrorType("test")
    , Fred::OperationErrorBase);

    //not ended by | - no parsable data - ok
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, "test exception || not found:1111: errtest")
        , Fred::OperationExceptionBase);
}

/**
 * test UpdateDomain
 * test UpdateDomain construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(update_domain)
{
    Fred::OperationContext ctx;

    std::string registrar_handle = static_cast<std::string>(
            ctx.get_conn().exec("SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
    BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar


    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string admin_contact_handle = std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark;
    Fred::CreateContact(admin_contact_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact1_handle = std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark;
    Fred::CreateContact(admin_contact1_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact2_handle = std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark;
    Fred::CreateContact(admin_contact2_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);


    std::string registrant_contact_handle = std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark;
    Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

    std::string test_domain_handle = std::string("fred")+xmark+".cz";
    Fred::CreateDomain(
            test_domain_handle //const std::string& fqdn
            , registrar_handle //const std::string& registrar
            , registrant_contact_handle //registrant
            )
    .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
    .exec(ctx);

    //call update using big ctor
    Fred::UpdateDomain(test_domain_handle//fqdn
            , registrar_handle//registrar
            , registrant_contact_handle //registrant - owner
            , std::string("testauthinfo1") //authinfo
            , Nullable<std::string>()//unset nsset - set to null
            , Optional<Nullable<std::string> >()//dont change keyset
            , Util::vector_of<std::string> (admin_contact1_handle)(registrant_contact_handle) //add admin contacts
            , Util::vector_of<std::string> (admin_contact2_handle) //remove admin contacts
            , Optional<unsigned long long>() //request_id not set
            ).exec(ctx);

    //call update using small ctor and set custom params
    Fred::UpdateDomain(test_domain_handle, registrar_handle)
    .set_authinfo("testauthinfo")
    .set_registrant(registrant_contact_handle)
    .add_admin_contact(admin_contact_handle)
    .rem_admin_contact(registrant_contact_handle)
    .rem_admin_contact(admin_contact1_handle)
    .exec(ctx);


    std::string test_nsset_handle = std::string("TEST-D-NSSET-HANDLE")+xmark;
    Fred::CreateNsset(test_nsset_handle, registrar_handle)
        .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
            )
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact2_handle))
            .exec(ctx);

    std::string test_keyset_handle = std::string("TEST-D-KEYSET-HANDLE")+xmark;
    Fred::CreateKeyset(test_keyset_handle, registrar_handle)
            //.set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
            .exec(ctx);

    //call update using small ctor and set one custom param
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_authinfo("testauthinfo").exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_registrant(registrant_contact_handle).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).add_admin_contact(admin_contact1_handle).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).rem_admin_contact(admin_contact_handle).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_nsset(test_nsset_handle).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_nsset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_nsset(test_nsset_handle).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).unset_nsset().exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_nsset(Nullable<std::string>(test_nsset_handle)).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_nsset(test_nsset_handle).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_keyset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_keyset(test_keyset_handle).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).unset_keyset().exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_keyset(Nullable<std::string>(test_keyset_handle)).exec(ctx);
    Fred::UpdateDomain(test_domain_handle, registrar_handle).set_logd_request_id(0u).exec(ctx);

    //commit db transaction
    ctx.commit_transaction();

    //TODO check result of updates
    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "SELECT o.authinfopw = $1::text "
        //" AND "
        " FROM object_registry oreg "
        " JOIN object o ON o.id = oreg.id "
        " WHERE oreg.name = $2::text"
        ,Database::query_param_list("testauthinfo")(test_domain_handle))[0][0]));
}//update_domain


struct update_domain_fixture
{
    Fred::OperationContext ctx;
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_handle;

    update_domain_fixture()
    :registrar_handle (static_cast<std::string>(ctx.get_conn().exec("SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]))
    , xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_handle ( std::string("fred")+xmark+".cz")
    {
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(registrant_contact_handle,registrar_handle)
                .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
                .set_disclosename(true)
                .set_street1(std::string("STR1")+xmark)
                .set_city("Praha").set_postalcode("11150").set_country("CZ")
                .set_discloseaddress(true)
                .exec(ctx);

        Fred::CreateDomain(
                test_domain_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~update_domain_fixture()
    {}
};

/**
 * test UpdateDomain with wrong fqdn
 */

BOOST_FIXTURE_TEST_CASE(update_domain_wrong_fqdn, update_domain_fixture )
{

    std::string bad_test_domain_handle = std::string("bad")+xmark+".cz";
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(bad_test_domain_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        Fred::GetOperationExceptionParamsDataToMmapCallback cb;
        ex.callback_exception_params(boost::ref(cb));
        BOOST_CHECK((cb.get().size()) == 1);
        BOOST_CHECK(boost::algorithm::trim_copy(cb.get().find("not found:fqdn")->second).compare(bad_test_domain_handle) == 0);
    }
}


/**
 * test UpdateDomain with wrong registrar
 */
BOOST_FIXTURE_TEST_CASE(update_domain_wrong_registrar, update_domain_fixture)
{
    std::string bad_registrar_handle = registrar_handle+xmark;

    Fred::InfoDomainData info_data_1 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(test_domain_handle, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        Fred::GetOperationExceptionParamsDataToMmapCallback cb;
        ex.callback_exception_params(boost::ref(cb));
        BOOST_CHECK((cb.get().size()) == 1);
        BOOST_CHECK(boost::algorithm::trim_copy(cb.get().find("not found:registrar")->second).compare(bad_registrar_handle) == 0);
    }

    Fred::InfoDomainData info_data_2 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.delete_time.isnull());

}

/**
 * test UpdateDomain with wrong registrant
 */
BOOST_FIXTURE_TEST_CASE(update_domain_wrong_registrant, update_domain_fixture)
{
    std::string bad_registrant_handle = registrant_contact_handle+xmark;

    Fred::InfoDomainData info_data_1 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(test_domain_handle, registrar_handle)
        .set_registrant(bad_registrant_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        Fred::GetOperationExceptionParamsDataToMmapCallback cb;
        ex.callback_exception_params(boost::ref(cb));
        BOOST_CHECK((cb.get().size()) == 1);
        BOOST_CHECK(boost::algorithm::trim_copy(cb.get().find("not found:registrant")->second).compare(bad_registrant_handle) == 0);
    }

    Fred::InfoDomainData info_data_2 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.delete_time.isnull());

}

/**
 * test UpdateDomain add non-existing admin
 */
BOOST_FIXTURE_TEST_CASE(update_domain_add_wrong_admin, update_domain_fixture)
{
    std::string bad_admin_contact_handle = admin_contact2_handle+xmark;

    Fred::InfoDomainData info_data_1 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(test_domain_handle, registrar_handle)
        .add_admin_contact(bad_admin_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        Fred::GetOperationExceptionParamsDataToMmapCallback cb;
        ex.callback_exception_params(boost::ref(cb));
        BOOST_CHECK((cb.get().size()) == 1);
        BOOST_CHECK(boost::algorithm::trim_copy(cb.get().find("not found:admin contact")->second).compare(bad_admin_contact_handle) == 0);
    }

    Fred::InfoDomainData info_data_2 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.delete_time.isnull());

}

/**
 * test UpdateDomain add already added admin
 */
BOOST_FIXTURE_TEST_CASE(update_domain_add_already_added_admin, update_domain_fixture)
{
    Fred::InfoDomainData info_data_1 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(test_domain_handle, registrar_handle)
        .add_admin_contact(admin_contact2_handle)
        .add_admin_contact(admin_contact2_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        Fred::GetOperationExceptionParamsDataToMmapCallback cb;
        ex.callback_exception_params(boost::ref(cb));
        BOOST_CHECK((cb.get().size()) == 1);
        BOOST_CHECK(boost::algorithm::trim_copy(cb.get().find("already set:admin contact")->second).compare(admin_contact2_handle) == 0);
    }

    Fred::InfoDomainData info_data_2 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.delete_time.isnull());

}

/**
 * test UpdateDomain remove non-existing admin
 */
BOOST_FIXTURE_TEST_CASE(update_domain_rem_wrong_admin, update_domain_fixture)
{
    std::string bad_admin_contact_handle = admin_contact2_handle+xmark;

    Fred::InfoDomainData info_data_1 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(test_domain_handle, registrar_handle)
        .rem_admin_contact(bad_admin_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        Fred::GetOperationExceptionParamsDataToMmapCallback cb;
        ex.callback_exception_params(boost::ref(cb));
        BOOST_CHECK((cb.get().size()) == 1);
        BOOST_CHECK(boost::algorithm::trim_copy(cb.get().find("not found:admin contact")->second).compare(bad_admin_contact_handle) == 0);
    }

    Fred::InfoDomainData info_data_2 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.delete_time.isnull());

}

/**
 * test UpdateDomain remove existing unassigned admin
 */
BOOST_FIXTURE_TEST_CASE(update_domain_rem_unassigned_admin, update_domain_fixture)
{
    std::string bad_admin_contact_handle = registrant_contact_handle;

    Fred::InfoDomainData info_data_1 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(test_domain_handle, registrar_handle)
        .rem_admin_contact(bad_admin_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        Fred::GetOperationExceptionParamsDataToMmapCallback cb;
        ex.callback_exception_params(boost::ref(cb));
        BOOST_CHECK((cb.get().size()) == 1);
        BOOST_CHECK(boost::algorithm::trim_copy(cb.get().find("invalid:admin contact")->second).compare(bad_admin_contact_handle) == 0);
    }

    Fred::InfoDomainData info_data_2 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.delete_time.isnull());

}

/**
 * test call InfoDomainHistory
 */
BOOST_FIXTURE_TEST_CASE(info_domain_history_test_call, update_domain_fixture)
{
    Fred::InfoDomainData info_data_1 = Fred::InfoDomain(test_domain_handle, registrar_handle).exec(ctx);
    //call update
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateDomain(test_domain_handle, registrar_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }


    std::vector<Fred::InfoDomainHistoryData> history_info_data_1 = Fred::InfoDomainHistory(info_data_1.roid, registrar_handle).exec(ctx);
}


BOOST_AUTO_TEST_SUITE_END();//TestUpdateDomain
