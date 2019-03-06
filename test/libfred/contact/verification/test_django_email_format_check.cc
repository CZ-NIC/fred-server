/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  test of django 1.6.9. email format checker
 */

#include "libfred/contact_verification/django_email_format.hh"

#include <string>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_AUTO_TEST_SUITE(TestDjangoEmailFormat)

const std::string server_name = "test-django-email-format";

/**
 testing correct returning of existing test result statuses
 @pre existing test result statuses
 @post correct rerurn values
 */
BOOST_AUTO_TEST_CASE(test_email_format)
{

    DjangoEmailFormat email;
    DjangoEmailFormat email_localdomain(Util::vector_of<std::string>("localdomain"));
    DjangoEmailFormat email_localhost(Util::vector_of<std::string>("localhost"));

    //tests data from https://github.com/django/django/blob/1.6.9/tests/validators/tests.py
    BOOST_CHECK(email.check("email@here.com"));
    BOOST_CHECK(email.check("weirder-email@here.and.there.com"));
    BOOST_CHECK(email.check("email@[127.0.0.1]"));
    BOOST_CHECK(email.check("example@valid-----hyphens.com"));
    BOOST_CHECK(email.check("example@valid-with-hyphens.com"));
    BOOST_CHECK(email.check("test@domain.with.idn.tld.उदाहरण.परीक्षा"));
    BOOST_CHECK(email_localhost.check("email@localhost"));
    BOOST_CHECK(!email.check("email@localhost"));
    BOOST_CHECK(email_localdomain.check("email@localdomain"));
    BOOST_CHECK(email.check("\"test@test\"@example.com"));

    BOOST_CHECK(!email.check(""));
    BOOST_CHECK(!email.check("abc"));
    BOOST_CHECK(!email.check("abc@"));
    BOOST_CHECK(!email.check("abc@bar"));
    BOOST_CHECK(!email.check("a @x.cz"));
    BOOST_CHECK(!email.check("abc@.com"));
    BOOST_CHECK(!email.check("something@@somewhere.com"));
    BOOST_CHECK(!email.check("email@127.0.0.1"));
    BOOST_CHECK(!email.check("example@invalid-.com"));
    BOOST_CHECK(!email.check("example@-invalid.com"));
    BOOST_CHECK(!email.check("example@invalid.com-"));
    BOOST_CHECK(!email.check("example@inv-.alid-.com"));
    BOOST_CHECK(!email.check("example@inv-.-alid.com"));
    BOOST_CHECK(!email.check("test@example.com\n\n<script src=\"x.js\">"));

    BOOST_CHECK(email.check("\"\\\011\"@here.com"));
    BOOST_CHECK(!email.check("\"\\\012\"@here.com"));
    BOOST_CHECK(!email.check("trailingdot@shouldfail.com."));

     //max label length 64
    BOOST_CHECK(!email.check("a@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        ".aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        ".aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.usa"));
    //max label length 63
    BOOST_CHECK(email.check("a@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        ".aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        ".aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.usaaaa"));

#define TEST_UTF8_EMAIL "test1@дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.ве.рф"
    BOOST_CHECK(email.check(TEST_UTF8_EMAIL));
    BOOST_CHECK(Util::get_utf8_char_len(TEST_UTF8_EMAIL)==200);
    BOOST_CHECK(std::string(TEST_UTF8_EMAIL).length() == 360);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
