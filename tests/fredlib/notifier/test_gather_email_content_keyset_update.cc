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
#include "tests/fredlib/notifier/util.h"
#include "tests/fredlib/notifier/fixture_data.h"

#include "src/fredlib/notifier/gather_email_data/gather_email_content.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>


BOOST_AUTO_TEST_SUITE(TestNotifier2)
BOOST_AUTO_TEST_SUITE(GatherEmailContent)
BOOST_AUTO_TEST_SUITE(Keyset)
BOOST_AUTO_TEST_SUITE(Update)

template<typename Thas_keyset> struct has_keyset_empty_update : Thas_keyset {

    const unsigned long long logd_request_id;
    const unsigned long long new_historyid;

    has_keyset_empty_update()
    :   logd_request_id(12345),
        new_historyid(
            Fred::UpdateKeyset(Thas_keyset::keyset.handle, Thas_keyset::keyset.create_registrar_handle)
                .set_logd_request_id(logd_request_id).exec(Thas_keyset::ctx)
        )
    { }
};

template<typename Thas_keyset>  struct has_keyset_big_update : Thas_keyset {
    const unsigned long long logd_request_id;
    const Fred::InfoKeysetData new_keyset_data;

    has_keyset_big_update()
    :   logd_request_id(12345),
        new_keyset_data(
            Fred::InfoKeysetHistoryByHistoryid(
                Fred::UpdateKeyset(Thas_keyset::keyset.handle, Thas_keyset::keyset.create_registrar_handle)
                    .set_sponsoring_registrar(
                        Test::exec(
                            Fred::CreateRegistrar("NEW_REGISTRAR1")
                                .set_name("Re Gistra R Jr.")
                                .set_url("registrar2.cz"),
                            Thas_keyset::ctx
                        ).handle
                    )
                    .set_authinfo(Thas_keyset::keyset.authinfopw + "X")
                    .add_dns_key( Fred::DnsKey(3, 3, 3, "key_no_3") )
                    .add_tech_contact(
                        Test::exec(
                            Fred::CreateContact("CONTACT3", Thas_keyset::registrar.handle)
                                .set_email("contact.3@nic.cz")
                                .set_notifyemail("contact.3.notify@nic.cz"),
                            Thas_keyset::ctx
                        ).handle
                    )
                    .set_logd_request_id(logd_request_id)
                    .exec(Thas_keyset::ctx)
            ).exec(Thas_keyset::ctx)
            .info_keyset_data
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_minimal_update_no_data, has_keyset_empty_update<has_empty_keyset>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "4";
    etalon["handle"] = keyset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::keyset, Notification::updated),
                registrar.id,
                new_historyid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_minimal_update_from_empty_data, has_keyset_empty_update<has_full_keyset>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "4";
    etalon["handle"] = keyset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::keyset, Notification::updated),
                registrar.id,
                new_historyid,
                input_svtrid
            )
        )
    );
}

/* TODO zkusit udelat technicky kontakt se stejnym handlem (ale jinym id) a otestovat co se stane, kdyz je u keysetu prohodim */

BOOST_FIXTURE_TEST_CASE(test_big_update_from_empty_data, has_keyset_big_update<has_empty_keyset>)
{
    const std::string input_svtrid = "xyz-987";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "4";
    etalon["handle"] = keyset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["changes.object.authinfo"]       = "1";
    etalon["changes.object.authinfo.old"]   = keyset         .authinfopw;
    etalon["changes.object.authinfo.new"]   = new_keyset_data.authinfopw;

    struct extract {
        static std::set<std::string> handles(const std::vector<Fred::ObjectIdHandlePair>& _in) {
            std::set<std::string> result;
            BOOST_FOREACH(const Fred::ObjectIdHandlePair& elem, _in) {
                result.insert(elem.handle);
            }
            return result;
        }
    };

    etalon["changes.keyset.tech_c"]       = "1";
    etalon["changes.keyset.tech_c.old"]   = boost::join( extract::handles( keyset         .tech_contacts ), " " );
    etalon["changes.keyset.tech_c.new"]   = boost::join( extract::handles( new_keyset_data.tech_contacts ), " " );

    etalon["changes.keyset.dnskey"]         = "1";
    etalon["changes.keyset.dnskey.new.0"]   = "(flags: 3 protocol: 3 algorithm: 3 key: key_no_3)";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::keyset, Notification::updated),
                registrar.id,
                new_keyset_data.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_big_update_from_full_data, has_keyset_big_update<has_full_keyset>)
{
    const std::string input_svtrid = "xyz-987";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "4";
    etalon["handle"] = keyset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["changes.object.authinfo"]       = "1";
    etalon["changes.object.authinfo.old"]   = keyset         .authinfopw;
    etalon["changes.object.authinfo.new"]   = new_keyset_data.authinfopw;

    struct extract {
        static std::set<std::string> handles(const std::vector<Fred::ObjectIdHandlePair>& _in) {
            std::set<std::string> result;
            BOOST_FOREACH(const Fred::ObjectIdHandlePair& elem, _in) {
                result.insert(elem.handle);
            }
            return result;
        }
    };

    etalon["changes.keyset.tech_c"]       = "1";
    etalon["changes.keyset.tech_c.old"]   = boost::join( extract::handles( keyset         .tech_contacts ), " " );
    etalon["changes.keyset.tech_c.new"]   = boost::join( extract::handles( new_keyset_data.tech_contacts ), " " );

    etalon["changes.keyset.dnskey"]         = "1";
    etalon["changes.keyset.dnskey.old.0"]   = "(flags: 1 protocol: 1 algorithm: 1 key: da_key!!!)";
    etalon["changes.keyset.dnskey.old.1"]   = "(flags: 2 protocol: 2 algorithm: 2 key: super_secret_key)";
    etalon["changes.keyset.dnskey.new.0"]   = "(flags: 1 protocol: 1 algorithm: 1 key: da_key!!!)";
    etalon["changes.keyset.dnskey.new.1"]   = "(flags: 3 protocol: 3 algorithm: 3 key: key_no_3)";
    etalon["changes.keyset.dnskey.new.2"]   = "(flags: 2 protocol: 2 algorithm: 2 key: super_secret_key)";


    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::keyset, Notification::updated),
                registrar.id,
                new_keyset_data.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
