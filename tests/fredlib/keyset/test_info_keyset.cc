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

#include <boost/test/unit_test.hpp>
#include <string>

#include "src/fredlib/opcontext.h"
#include <fredlib/keyset.h>
#include <fredlib/contact.h>
#include "src/fredlib/keyset/info_keyset_impl.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-info-domain";

struct info_keyset_fixture : public Test::Fixture::instantiate_db_template
{
    Fred::OperationContext fixture_ctx;
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact4_handle;
    std::string admin_contact5_handle;
    std::string admin_contact6_handle;
    std::string test_keyset_handle;

    info_keyset_fixture()
    :registrar_handle (static_cast<std::string>(fixture_ctx.get_conn().exec("SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]))
    , xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact4_handle(std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark)
    , admin_contact5_handle(std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark)
    , admin_contact6_handle(std::string("TEST-ADMIN-CONTACT6-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    {
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact4_handle,registrar_handle)
            .set_name(admin_contact4_handle+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(fixture_ctx);
        BOOST_MESSAGE(std::string("admin_contact4_handle: ") + admin_contact4_handle);

        Fred::CreateContact(admin_contact5_handle,registrar_handle)
            .set_name(admin_contact5_handle+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(fixture_ctx);
        BOOST_MESSAGE(std::string("admin_contact5_handle: ") + admin_contact5_handle);

        Fred::CreateContact(admin_contact6_handle,registrar_handle)
            .set_name(admin_contact6_handle+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(fixture_ctx);
        BOOST_MESSAGE(std::string("admin_contact6_handle: ") + admin_contact6_handle);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(fixture_ctx);
        BOOST_MESSAGE(std::string("test_keyset_handle: ") + test_keyset_handle);
        fixture_ctx.commit_transaction();
    }
    ~info_keyset_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestInfoKeyset, info_keyset_fixture)

/**
 * test call InfoKeyset
*/
BOOST_AUTO_TEST_CASE(info_keyset)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoKeysetOutput> keyset_res;

    BOOST_MESSAGE(Fred::InfoKeyset()
                .set_handle(test_keyset_handle)
                .set_history_query(false)
                .explain_analyze(ctx,keyset_res,"Europe/Prague")
                );

    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeysetByHandle(test_keyset_handle).set_lock().exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);

    Fred::InfoKeysetOutput info_data_3 = Fred::InfoKeysetById(info_data_1.info_keyset_data.id).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_3);
    Fred::InfoKeysetOutput info_data_4 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_4);
    Fred::InfoKeysetOutput info_data_5 = Fred::InfoKeysetHistoryById(info_data_1.info_keyset_data.id).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_5);
    Fred::InfoKeysetOutput info_data_6 = Fred::InfoKeysetHistoryByHistoryid(info_data_1.info_keyset_data.historyid).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_6);

    //impl
    for( int j = 0; j < (1 << 7); ++j)
    {
        Fred::InfoKeyset i;
        if(j & (1 << 0)) i.set_handle(info_data_1.info_keyset_data.handle);
        if(j & (1 << 1)) i.set_roid(info_data_1.info_keyset_data.roid);
        if(j & (1 << 2)) i.set_id(info_data_1.info_keyset_data.id);
        if(j & (1 << 3)) i.set_historyid(info_data_1.info_keyset_data.historyid);
        if(j & (1 << 4)) i.set_lock();
        if(j & (1 << 5)) i.set_history_timestamp(info_data_1.info_keyset_data.creation_time);
        if(j & (1 << 6)) i.set_history_query(true);

        std::vector<Fred::InfoKeysetOutput> output;
        BOOST_MESSAGE(i.explain_analyze(ctx,output));
        if((j & (1 << 0)) || (j & (1 << 1)) || (j & (1 << 2)) || (j & (1 << 3)))//check if selective
        {
            if((info_data_1 != output.at(0)))
            {
                BOOST_MESSAGE(Fred::diff_keyset_data(info_data_1.info_keyset_data
                        , output.at(0).info_keyset_data).to_string());
            }
            BOOST_CHECK(output.at(0) == info_data_1);
        }
    }


    ctx.commit_transaction();
}

/**
 * test call InfoKeysetDiff
*/
BOOST_AUTO_TEST_CASE(info_keyset_diff)
{
    Fred::OperationContext ctx;
    Fred::InfoKeysetOutput keyset_info1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    Fred::InfoKeysetOutput keyset_info2 = Fred::InfoKeysetByHandle(test_keyset_handle).set_lock().exec(ctx);

    Fred::InfoKeysetDiff test_diff, test_empty_diff;

    //differing data
    test_diff.crhistoryid = std::make_pair(1ull,2ull);
    test_diff.historyid = std::make_pair(1ull,2ull);
    test_diff.id = std::make_pair(1ull,2ull);
    test_diff.delete_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.handle = std::make_pair(std::string("test1"),std::string("test2"));
    test_diff.roid = std::make_pair(std::string("testroid1"),std::string("testroid2"));
    test_diff.sponsoring_registrar_handle = std::make_pair(std::string("testspreg1"),std::string("testspreg2"));
    test_diff.create_registrar_handle = std::make_pair(std::string("testcrreg1"),std::string("testcrreg2"));
    test_diff.update_registrar_handle = std::make_pair(Nullable<std::string>("testcrreg1"),Nullable<std::string>());
    test_diff.creation_time = std::make_pair(boost::posix_time::ptime(),boost::posix_time::second_clock::local_time());
    test_diff.update_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.transfer_time = std::make_pair(Nullable<boost::posix_time::ptime>()
                ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.authinfopw = std::make_pair(std::string("testpass1"),std::string("testpass2"));

    BOOST_MESSAGE(test_diff.to_string());
    BOOST_MESSAGE(test_empty_diff.to_string());

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_MESSAGE(Fred::diff_keyset_data(keyset_info1.info_keyset_data,keyset_info2.info_keyset_data).to_string());

    //because of changes to Nullable::operator<<
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Database::QPNull))[0][0].isnull());
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Nullable<std::string>()))[0][0].isnull());
}


BOOST_AUTO_TEST_SUITE_END();//TestInfoKeyset
