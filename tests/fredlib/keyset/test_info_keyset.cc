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


BOOST_AUTO_TEST_SUITE(TestInfoKeyset);

struct info_keyset_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact4_handle;
    std::string admin_contact5_handle;
    std::string admin_contact6_handle;
    std::string test_keyset_handle;
    std::string test_keyset_history_handle;

    Fred::InfoKeysetOutput test_info_keyset_output;

    info_keyset_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact4_handle(std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark)
    , admin_contact5_handle(std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark)
    , admin_contact6_handle(std::string("TEST-ADMIN-CONTACT6-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    , test_keyset_history_handle(std::string("TEST-KEYSET-HISTORY-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);

        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(admin_contact4_handle,registrar_handle)
            .set_name(admin_contact4_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_MESSAGE(std::string("admin_contact4_handle: ") + admin_contact4_handle);

        Fred::CreateContact(admin_contact5_handle,registrar_handle)
            .set_name(admin_contact5_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_MESSAGE(std::string("admin_contact5_handle: ") + admin_contact5_handle);

        Fred::CreateContact(admin_contact6_handle,registrar_handle)
            .set_name(admin_contact6_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_MESSAGE(std::string("admin_contact6_handle: ") + admin_contact6_handle);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_authinfo("testauthinfo1")
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);
        BOOST_MESSAGE(std::string("test_keyset_handle: ") + test_keyset_handle);

        Fred::CreateKeyset(test_keyset_history_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);
        BOOST_MESSAGE(std::string("test_keyset_handle: ") + test_keyset_handle);

        //id query
        Database::Result id_res = ctx.get_conn().exec_params("SELECT"
        " (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND name = UPPER($1::text)) AS test_keyset_id"
        ",  (SELECT roid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND name = UPPER($1::text)) AS test_keyset_roid"
        ",  (SELECT crhistoryid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND name = UPPER($1::text)) AS test_keyset_crhistoryid"
        ",  (SELECT historyid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND name = UPPER($1::text)) AS test_keyset_historyid"
        ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) AND name = UPPER($2::text)) AS admin_contact6_handle_id"
        ", (SELECT id FROM registrar WHERE handle = UPPER($3::text)) AS registrar_handle_id"
        , Database::query_param_list(test_keyset_handle)(admin_contact6_handle)(registrar_handle));

        //crdate fix
        ctx.get_conn().exec_params("UPDATE object_registry SET crdate = $1::timestamp WHERE id = $2::bigint",
            Database::query_param_list("2011-06-30T23:59:59")(static_cast<unsigned long long>(id_res[0]["test_keyset_id"])));

        ctx.commit_transaction();

        test_info_keyset_output.info_keyset_data.id = static_cast<unsigned long long>(id_res[0]["test_keyset_id"]);
        test_info_keyset_output.info_keyset_data.roid = static_cast<std::string>(id_res[0]["test_keyset_roid"]);
        test_info_keyset_output.info_keyset_data.crhistoryid = static_cast<unsigned long long>(id_res[0]["test_keyset_crhistoryid"]);
        test_info_keyset_output.info_keyset_data.historyid = static_cast<unsigned long long>(id_res[0]["test_keyset_historyid"]);
        test_info_keyset_output.info_keyset_data.handle = test_keyset_handle;
        test_info_keyset_output.info_keyset_data.roid = static_cast<std::string>(id_res[0]["test_keyset_roid"]);
        test_info_keyset_output.info_keyset_data.sponsoring_registrar_handle = registrar_handle;
        test_info_keyset_output.info_keyset_data.create_registrar_handle = registrar_handle;
        test_info_keyset_output.info_keyset_data.update_registrar_handle = Nullable<std::string>();
        test_info_keyset_output.info_keyset_data.creation_time = boost::posix_time::time_from_string("2011-07-01 01:59:59");
        test_info_keyset_output.info_keyset_data.transfer_time = Nullable<boost::posix_time::ptime>();
        test_info_keyset_output.info_keyset_data.authinfopw = "testauthinfo1";
        test_info_keyset_output.info_keyset_data.tech_contacts = Util::vector_of<Fred::ObjectIdHandlePair>
            (Fred::ObjectIdHandlePair(static_cast<unsigned long long>(id_res[0]["admin_contact6_handle_id"]), admin_contact6_handle));
        test_info_keyset_output.info_keyset_data.delete_time = Nullable<boost::posix_time::ptime>();
        test_info_keyset_output.info_keyset_data.dns_keys = Util::vector_of<Fred::DnsKey>
            (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"));

    }
    ~info_keyset_fixture()
    {}
};


