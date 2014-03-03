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
#include "src/fredlib/registrar.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/delete_domain.h"
#include "src/fredlib/opexception.h"
#include "util/util.h"

#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/object_states.h"
#include "src/contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "src/fredlib/db_settings.h"

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

BOOST_AUTO_TEST_SUITE(TestDeleteDomain)

const std::string server_name = "test-delete-domain";

struct delete_enum_domain_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;

    delete_enum_domain_fixture()
    : xmark(RandomDataGenerator().xnumstring(9))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn ( std::string()+xmark.at(0)+'.'+xmark.at(1)+'.'+xmark.at(2)+'.'
                                    +xmark.at(3)+'.'+xmark.at(4)+'.'+xmark.at(5)+'.'
                                    +xmark.at(6)+'.'+xmark.at(7)+'.'+xmark.at(8)+".0.2.4.e164.arpa")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
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
                test_domain_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(boost::gregorian::day_clock::day_clock::universal_day()+boost::gregorian::days(5))
        .set_enum_publish_flag(false)
        .exec(ctx);
        ctx.commit_transaction();
    }
    ~delete_enum_domain_fixture()
    {}
};

struct delete_domain_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;

    delete_domain_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn ( std::string("fred")+xmark+".cz")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
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
                test_domain_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    ~delete_domain_fixture()
    {}
};

/**
 * test DeleteDomain
 * create test domain, delete test domain, check erdate of test domain is null
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(delete_domain, delete_domain_fixture )
{
    Fred::OperationContext ctx;

    Fred::InfoDomainOutput domain_info1 = Fred::InfoDomainByHandle(test_domain_fqdn).exec(ctx);
    BOOST_CHECK(domain_info1.info_domain_data.delete_time.isnull());

    Fred::DeleteDomainByHandle(test_domain_fqdn).exec(ctx);
    ctx.commit_transaction();

    std::vector<Fred::InfoDomainOutput> domain_history_info1 = Fred::InfoDomainHistory(
    domain_info1.info_domain_data.roid).exec(ctx);

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    Fred::InfoDomainOutput domain_info1_with_change = domain_info1;
    domain_info1_with_change.info_domain_data.delete_time = domain_history_info1.at(0).info_domain_data.delete_time;

    BOOST_CHECK(domain_info1_with_change == domain_history_info1.at(0));

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    BOOST_CHECK(domain_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(domain_history_info1.at(0).history_valid_from <= domain_history_info1.at(0).history_valid_to.get_value());

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_domain_fqdn))[0][0]));

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT o.id FROM object o JOIN object_registry oreg ON o.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT d.id FROM domain d JOIN object_registry oreg ON d.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT dcm.contactid FROM domain_contact_map dcm JOIN object_registry oreg ON dcm.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT ev.domainid FROM enumval ev JOIN object_registry oreg ON ev.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

}//delete_domain

/**
 * test DeleteDomain
 * create test ENUM domain, delete test ENUM domain, check erdate of test ENUM domain is null
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(delete_enum_domain, delete_enum_domain_fixture )
{
    Fred::OperationContext ctx;

    Fred::InfoDomainOutput domain_info1 = Fred::InfoDomainByHandle(test_domain_fqdn).exec(ctx);
    BOOST_CHECK(domain_info1.info_domain_data.delete_time.isnull());

    Fred::DeleteDomainByHandle(test_domain_fqdn).exec(ctx);
    ctx.commit_transaction();

    std::vector<Fred::InfoDomainOutput> domain_history_info1 = Fred::InfoDomainHistory(
    domain_info1.info_domain_data.roid).exec(ctx);

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    Fred::InfoDomainOutput domain_info1_with_change = domain_info1;
    domain_info1_with_change.info_domain_data.delete_time = domain_history_info1.at(0).info_domain_data.delete_time;

    BOOST_CHECK(domain_info1_with_change == domain_history_info1.at(0));

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    BOOST_CHECK(domain_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(domain_history_info1.at(0).history_valid_from <= domain_history_info1.at(0).history_valid_to.get_value());

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_domain_fqdn))[0][0]));

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT o.id FROM object o JOIN object_registry oreg ON o.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT d.id FROM domain d JOIN object_registry oreg ON d.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT dcm.contactid FROM domain_contact_map dcm JOIN object_registry oreg ON dcm.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT ev.domainid FROM enumval ev JOIN object_registry oreg ON ev.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

}//delete_domain


/**
 * test DeleteDomain with wrong fqdn
 */

BOOST_FIXTURE_TEST_CASE(delete_domain_with_wrong_fqdn, delete_domain_fixture )
{
    std::string bad_test_domain_fqdn = std::string("bad")+test_domain_fqdn;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteDomainByHandle(bad_test_domain_fqdn).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteDomainByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_domain_fqdn());
        BOOST_CHECK(ex.get_unknown_domain_fqdn().compare(bad_test_domain_fqdn) == 0);
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestDeleteDomain
