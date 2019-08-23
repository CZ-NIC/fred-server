/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/delete_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_diff.hh"
#include "libfred/registrable_object/keyset/info_keyset_impl.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_AUTO_TEST_SUITE(TestInfoKeyset);

struct info_keyset_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact4_handle;
    std::string admin_contact5_handle;
    std::string admin_contact6_handle;
    std::string admin_contact6_uuid;
    std::string test_keyset_handle;
    std::string test_keyset_history_handle;

    ::LibFred::InfoKeysetOutput test_info_keyset_output;

    info_keyset_fixture()
    :xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , admin_contact4_handle(std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark)
    , admin_contact5_handle(std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark)
    , admin_contact6_handle(std::string("e70a23bd-0727-4240-8bdd-d72af02f0a56")+xmark)
    , admin_contact6_uuid(std::string("TEST-ADMIN-CONTACT6-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    , test_keyset_history_handle(std::string("TEST-KEYSET-HISTORY-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);

        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact4_handle,registrar_handle)
            .set_name(admin_contact4_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("admin_contact4_handle: ") + admin_contact4_handle);

        ::LibFred::CreateContact(admin_contact5_handle,registrar_handle)
            .set_name(admin_contact5_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("admin_contact5_handle: ") + admin_contact5_handle);

        ::LibFred::CreateContact(admin_contact6_handle,registrar_handle)
            .set_name(admin_contact6_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("admin_contact6_handle: ") + admin_contact6_handle);

        ::LibFred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_authinfo("testauthinfo1")
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .set_dns_keys(Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("test_keyset_handle: ") + test_keyset_handle);

        ::LibFred::CreateKeyset(test_keyset_history_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .set_dns_keys(Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("test_keyset_handle: ") + test_keyset_handle);

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
        test_info_keyset_output.info_keyset_data.tech_contacts =
            Util::vector_of<::LibFred::RegistrableObject::Contact::ContactReference>(
                    ::LibFred::RegistrableObject::Contact::ContactReference(
                            static_cast<unsigned long long>(id_res[0]["admin_contact6_handle_id"]),
                            admin_contact6_handle,
                            LibFred::RegistrableObject::make_uuid_of<LibFred::Object_Type::contact>(admin_contact6_uuid)));
        test_info_keyset_output.info_keyset_data.delete_time = Nullable<boost::posix_time::ptime>();
        test_info_keyset_output.info_keyset_data.dns_keys = Util::vector_of<::LibFred::DnsKey>
            (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"));

    }
    ~info_keyset_fixture()
    {}
};


/**
 * test call InfoKeyset
*/
BOOST_FIXTURE_TEST_CASE(info_keyset, info_keyset_fixture)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoKeysetOutput info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_1);
    ::LibFred::InfoKeysetOutput info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).set_lock().exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_2);

    ::LibFred::InfoKeysetOutput info_data_tc_1 = ::LibFred::InfoKeysetByTechContactHandle(admin_contact6_handle).exec(ctx).at(1);
    BOOST_CHECK(test_info_keyset_output == info_data_tc_1);
    ::LibFred::InfoKeysetOutput info_data_tc_2 = ::LibFred::InfoKeysetByTechContactHandle(admin_contact6_handle).set_lock().exec(ctx).at(1);
    BOOST_CHECK(test_info_keyset_output == info_data_tc_2);

    ::LibFred::InfoKeysetOutput info_data_3 = ::LibFred::InfoKeysetById(test_info_keyset_output.info_keyset_data.id).exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_3);
    ::LibFred::InfoKeysetOutput info_data_4 = ::LibFred::InfoKeysetHistoryByRoid(test_info_keyset_output.info_keyset_data.roid).exec(ctx).at(0);
    BOOST_CHECK(test_info_keyset_output == info_data_4);
    ::LibFred::InfoKeysetOutput info_data_5 = ::LibFred::InfoKeysetHistoryById(test_info_keyset_output.info_keyset_data.id).exec(ctx).at(0);
    BOOST_CHECK(test_info_keyset_output == info_data_5);
    ::LibFred::InfoKeysetOutput info_data_6 = ::LibFred::InfoKeysetHistoryByHistoryid(test_info_keyset_output.info_keyset_data.historyid).exec(ctx);
    BOOST_CHECK(test_info_keyset_output == info_data_6);

    //empty output
    BOOST_CHECK(::LibFred::InfoKeysetHistoryByRoid(xmark+test_info_keyset_output.info_keyset_data.roid).exec(ctx).empty());
    BOOST_CHECK(::LibFred::InfoKeysetHistoryById(0).exec(ctx).empty());
    ctx.commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(test_info_keyset_output_timestamp, info_keyset_fixture)
{
    const std::string timezone = "Europe/Prague";
    ::LibFred::OperationContextCreator ctx;
    const ::LibFred::InfoKeysetOutput keyset_output_by_handle              = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, timezone);
    const ::LibFred::InfoKeysetOutput keyset_output_by_id                  = ::LibFred::InfoKeysetById(keyset_output_by_handle.info_keyset_data.id).exec(ctx, timezone);
    const ::LibFred::InfoKeysetOutput keyset_output_history_by_historyid   = ::LibFred::InfoKeysetHistoryByHistoryid(keyset_output_by_handle.info_keyset_data.historyid).exec(ctx, timezone);
    const ::LibFred::InfoKeysetOutput keyset_output_history_by_id          = ::LibFred::InfoKeysetHistoryById(keyset_output_by_handle.info_keyset_data.id).exec(ctx, timezone).at(0);
    const ::LibFred::InfoKeysetOutput keyset_output_history_by_roid        = ::LibFred::InfoKeysetHistoryByRoid(keyset_output_by_handle.info_keyset_data.roid).exec(ctx, timezone).at(0);

    /* all are equal amongst themselves ... */
    BOOST_CHECK(
            keyset_output_by_handle.utc_timestamp              == keyset_output_by_id.utc_timestamp
        &&  keyset_output_by_id.utc_timestamp                  == keyset_output_history_by_historyid.utc_timestamp
        &&  keyset_output_history_by_historyid.utc_timestamp   == keyset_output_history_by_id.utc_timestamp
        &&  keyset_output_history_by_id.utc_timestamp          == keyset_output_history_by_roid.utc_timestamp
    );

    /* ... and one of them is equal to correct constant value */
    BOOST_CHECK_EQUAL(
        keyset_output_by_handle.utc_timestamp,
        boost::posix_time::time_from_string( static_cast<std::string>( ctx.get_conn().exec("SELECT now()::timestamp")[0][0] ) )
    );
}

