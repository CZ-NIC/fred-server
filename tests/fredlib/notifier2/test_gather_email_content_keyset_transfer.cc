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

BOOST_AUTO_TEST_SUITE(TestNotifier2)
BOOST_AUTO_TEST_SUITE(GatherEmailContent)
BOOST_AUTO_TEST_SUITE(Keyset)
BOOST_AUTO_TEST_SUITE(Transfer)

template<typename T_has_keyset>struct has_keyset_transferred : T_has_keyset {

    const unsigned long long logd_request_id;
    const Fred::InfoRegistrarData new_registrar;
    const unsigned long long new_historyid;

    has_keyset_transferred()
    :   logd_request_id(12345),
        new_registrar( Test::registrar(T_has_keyset::ctx).info_data ),
        new_historyid(
            Fred::UpdateKeyset(T_has_keyset::keyset.handle, T_has_keyset::registrar.handle)
                .set_sponsoring_registrar(new_registrar.handle)
                .set_logd_request_id(logd_request_id)
                .exec(T_has_keyset::ctx)
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_empty_transfer, has_keyset_transferred<has_empty_keyset>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "4";
    etalon["name"] = keyset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::keyset, Notification::transferred),
                registrar.id,
                keyset.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_full_transfer, has_keyset_transferred<has_full_keyset>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "4";
    etalon["name"] = keyset.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(Fred::keyset, Notification::transferred),
                registrar.id,
                keyset.crhistoryid,
                input_svtrid
            )
        )
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