/**
 * test call InfoKeyset
*/
BOOST_FIXTURE_TEST_CASE(info_keyset, info_keyset_fixture)
{
    Fred::OperationContext ctx;

    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_1);
    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeysetByHandle(test_keyset_handle).set_lock().exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_2);

    Fred::InfoKeysetOutput info_data_tc_1 = Fred::InfoKeysetByTechContactHandle(admin_contact6_handle).exec(ctx).at(1);
    BOOST_CHECK(test_info_keyset_output == info_data_tc_1);
    Fred::InfoKeysetOutput info_data_tc_2 = Fred::InfoKeysetByTechContactHandle(admin_contact6_handle).set_lock().exec(ctx).at(1);
    BOOST_CHECK(test_info_keyset_output == info_data_tc_2);

    Fred::InfoKeysetOutput info_data_3 = Fred::InfoKeysetById(test_info_keyset_output.info_keyset_data.id).exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_3);
    Fred::InfoKeysetOutput info_data_4 = Fred::InfoKeysetHistoryByRoid(test_info_keyset_output.info_keyset_data.roid).exec(ctx).at(0);
    BOOST_CHECK(test_info_keyset_output == info_data_4);
    Fred::InfoKeysetOutput info_data_5 = Fred::InfoKeysetHistoryById(test_info_keyset_output.info_keyset_data.id).exec(ctx).at(0);
    BOOST_CHECK(test_info_keyset_output == info_data_5);
    Fred::InfoKeysetOutput info_data_6 = Fred::InfoKeysetHistoryByHistoryid(test_info_keyset_output.info_keyset_data.historyid).exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_6);

    //empty output
    BOOST_CHECK(Fred::InfoKeysetHistoryByRoid(xmark+test_info_keyset_output.info_keyset_data.roid).exec(ctx).empty());
    BOOST_CHECK(Fred::InfoKeysetHistoryById(0).exec(ctx).empty());
    ctx.commit_transaction();
}

/**
 * test InfoKeysetByHandle with wrong handle
 */
BOOST_FIXTURE_TEST_CASE(info_keyset_wrong_handle, info_keyset_fixture)
{
    std::string wrong_handle = xmark+test_keyset_handle;

    try
    {
        Fred::OperationContext ctx;
        Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeysetByHandle(wrong_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::InfoKeysetByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_handle());
        BOOST_MESSAGE(wrong_handle);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_handle() == wrong_handle);
    }
}

/**
 * test InfoKeysetById with wrong id
 */
BOOST_FIXTURE_TEST_CASE(info_keyset_wrong_id, info_keyset_fixture)
{
    unsigned long long wrong_id = 0;

    try
    {
        Fred::OperationContext ctx;
        Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeysetById(wrong_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::InfoKeysetById::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_id());
        BOOST_MESSAGE(wrong_id);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_object_id() == wrong_id);
    }
}

/**
 * test InfoKeysetHistoryByHistoryid with wrong historyid
 */
BOOST_FIXTURE_TEST_CASE(info_keyset_history_wrong_historyid, info_keyset_fixture)
{
    unsigned long long wrong_id = 0;

    try
    {
        Fred::OperationContext ctx;
        Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeysetHistoryByHistoryid(wrong_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::InfoKeysetHistoryByHistoryid::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_historyid());
        BOOST_MESSAGE(wrong_id);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_object_historyid() == wrong_id);
    }
}

/**
 * test InfoKeysetByTechContactHandle with unused tech contact handle
 */
BOOST_FIXTURE_TEST_CASE(info_keyset_tech_c_unknown_handle, info_keyset_fixture)
{
    std::string bad_tech_c_handle = admin_contact6_handle+xmark;

    try
    {
        Fred::OperationContext ctx;
        Fred::InfoKeysetByTechContactHandle(bad_tech_c_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::InfoKeysetByTechContactHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_tech_contact_handle());
        BOOST_MESSAGE(bad_tech_c_handle);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_tech_contact_handle() == bad_tech_c_handle);
    }
}

/**
 * test call InfoKeysetDiff
*/
BOOST_FIXTURE_TEST_CASE(info_keyset_diff, info_keyset_fixture)
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
}

struct info_keyset_history_order_fixture : public info_keyset_fixture
{
    info_keyset_history_order_fixture()
    {
        Fred::OperationContext ctx;
        Fred::UpdateKeyset(test_keyset_history_handle, registrar_handle)
            .rem_tech_contact(admin_contact6_handle)
            .exec(ctx);
        ctx.commit_transaction();
    }
};

/**
 * test InfoKeysetHistoryByRoid output data sorted by historyid in descending order (current data first, older next)
*/

BOOST_FIXTURE_TEST_CASE(info_keyset_history_order, info_keyset_history_order_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoKeysetOutput keyset_history_info = Fred::InfoKeysetByHandle(test_keyset_history_handle).exec(ctx);

    std::vector<Fred::InfoKeysetOutput> keyset_history_info_by_roid = Fred::InfoKeysetHistoryByRoid(keyset_history_info.info_keyset_data.roid).exec(ctx);
    BOOST_CHECK(keyset_history_info_by_roid.size() == 2);
    BOOST_CHECK(keyset_history_info_by_roid.at(0).info_keyset_data.historyid > keyset_history_info_by_roid.at(1).info_keyset_data.historyid);

    BOOST_CHECK(keyset_history_info_by_roid.at(0).info_keyset_data.tech_contacts.empty());
    BOOST_CHECK(keyset_history_info_by_roid.at(1).info_keyset_data.tech_contacts.at(0).handle == admin_contact6_handle);

    std::vector<Fred::InfoKeysetOutput> keyset_history_info_by_id = Fred::InfoKeysetHistoryById(keyset_history_info.info_keyset_data.id).exec(ctx);
    BOOST_CHECK(keyset_history_info_by_id.size() == 2);
    BOOST_CHECK(keyset_history_info_by_id.at(0).info_keyset_data.historyid > keyset_history_info_by_roid.at(1).info_keyset_data.historyid);

    BOOST_CHECK(keyset_history_info_by_id.at(0).info_keyset_data.tech_contacts.empty());
    BOOST_CHECK(keyset_history_info_by_id.at(1).info_keyset_data.tech_contacts.at(0).handle == admin_contact6_handle);
}


BOOST_AUTO_TEST_SUITE_END();