/**
 * test InfoKeysetByHandle with wrong handle
 */
BOOST_FIXTURE_TEST_CASE(info_keyset_wrong_handle, info_keyset_fixture)
{
    std::string wrong_handle = xmark+test_keyset_handle;

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoKeysetOutput info_data_1 = ::LibFred::InfoKeysetByHandle(wrong_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoKeysetByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_handle());
        BOOST_TEST_MESSAGE(wrong_handle);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
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
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoKeysetOutput info_data_1 = ::LibFred::InfoKeysetById(wrong_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoKeysetById::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_id());
        BOOST_TEST_MESSAGE(wrong_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
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
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoKeysetOutput info_data_1 = ::LibFred::InfoKeysetHistoryByHistoryid(wrong_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoKeysetHistoryByHistoryid::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_historyid());
        BOOST_TEST_MESSAGE(wrong_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_object_historyid() == wrong_id);
    }
}

/**
 * test InfoKeysetByTechContactHandle with unused tech contact handle
 */
BOOST_FIXTURE_TEST_CASE(info_keyset_tech_c_unknown_handle, info_keyset_fixture)
{
    std::string bad_tech_c_handle = admin_contact6_handle+xmark;

    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::InfoKeysetOutput> info = ::LibFred::InfoKeysetByTechContactHandle(bad_tech_c_handle).exec(ctx);
    BOOST_CHECK(info.empty());
}

/**
 * test call InfoKeysetDiff
*/
BOOST_FIXTURE_TEST_CASE(info_keyset_diff, info_keyset_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoKeysetOutput keyset_info1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    ::LibFred::InfoKeysetOutput keyset_info2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).set_lock().exec(ctx);

    ::LibFred::InfoKeysetDiff test_diff, test_empty_diff;

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

    BOOST_TEST_MESSAGE(test_diff.to_string());
    BOOST_TEST_MESSAGE(test_empty_diff.to_string());

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_TEST_MESSAGE(::LibFred::diff_keyset_data(keyset_info1.info_keyset_data,keyset_info2.info_keyset_data).to_string());
}

struct info_keyset_history_order_fixture : public info_keyset_fixture
{
    info_keyset_history_order_fixture()
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::UpdateKeyset(test_keyset_history_handle, registrar_handle)
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
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoKeysetOutput keyset_history_info = ::LibFred::InfoKeysetByHandle(test_keyset_history_handle).exec(ctx);

    std::vector<::LibFred::InfoKeysetOutput> keyset_history_info_by_roid = ::LibFred::InfoKeysetHistoryByRoid(keyset_history_info.info_keyset_data.roid).exec(ctx);
    BOOST_CHECK(keyset_history_info_by_roid.size() == 2);
    BOOST_CHECK(keyset_history_info_by_roid.at(0).info_keyset_data.historyid > keyset_history_info_by_roid.at(1).info_keyset_data.historyid);

    BOOST_CHECK(keyset_history_info_by_roid.at(0).info_keyset_data.tech_contacts.empty());
    BOOST_CHECK(keyset_history_info_by_roid.at(1).info_keyset_data.tech_contacts.at(0).handle == admin_contact6_handle);

    std::vector<::LibFred::InfoKeysetOutput> keyset_history_info_by_id = ::LibFred::InfoKeysetHistoryById(keyset_history_info.info_keyset_data.id).exec(ctx);
    BOOST_CHECK(keyset_history_info_by_id.size() == 2);
    BOOST_CHECK(keyset_history_info_by_id.at(0).info_keyset_data.historyid > keyset_history_info_by_roid.at(1).info_keyset_data.historyid);

    BOOST_CHECK(keyset_history_info_by_id.at(0).info_keyset_data.tech_contacts.empty());
    BOOST_CHECK(keyset_history_info_by_id.at(1).info_keyset_data.tech_contacts.at(0).handle == admin_contact6_handle);
}


BOOST_AUTO_TEST_SUITE_END();
