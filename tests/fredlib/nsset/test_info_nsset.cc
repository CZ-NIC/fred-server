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
#include <fredlib/nsset.h>
#include <fredlib/contact.h>
#include "src/fredlib/nsset/info_nsset_impl.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-info-domain";

struct info_nsset_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string admin_contact3_handle;
    std::string test_nsset_handle;
    std::string test_nsset_history_handle;
    std::string test_nsset_dnsname;

    info_nsset_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark)
    , admin_contact3_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , test_nsset_handle ( std::string("TEST-NSSET-HANDLE")+xmark)
    , test_nsset_history_handle ( std::string("TEST-NSSET-HISTORY-HANDLE")+xmark)
    , test_nsset_dnsname (std::string("ns")+xmark+".ns.nic.cz")
    {
        namespace ip = boost::asio::ip;

        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(admin_contact3_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost(test_nsset_dnsname,  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle))
                .exec(ctx);

        Fred::CreateNsset(test_nsset_history_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("c.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle))
                .exec(ctx);

        Fred::UpdateNsset(test_nsset_history_handle, registrar_handle)
        .rem_tech_contact(admin_contact3_handle)
        .exec(ctx);


        ctx.commit_transaction();//commit fixture
    }
    ~info_nsset_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestInfoNsset, info_nsset_fixture)

/**
 * test call InfoNsset
*/
BOOST_AUTO_TEST_CASE(info_nsset)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoNssetOutput> nsset_res;

    BOOST_MESSAGE(Fred::InfoNsset()
                .set_history_query(false)
                .explain_analyze(ctx,nsset_res,"Europe/Prague")
                );

    BOOST_MESSAGE(Fred::InfoNsset()
                .set_inline_view_filter(Database::ParamQuery("info_nsset_handle = ").param_text(test_nsset_handle))
                .set_history_query(false)
                .explain_analyze(ctx,nsset_res,"Europe/Prague")
                );

    //InfoNssetByDNSFqdn
    BOOST_MESSAGE(Fred::InfoNsset()
                .set_cte_id_filter(Database::ParamQuery("SELECT id FROM host WHERE fqdn = ").param_text("a.ns.nic.cz"))
                .set_history_query(false)
                .explain_analyze(ctx,nsset_res,"Europe/Prague")
                );

    //InfoNssetByTechContactHandle
    BOOST_MESSAGE(Fred::InfoNsset()
                .set_cte_id_filter(Database::ParamQuery(
                    "SELECT ncm.nssetid"
                    " FROM object_registry oreg"
                    " JOIN  enum_object_type eot ON oreg.type = eot.id AND eot.name = 'contact'"
                    " JOIN nsset_contact_map ncm ON ncm.contactid = oreg.id"
                    " WHERE oreg.name = UPPER(").param_text(admin_contact3_handle)(") AND oreg.erdate IS NULL")
                    )
                .set_history_query(false)
                .explain_analyze(ctx,nsset_res,"Europe/Prague")
                );

    BOOST_MESSAGE(Fred::InfoNsset()
                .set_cte_id_filter(Database::ParamQuery("SELECT id FROM host WHERE fqdn = ").param_text("a.ns.nic.cz"))
                .set_inline_view_filter(Database::ParamQuery("info_nsset_handle = ").param_text(test_nsset_handle))
                .set_history_query(false)
                .explain_analyze(ctx,nsset_res,"Europe/Prague")
                );

    BOOST_MESSAGE(Fred::InfoNsset()
                .set_cte_id_filter(Database::ParamQuery("SELECT id FROM host WHERE fqdn = ").param_text("a.ns.nic.cz"))
                .set_inline_view_filter(Database::ParamQuery("info_nsset_handle = ").param_text(test_nsset_handle))
                .set_history_query(true)
                .explain_analyze(ctx,nsset_res,"Europe/Prague")
                );

    Fred::InfoNssetOutput info_data_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    Fred::InfoNssetOutput info_data_2 = Fred::InfoNssetByHandle(test_nsset_handle).set_lock().exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);

    Fred::InfoNssetOutput info_data_dns_1 = Fred::InfoNssetByDNSFqdn(test_nsset_dnsname).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_dns_1);
    Fred::InfoNssetOutput info_data_dns_2 = Fred::InfoNssetByDNSFqdn(test_nsset_dnsname).set_lock().exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_dns_2);

    Fred::InfoNssetOutput info_data_tc_1 = Fred::InfoNssetByTechContactHandle(admin_contact3_handle).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_tc_1);
    Fred::InfoNssetOutput info_data_tc_2 = Fred::InfoNssetByTechContactHandle(admin_contact3_handle).set_lock().exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_tc_2);

    Fred::InfoNssetOutput info_data_3 = Fred::InfoNssetById(info_data_1.info_nsset_data.id).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_3);
    Fred::InfoNssetOutput info_data_4 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_4);
    Fred::InfoNssetOutput info_data_5 = Fred::InfoNssetHistoryById(info_data_1.info_nsset_data.id).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_5);
    Fred::InfoNssetOutput info_data_6 = Fred::InfoNssetHistoryByHistoryid(info_data_1.info_nsset_data.historyid).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_6);

    ctx.commit_transaction();
}

/**
 * test call InfoNssetDiff
*/
BOOST_AUTO_TEST_CASE(info_nsset_diff)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOutput nsset_info1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    Fred::InfoNssetOutput nsset_info2 = Fred::InfoNssetByHandle(test_nsset_handle).set_lock().exec(ctx);

    Fred::InfoNssetDiff test_diff, test_empty_diff;

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

    BOOST_MESSAGE(Fred::diff_nsset_data(nsset_info1.info_nsset_data,nsset_info2.info_nsset_data).to_string());

    //because of changes to Nullable::operator<<
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Database::QPNull))[0][0].isnull());
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Nullable<std::string>()))[0][0].isnull());
}


/**
 * test InfoNssetHistory output data sorted by historyid in descending order (current data first, older next)
*/

BOOST_AUTO_TEST_CASE(info_nsset_history_order)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOutput nsset_history_info = Fred::InfoNssetByHandle(test_nsset_history_handle).exec(ctx);

    std::vector<Fred::InfoNssetOutput> nsset_history_info_by_roid = Fred::InfoNssetHistory(nsset_history_info.info_nsset_data.roid).exec(ctx);
    BOOST_CHECK(nsset_history_info_by_roid.size() == 2);
    BOOST_CHECK(nsset_history_info_by_roid.at(0).info_nsset_data.historyid > nsset_history_info_by_roid.at(1).info_nsset_data.historyid);

    BOOST_CHECK(nsset_history_info_by_roid.at(0).info_nsset_data.tech_contacts.empty());
    BOOST_CHECK(nsset_history_info_by_roid.at(1).info_nsset_data.tech_contacts.at(0).handle == admin_contact3_handle);

    std::vector<Fred::InfoNssetOutput> nsset_history_info_by_id = Fred::InfoNssetHistoryById(nsset_history_info.info_nsset_data.id).exec(ctx);
    BOOST_CHECK(nsset_history_info_by_id.size() == 2);
    BOOST_CHECK(nsset_history_info_by_id.at(0).info_nsset_data.historyid > nsset_history_info_by_roid.at(1).info_nsset_data.historyid);

    BOOST_CHECK(nsset_history_info_by_id.at(0).info_nsset_data.tech_contacts.empty());
    BOOST_CHECK(nsset_history_info_by_id.at(1).info_nsset_data.tech_contacts.at(0).handle == admin_contact3_handle);
}


BOOST_AUTO_TEST_SUITE_END();//TestInfoNsset
