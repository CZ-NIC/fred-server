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
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/delete_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_diff.hh"
#include "libfred/registrable_object/nsset/info_nsset_impl.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_AUTO_TEST_SUITE(TestInfoNsset);

struct info_nsset_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string admin_contact3_handle;
    std::string test_nsset_handle;
    std::string test_nsset_history_handle;
    std::string test_nsset_dnsname;

    ::LibFred::InfoNssetOutput test_info_nsset_output;

    info_nsset_fixture()
    : xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark)
    , admin_contact3_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , test_nsset_handle ( std::string("TEST-NSSET-HANDLE")+xmark)
    , test_nsset_history_handle ( std::string("TEST-NSSET-HISTORY-HANDLE")+xmark)
    , test_nsset_dnsname (std::string("ns")+xmark+".ns.nic.cz")
    {
        namespace ip = boost::asio::ip;

        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateContact(admin_contact3_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost(test_nsset_dnsname,  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                )
                .set_authinfo("testauthinfo1")
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle))
                .exec(ctx);

        ::LibFred::CreateNsset(test_nsset_history_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost("c.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle))
                .exec(ctx);

        //id query
        Database::Result id_res = ctx.get_conn().exec_params("SELECT"
        " (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND name = UPPER($1::text)) AS test_nsset_id"
        ",  (SELECT roid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND name = UPPER($1::text)) AS test_nsset_roid"
        ",  (SELECT crhistoryid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND name = UPPER($1::text)) AS test_nsset_crhistoryid"
        ",  (SELECT historyid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND name = UPPER($1::text)) AS test_nsset_historyid"
        ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) AND name = UPPER($2::text)) AS admin_contact3_handle_id"
        ", (SELECT id FROM registrar WHERE handle = UPPER($3::text)) AS registrar_handle_id"
        , Database::query_param_list(test_nsset_handle)(admin_contact3_handle)(registrar_handle));

        //crdate fix
        ctx.get_conn().exec_params("UPDATE object_registry SET crdate = $1::timestamp WHERE id = $2::bigint",
            Database::query_param_list("2011-06-30T23:59:59")(static_cast<unsigned long long>(id_res[0]["test_nsset_id"])));

        ctx.commit_transaction();

        test_info_nsset_output.info_nsset_data.id = static_cast<unsigned long long>(id_res[0]["test_nsset_id"]);
        test_info_nsset_output.info_nsset_data.roid = static_cast<std::string>(id_res[0]["test_nsset_roid"]);
        test_info_nsset_output.info_nsset_data.crhistoryid = static_cast<unsigned long long>(id_res[0]["test_nsset_crhistoryid"]);
        test_info_nsset_output.info_nsset_data.historyid = static_cast<unsigned long long>(id_res[0]["test_nsset_historyid"]);
        test_info_nsset_output.info_nsset_data.handle = test_nsset_handle;
        test_info_nsset_output.info_nsset_data.roid = static_cast<std::string>(id_res[0]["test_nsset_roid"]);
        test_info_nsset_output.info_nsset_data.sponsoring_registrar_handle = registrar_handle;
        test_info_nsset_output.info_nsset_data.create_registrar_handle = registrar_handle;
        test_info_nsset_output.info_nsset_data.update_registrar_handle = Nullable<std::string>();
        test_info_nsset_output.info_nsset_data.creation_time = boost::posix_time::time_from_string("2011-07-01 01:59:59");
        test_info_nsset_output.info_nsset_data.transfer_time = Nullable<boost::posix_time::ptime>();
        test_info_nsset_output.info_nsset_data.authinfopw = "testauthinfo1";
        test_info_nsset_output.info_nsset_data.tech_contacts = Util::vector_of<::LibFred::ObjectIdHandlePair>
            (::LibFred::ObjectIdHandlePair(static_cast<unsigned long long>(id_res[0]["admin_contact3_handle_id"]),admin_contact3_handle));
        test_info_nsset_output.info_nsset_data.delete_time = Nullable<boost::posix_time::ptime>();
        test_info_nsset_output.info_nsset_data.dns_hosts = Util::vector_of<::LibFred::DnsHost>
            (::LibFred::DnsHost("a.ns.nic.cz", Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3"))))
            (::LibFred::DnsHost(test_nsset_dnsname, Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4"))));
        test_info_nsset_output.info_nsset_data.tech_check_level = Nullable<short>(0);

    }
    ~info_nsset_fixture()
    {}
};

/**
 * test call InfoNsset
*/
BOOST_FIXTURE_TEST_CASE(info_nsset, info_nsset_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoNssetOutput info_data_1 = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    BOOST_CHECK(test_info_nsset_output == info_data_1);
    ::LibFred::InfoNssetOutput info_data_2 = ::LibFred::InfoNssetByHandle(test_nsset_handle).set_lock().exec(ctx);
    BOOST_CHECK(test_info_nsset_output == info_data_2);
    ::LibFred::InfoNssetOutput info_data_dns_1 = ::LibFred::InfoNssetByDNSFqdn(test_nsset_dnsname).exec(ctx).at(0);
    BOOST_CHECK(test_info_nsset_output == info_data_dns_1);
    ::LibFred::InfoNssetOutput info_data_dns_2 = ::LibFred::InfoNssetByDNSFqdn(test_nsset_dnsname).set_lock().exec(ctx).at(0);
    BOOST_CHECK(test_info_nsset_output == info_data_dns_2);
    ::LibFred::InfoNssetOutput info_data_tc_1 = ::LibFred::InfoNssetByTechContactHandle(admin_contact3_handle).exec(ctx).at(1);
    BOOST_CHECK(test_info_nsset_output == info_data_tc_1);
    ::LibFred::InfoNssetOutput info_data_tc_2 = ::LibFred::InfoNssetByTechContactHandle(admin_contact3_handle).set_lock().exec(ctx).at(1);
    BOOST_CHECK(test_info_nsset_output == info_data_tc_2);
    ::LibFred::InfoNssetOutput info_data_3 = ::LibFred::InfoNssetById(test_info_nsset_output.info_nsset_data.id).exec(ctx);
    BOOST_CHECK(test_info_nsset_output == info_data_3);
    ::LibFred::InfoNssetOutput info_data_4 = ::LibFred::InfoNssetHistoryByRoid(test_info_nsset_output.info_nsset_data.roid).exec(ctx).at(0);
    BOOST_CHECK(test_info_nsset_output == info_data_4);
    ::LibFred::InfoNssetOutput info_data_5 = ::LibFred::InfoNssetHistoryById(test_info_nsset_output.info_nsset_data.id).exec(ctx).at(0);
    BOOST_CHECK(test_info_nsset_output == info_data_5);
    ::LibFred::InfoNssetOutput info_data_6 = ::LibFred::InfoNssetHistoryByHistoryid(test_info_nsset_output.info_nsset_data.historyid).exec(ctx);
    BOOST_CHECK(test_info_nsset_output == info_data_6);
    //empty output
    BOOST_CHECK(::LibFred::InfoNssetHistoryByRoid(xmark+test_info_nsset_output.info_nsset_data.roid).exec(ctx).empty());
    BOOST_CHECK(::LibFred::InfoNssetHistoryById(0).exec(ctx).empty());

}

BOOST_FIXTURE_TEST_CASE(test_info_nsset_output_timestamp, info_nsset_fixture)
{
    const std::string timezone = "Europe/Prague";
    ::LibFred::OperationContextCreator ctx;
    const ::LibFred::InfoNssetOutput nsset_output_by_handle              = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx, timezone);
    const ::LibFred::InfoNssetOutput nsset_output_by_id                  = ::LibFred::InfoNssetById(nsset_output_by_handle.info_nsset_data.id).exec(ctx, timezone);
    const ::LibFred::InfoNssetOutput nsset_output_history_by_historyid   = ::LibFred::InfoNssetHistoryByHistoryid(nsset_output_by_handle.info_nsset_data.historyid).exec(ctx, timezone);
    const ::LibFred::InfoNssetOutput nsset_output_history_by_id          = ::LibFred::InfoNssetHistoryById(nsset_output_by_handle.info_nsset_data.id).exec(ctx, timezone).at(0);
    const ::LibFred::InfoNssetOutput nsset_output_history_by_roid        = ::LibFred::InfoNssetHistoryByRoid(nsset_output_by_handle.info_nsset_data.roid).exec(ctx, timezone).at(0);

    /* all are equal amongst themselves ... */
    BOOST_CHECK(
            nsset_output_by_handle.utc_timestamp              == nsset_output_by_id.utc_timestamp
        &&  nsset_output_by_id.utc_timestamp                  == nsset_output_history_by_historyid.utc_timestamp
        &&  nsset_output_history_by_historyid.utc_timestamp   == nsset_output_history_by_id.utc_timestamp
        &&  nsset_output_history_by_id.utc_timestamp          == nsset_output_history_by_roid.utc_timestamp
    );

    /* ... and one of them is equal to correct constant value */
    BOOST_CHECK_EQUAL(
        nsset_output_by_handle.utc_timestamp,
        boost::posix_time::time_from_string( static_cast<std::string>( ctx.get_conn().exec("SELECT now()::timestamp")[0][0] ) )
    );
}

/**
 * test InfoNssetByHandle with wrong handle
 */
BOOST_FIXTURE_TEST_CASE(info_nsset_wrong_handle, info_nsset_fixture)
{
    std::string wrong_handle = xmark+test_nsset_handle;

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoNssetOutput info_data_1 = ::LibFred::InfoNssetByHandle(wrong_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoNssetByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_handle());
        BOOST_TEST_MESSAGE(wrong_handle);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_handle() == wrong_handle);
    }
}

