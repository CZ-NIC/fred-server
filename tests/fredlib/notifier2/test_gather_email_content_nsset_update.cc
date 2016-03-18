/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 */

#include <boost/test/unit_test.hpp>
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/notifier2/util.h"
#include "tests/fredlib/notifier2/fixture_data.h"

#include "src/fredlib/notifier2/gather_email_data/gather_email_content.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>


BOOST_AUTO_TEST_SUITE(TestNotifier2)
BOOST_AUTO_TEST_SUITE(GatherEmailContent)
BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(Update)

template<typename Thas_nsset> struct has_nsset_empty_update : Thas_nsset {

    const unsigned long long logd_request_id;
    const unsigned long long new_historyid;

    has_nsset_empty_update()
    :   logd_request_id(12345),
        new_historyid(
            Fred::UpdateNsset(Thas_nsset::nsset.handle, Thas_nsset::nsset.create_registrar_handle)
                .set_logd_request_id(logd_request_id).exec(Thas_nsset::ctx)
        )
    { }
};

template<typename Thas_nsset>  struct has_nsset_big_update : Thas_nsset {
    const unsigned long long logd_request_id;
    const Fred::InfoNssetData new_nsset_data;

    has_nsset_big_update()
    :   logd_request_id(12345),
        new_nsset_data(
            Fred::InfoNssetHistoryByHistoryid(
                Fred::UpdateNsset(Thas_nsset::nsset.handle, Thas_nsset::nsset.create_registrar_handle)
                    .set_sponsoring_registrar(
                        Test::exec(
                            Fred::CreateRegistrar("NEW_REGISTRAR1")
                                .set_name("Re Gistra R Jr.")
                                .set_url("registrar2.cz"),
                            Thas_nsset::ctx
                        ).handle
                    )
                    .set_authinfo(Thas_nsset::nsset.authinfopw + "X")
                    .add_dns(
                        Fred::DnsHost(
                            "host2.nic.cz",
                            boost::assign::list_of
                                (boost::asio::ip::address::from_string("127.0.0.100"))
                                (boost::asio::ip::address::from_string("192.168.0.100"))
                        )
                    )
                    .add_tech_contact(
                        Test::exec(
                            Fred::CreateContact("CONTACT3", Thas_nsset::registrar.handle)
                                .set_email("contact.3@nic.cz")
                                .set_notifyemail("contact.3.notify@nic.cz"),
                            Thas_nsset::ctx
                        ).handle
                    )
                    .set_tech_check_level(Thas_nsset::nsset.tech_check_level.get_value_or(0) < 10 ? Thas_nsset::nsset.tech_check_level.get_value_or(0) + 1 : 0)
                    .set_logd_request_id(logd_request_id)
                    .exec(Thas_nsset::ctx)
            ).exec(Thas_nsset::ctx)
            .info_nsset_data
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_minimal_update_no_data, has_nsset_empty_update<has_empty_nsset>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "2";
    etalon["handle"] = nsset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::nsset, Notification::updated),
                registrar.id,
                new_historyid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_minimal_update_from_empty_data, has_nsset_empty_update<has_full_nsset>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "2";
    etalon["handle"] = nsset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::nsset, Notification::updated),
                registrar.id,
                new_historyid,
                input_svtrid
            )
        )
    );
}

/* TODO zkusit udelat technicky kontakt se stejnym handlem (ale jinym id) a otestovat co se stane, kdyz je u nssetu prohodim */

BOOST_FIXTURE_TEST_CASE(test_big_update_from_empty_data, has_nsset_big_update<has_empty_nsset>)
{
    const std::string input_svtrid = "xyz-987";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "2";
    etalon["handle"] = nsset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["object.authinfo"]       = "1";
    etalon["object.authinfo.old"]   = nsset         .authinfopw;
    etalon["object.authinfo.new"]   = new_nsset_data.authinfopw;

    etalon["nsset.check_level"]       = "1";
    etalon["nsset.check_level.old"]   = boost::lexical_cast<std::string>( nsset         .tech_check_level.get_value() );
    etalon["nsset.check_level.new"]   = boost::lexical_cast<std::string>( new_nsset_data.tech_check_level.get_value() );

    struct extract {
        static std::set<std::string> handles(const std::vector<Fred::ObjectIdHandlePair>& _in) {
            std::set<std::string> result;
            BOOST_FOREACH(const Fred::ObjectIdHandlePair& elem, _in) {
                result.insert(elem.handle);
            }
            return result;
        }
    };

    etalon["nsset.tech_c"]       = "1";
    etalon["nsset.tech_c.old"]   = boost::join( extract::handles( nsset         .tech_contacts ), " " );
    etalon["nsset.tech_c.new"]   = boost::join( extract::handles( new_nsset_data.tech_contacts ), " " );

    etalon["nsset.dns"]         = "1";
    etalon["nsset.dns.new.0"]   = "host2.nic.cz (127.0.0.100 192.168.0.100)";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::nsset, Notification::updated),
                registrar.id,
                new_nsset_data.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_big_update_from_full_data, has_nsset_big_update<has_full_nsset>)
{
    const std::string input_svtrid = "xyz-987";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "2";
    etalon["handle"] = nsset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["object.authinfo"]       = "1";
    etalon["object.authinfo.old"]   = nsset         .authinfopw;
    etalon["object.authinfo.new"]   = new_nsset_data.authinfopw;

    etalon["nsset.check_level"]       = "1";
    etalon["nsset.check_level.old"]   = boost::lexical_cast<std::string>( nsset         .tech_check_level.get_value() );
    etalon["nsset.check_level.new"]   = boost::lexical_cast<std::string>( new_nsset_data.tech_check_level.get_value() );

    struct extract {
        static std::set<std::string> handles(const std::vector<Fred::ObjectIdHandlePair>& _in) {
            std::set<std::string> result;
            BOOST_FOREACH(const Fred::ObjectIdHandlePair& elem, _in) {
                result.insert(elem.handle);
            }
            return result;
        }
    };

    etalon["nsset.tech_c"]       = "1";
    etalon["nsset.tech_c.old"]   = boost::join( extract::handles( nsset         .tech_contacts ), " " );
    etalon["nsset.tech_c.new"]   = boost::join( extract::handles( new_nsset_data.tech_contacts ), " " );

    etalon["nsset.dns"]         = "1";
    etalon["nsset.dns.old.0"]   = "host1.nic.cz (127.0.0.1 192.168.0.1)";
    etalon["nsset.dns.old.1"]   = "ns.wtf.net (4.5.6.7 123.147.159.0)";
    etalon["nsset.dns.new.0"]   = "host1.nic.cz (127.0.0.1 192.168.0.1)";
    etalon["nsset.dns.new.1"]   = "host2.nic.cz (127.0.0.100 192.168.0.100)";
    etalon["nsset.dns.new.2"]   = "ns.wtf.net (4.5.6.7 123.147.159.0)";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::nsset, Notification::updated),
                registrar.id,
                new_nsset_data.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
