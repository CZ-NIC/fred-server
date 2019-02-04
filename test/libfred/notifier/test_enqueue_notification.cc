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

/**
 *  @file
 */

#include <boost/test/unit_test.hpp>
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/notifier/util.hh"

#include "libfred/notifier/enqueue_notification.hh"

BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(EnqueueNotification)

struct has_domain : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData creating_registrar;
    const ::LibFred::InfoContactData registrant;
    const ::LibFred::InfoDomainData dom;

    has_domain()
    :
        creating_registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("BIG_R")
                    .set_name("Novakovic Jan")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        registrant(
            Test::exec(
                ::LibFred::CreateContact("REGISTRANT1", creating_registrar.handle)
                    .set_email("registrant1@.nic.cz") /* <== should not be a problem because should not be used */
                    .set_notifyemail("registrant1notify@nic.cz"),
                ctx
            )
        ),
        dom(
            Test::exec(
                ::LibFred::CreateDomain("mydomain123.cz", creating_registrar.handle, registrant.handle ),
                ctx
            )
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_request_notification, has_domain)
{
    const std::string svtrid_test_value = "Test-001-svtrid";

    Notification::enqueue_notification(
        ctx,
        Notification::created,
        creating_registrar.id,
        dom.historyid,
        svtrid_test_value
    );

    const Database::Result check_res = ctx.get_conn().exec(
        "SELECT "
            "change, done_by_registrar, historyid_post_change, svtrid "
        "FROM notification_queue "
    );

    BOOST_REQUIRE_EQUAL(check_res.size(), 1);
    BOOST_CHECK_EQUAL(
        static_cast<std::string>( check_res[0]["change"] ), to_db_handle(Notification::created)
    );
    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>( check_res[0]["done_by_registrar"] ),
        creating_registrar.id
    );
    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>( check_res[0]["historyid_post_change"] ),
        dom.historyid
    );
    BOOST_CHECK_EQUAL(
        static_cast<std::string>( check_res[0]["svtrid"] ),
        svtrid_test_value
    );
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