/**
 * test InfoNssetById with wrong id
 */
BOOST_FIXTURE_TEST_CASE(info_nsset_wrong_id, info_nsset_fixture)
{
    unsigned long long wrong_id = 0;

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoNssetOutput info_data_1 = ::LibFred::InfoNssetById(wrong_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoNssetById::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_id());
        BOOST_TEST_MESSAGE(wrong_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_object_id() == wrong_id);
    }
}

/**
 * test InfoNssetHistoryByHistoryid with wrong historyid
 */
BOOST_FIXTURE_TEST_CASE(info_nsset_history_wrong_historyid, info_nsset_fixture)
{
    unsigned long long wrong_id = 0;

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoNssetOutput info_data_1 = ::LibFred::InfoNssetHistoryByHistoryid(wrong_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoNssetHistoryByHistoryid::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_historyid());
        BOOST_TEST_MESSAGE(wrong_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_object_historyid() == wrong_id);
    }
}

/**
 * test InfoNssetByDNSFqdn with unused fqdn
 */
BOOST_FIXTURE_TEST_CASE(info_nsset_ns_unknown_ns, info_nsset_fixture)
{
    std::string bad_ns_fqdn = xmark+test_nsset_dnsname;

    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::InfoNssetOutput> info = ::LibFred::InfoNssetByDNSFqdn(bad_ns_fqdn).exec(ctx);
    BOOST_CHECK(info.empty());
}

