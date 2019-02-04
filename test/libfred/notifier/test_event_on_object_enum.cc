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

#include "libfred/notifier/event_on_object_enum.hh"

BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(EventOnObjectEnum)

BOOST_AUTO_TEST_CASE(renew_only_with_domain)
{
    BOOST_CHECK_NO_THROW(
        Notification::EventOnObject(::LibFred::domain, Notification::renewed)
    );

    BOOST_CHECK_THROW(
        Notification::EventOnObject(::LibFred::contact, Notification::renewed),
        Notification::ExceptionInvalidEventOnObject
    );

    BOOST_CHECK_THROW(
        Notification::EventOnObject(::LibFred::keyset, Notification::renewed),
        Notification::ExceptionInvalidEventOnObject
    );

    BOOST_CHECK_THROW(
        Notification::EventOnObject(::LibFred::nsset, Notification::renewed),
        Notification::ExceptionInvalidEventOnObject
    );

}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
