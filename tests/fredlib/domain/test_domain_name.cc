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
#include <fredlib/domain.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

BOOST_FIXTURE_TEST_SUITE(TestDomainName, Test::Fixture::instantiate_db_template)

const std::string server_name = "test-domain-name";

/**
 * test cases for DomainName type
 *
*/
BOOST_AUTO_TEST_CASE(test_DomainName)
{
    /// ctor
    bool exception_thrown_from_DomainName_ctor_when_NULL_ptr_given = false;
    try {
        Fred::Domain::DomainName temp(NULL);
    } catch(Fred::Domain::ExceptionInvalidFqdn&) {
        exception_thrown_from_DomainName_ctor_when_NULL_ptr_given = true;
    } catch (...) {
        BOOST_FAIL( "Unexpected exception type after DomainName::DomainName(NULL) called" );
    }
    BOOST_CHECK( exception_thrown_from_DomainName_ctor_when_NULL_ptr_given );

    bool exception_thrown_from_DomainName_ctor_when_0_given = false;
    try {
        Fred::Domain::DomainName temp(0);
    } catch(Fred::Domain::ExceptionInvalidFqdn&) {
        exception_thrown_from_DomainName_ctor_when_0_given = true;
    } catch (...) {
        BOOST_FAIL( "Unexpected exception type after DomainName::DomainName(0) called" );
    }
    BOOST_CHECK( exception_thrown_from_DomainName_ctor_when_0_given );


    /// get_subdomains
    Fred::Domain::DomainName temp2("relative.zone1.zone2");
    Fred::Domain::DomainName temp3("dummy_valid_string");

    try {
        temp2.get_subdomains(2);
    } catch (...) {
        BOOST_FAIL( "Unexpected exception after DomainName::get_subdomains(valid int) called" );
    }


    bool exception_thrown_from_get_subdomains_when_to_high_int_given = false;
    try {
        temp2.get_subdomains(4);
    } catch(Fred::Domain::ExceptionInvalidLabelCount&) {
        exception_thrown_from_get_subdomains_when_to_high_int_given = true;
    } catch (...) {
        BOOST_FAIL( "Unexpected exception type after DomainName::get_subdomains(too big int) called" );
    }
    BOOST_CHECK( exception_thrown_from_get_subdomains_when_to_high_int_given );

    bool exception_thrown_from_get_subdomains_when_negative_input_given = false;
    try {
        temp2.get_subdomains(-1);
    } catch(Fred::Domain::ExceptionInvalidLabelCount&) {
        exception_thrown_from_get_subdomains_when_negative_input_given = true;
    } catch (...) {
        BOOST_FAIL( "Unexpected exception type after DomainName::get_subdomains(negative int) called" );
    }
    BOOST_CHECK( exception_thrown_from_get_subdomains_when_negative_input_given );
}
/**
 * test cases for general domain name syntax with '.' separator and lengths
 *
*/
BOOST_AUTO_TEST_CASE(test_general_domain_name_syntax_check)
{

    BOOST_CHECK(Fred::Domain::general_domain_name_syntax_check("8.4.1.0.6.4.9.7.0.2.4.4.e164.arpa"));
    BOOST_CHECK(Fred::Domain::general_domain_name_syntax_check("Donald\\032E\\.\\032Eastlake\\0323rd.example."));
    BOOST_CHECK(Fred::Domain::general_domain_name_syntax_check("fred.cz"));
    BOOST_CHECK(Fred::Domain::general_domain_name_syntax_check("fred.cz."));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check("fred..cz"));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check(".fred.cz"));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check("fred.cz.."));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check(
        "0123456789012345678901234567890123456789012345678901234567890123456789.cz"));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check(
        "cz.0123456789012345678901234567890123456789012345678901234567890123456789"));
    //max lengths
    BOOST_CHECK(Fred::Domain::general_domain_name_syntax_check(
        "012345678901234567890123456789012345678901234567890123456789012."//63 octets label
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890"
    ));
    BOOST_CHECK(Fred::Domain::general_domain_name_syntax_check(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890."
    ));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check(
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890123."
    ));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "01234567890123456789012345678901234567890123456789012345678901."
    ));
    BOOST_CHECK(!Fred::Domain::general_domain_name_syntax_check(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "01234567890123456789012345678901234567890123456789012345678901"
    ));
}
/**
 * test cases for specific domain name syntax checkers
 *
*/
BOOST_AUTO_TEST_CASE(test_domain_name_validator)
{
    typedef Fred::Domain::DomainName DomainName;
    typedef Fred::Domain::DomainNameValidator DomainNameValidator;
    Fred::OperationContextCreator ctx;

    // basic check
    BOOST_CHECK( DomainNameValidator()
                    .add(Fred::Domain::DNCHECK_NOT_EMPTY_DOMAIN_NAME)
                    .set_zone_name(DomainName("zone"))
                    .set_ctx(ctx)
                    .exec(DomainName("domain.zone"), 1) );

    //RFC1035 preferred syntax check test cases
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.com"), 0) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("2fred.cz"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("-fred.cz"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred-.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red5.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.fred.com"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("2fred.fred.cz"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("-fred.fred.cz"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred-.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red5.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red5.f--red5.cz"), 1) );

    //no '--' checks
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fred-.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fre-d-.fred.cz"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fre-d--.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fre-d.fre-d.cz"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fre-d.fre--d.cz"), 1) );

    //single digit labels
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).exec(DomainName("8.4.1.0.6.4.9.7.4.0.2.4.e164.arpa"), 5) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).exec(DomainName("8.4.10.0.6.4.9.7.4.0.2.4.e164.arpa"), 5) );
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).exec(DomainName("8.4.1.0.6.4.9.7.a.0.2.4.e164.arpa"), 5) );

    //general LDH rule
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_LETTERS_DIGITS_HYPHEN_CHARS_ONLY).exec(DomainName("8.4.1.0.6.4.9.7.4.0.2.4.e164.arpa"), 5));
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_LETTERS_DIGITS_HYPHEN_CHARS_ONLY).exec(DomainName("-fred.fred-.2fred.fred2.-Fred.Fred-.2Fred.Fred2.cz"), 1));
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_LETTERS_DIGITS_HYPHEN_CHARS_ONLY).exec(DomainName("1a.cz"), 1));
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_LETTERS_DIGITS_HYPHEN_CHARS_ONLY).exec(DomainName("1~a.cz"), 1));
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_LETTERS_DIGITS_HYPHEN_CHARS_ONLY).exec(DomainName("fred@.cz"), 1));

    //combined
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fred.cz"), 1));
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("8.4.1.0.6.4.9.7.4.0.2.4.e164.arpa"), 5));
}

BOOST_AUTO_TEST_SUITE_END();//TestDomainName