/**
 * test InfoNssetByTechContactHandle with unused tech contact handle
 */
BOOST_FIXTURE_TEST_CASE(info_nsset_tech_c_unknown_handle, info_nsset_fixture)
{
    std::string bad_tech_c_handle = admin_contact3_handle+xmark;

    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::InfoNssetOutput> info = ::LibFred::InfoNssetByTechContactHandle(bad_tech_c_handle).exec(ctx);
    BOOST_CHECK(info.empty());
}

/**
 * test call InfoNssetDiff
*/
BOOST_FIXTURE_TEST_CASE(info_nsset_diff, info_nsset_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoNssetOutput nsset_info1 = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    ::LibFred::InfoNssetOutput nsset_info2 = ::LibFred::InfoNssetByHandle(test_nsset_handle).set_lock().exec(ctx);

    ::LibFred::InfoNssetDiff test_diff, test_empty_diff;

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

    BOOST_TEST_MESSAGE(::LibFred::diff_nsset_data(nsset_info1.info_nsset_data,nsset_info2.info_nsset_data).to_string());

}

struct info_nsset_history_fixture : public info_nsset_fixture
{
    info_nsset_history_fixture()
    {
        ::LibFred::OperationContextCreator ctx;
            ::LibFred::UpdateNsset(test_nsset_history_handle, registrar_handle)
            .rem_tech_contact(admin_contact3_handle)
            .exec(ctx);
        ctx.commit_transaction();
    }
};

/**
 * test InfoNssetHistoryByRoid output data sorted by historyid in descending order (current data first, older next)
*/
BOOST_FIXTURE_TEST_CASE(info_nsset_history_order, info_nsset_history_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoNssetOutput nsset_history_info = ::LibFred::InfoNssetByHandle(test_nsset_history_handle).exec(ctx);

    std::vector<::LibFred::InfoNssetOutput> nsset_history_info_by_roid = ::LibFred::InfoNssetHistoryByRoid(nsset_history_info.info_nsset_data.roid).exec(ctx);
    BOOST_CHECK(nsset_history_info_by_roid.size() == 2);
    BOOST_CHECK(nsset_history_info_by_roid.at(0).info_nsset_data.historyid > nsset_history_info_by_roid.at(1).info_nsset_data.historyid);

    BOOST_CHECK(nsset_history_info_by_roid.at(0).info_nsset_data.tech_contacts.empty());
    BOOST_CHECK(nsset_history_info_by_roid.at(1).info_nsset_data.tech_contacts.at(0).handle == admin_contact3_handle);

    std::vector<::LibFred::InfoNssetOutput> nsset_history_info_by_id = ::LibFred::InfoNssetHistoryById(nsset_history_info.info_nsset_data.id).exec(ctx);
    BOOST_CHECK(nsset_history_info_by_id.size() == 2);
    BOOST_CHECK(nsset_history_info_by_id.at(0).info_nsset_data.historyid > nsset_history_info_by_roid.at(1).info_nsset_data.historyid);

    BOOST_CHECK(nsset_history_info_by_id.at(0).info_nsset_data.tech_contacts.empty());
    BOOST_CHECK(nsset_history_info_by_id.at(1).info_nsset_data.tech_contacts.at(0).handle == admin_contact3_handle);
}


BOOST_AUTO_TEST_SUITE_END();
