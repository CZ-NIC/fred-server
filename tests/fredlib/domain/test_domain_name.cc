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
 * test cases for RFC1123 section 2.1 compliant host name (however, final dot '.' is optional)
 *
*/
BOOST_AUTO_TEST_CASE(test_is_rfc1123_compliant_host_name)
{

    // *valid names*

    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("8.4.1.0.6.4.9.7.0.2.4.4.e164.arpa"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("fred.cz"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("fred.cz."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("fr--ed.cz."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("xn--jra-ela.cz"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("fred.com"));

    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("127.0.0.1"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2fred.com"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2-fred.com"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2--fred.com"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2---fred.com"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2---fred.c-om"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m"));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("fred.com."));

    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("127.0.0.1."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2fred.com."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2-fred.com."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2--fred.com."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2---fred.com."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2---fred.c-om."));
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m."));

    // maximal domain name length if final dot ommited
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890"
    ));

    // maximal domain name length if final dot present
    BOOST_CHECK(Fred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890."
    ));

    // *invalid names*

    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(""));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("."));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(".cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(".fred.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred..cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred.cz.."));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("-fred.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred-.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred.-cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred.cz-"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("-fred.fred.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred-.fred.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fr ed.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fr@ed.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fr\\ed.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fr\red.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fr\ned.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fr\ted.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fr;ed.cz"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("Donald\\032E\\.\\032Eastlake\\0323rd.example."));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("jára.cz"));

    // maximal label length exceeded (second level label)
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(
        "0123456789012345678901234567890123456789012345678901234567890123.cz"));

    // maximal label length exceeded (top level label, final dot ommited)
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(
        "cz.0123456789012345678901234567890123456789012345678901234567890123"));

    // maximal label length exceeded (top level label, final dot present)
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890123."
    ));

    // maximal domain name length exceeded (if final dot ommited)
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "01234567890123456789012345678901234567890123456789012345678901"
    ));

    // maximal domain name length exceeded (if final dot present)
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "01234567890123456789012345678901234567890123456789012345678901."
    ));

    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("-2fred.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2--fred-.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m-"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m-"));

    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("-2fred.com."));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2--fred-.com."));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m-."));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m."));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m-."));

    // genzone vulnerability, forbidden characters in NS name (RFC1035, section 5)
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred@txt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred\rtxt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred\ntxt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred\ttxt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred txt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred\"txt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred(txt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred)txt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred\\txt.com"));
    BOOST_CHECK(!Fred::Domain::is_rfc1123_compliant_host_name("fred;txt.com"));
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

    //combined
    BOOST_CHECK(  DomainNameValidator().add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fred.cz"), 1));
    BOOST_CHECK( !DomainNameValidator().add(Fred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("8.4.1.0.6.4.9.7.4.0.2.4.e164.arpa"), 5));
}

BOOST_AUTO_TEST_SUITE_END();//TestDomainName
