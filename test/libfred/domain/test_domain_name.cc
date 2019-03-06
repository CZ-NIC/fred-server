/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/create_domain_name_blacklist_id.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_FIXTURE_TEST_SUITE(TestDomainName, Test::instantiate_db_template)

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
        ::LibFred::Domain::DomainName temp(NULL);
    } catch(::LibFred::Domain::ExceptionInvalidFqdn&) {
        exception_thrown_from_DomainName_ctor_when_NULL_ptr_given = true;
    } catch (...) {
        BOOST_FAIL( "Unexpected exception type after DomainName::DomainName(NULL) called" );
    }
    BOOST_CHECK( exception_thrown_from_DomainName_ctor_when_NULL_ptr_given );

    bool exception_thrown_from_DomainName_ctor_when_0_given = false;
    try {
        ::LibFred::Domain::DomainName temp(0);
    } catch(::LibFred::Domain::ExceptionInvalidFqdn&) {
        exception_thrown_from_DomainName_ctor_when_0_given = true;
    } catch (...) {
        BOOST_FAIL( "Unexpected exception type after DomainName::DomainName(0) called" );
    }
    BOOST_CHECK( exception_thrown_from_DomainName_ctor_when_0_given );


    /// get_subdomains
    ::LibFred::Domain::DomainName temp2("relative.zone1.zone2");

    try {
        temp2.get_subdomains(2);
    } catch (...) {
        BOOST_FAIL( "Unexpected exception after DomainName::get_subdomains(valid int) called" );
    }


    bool exception_thrown_from_get_subdomains_when_to_high_int_given = false;
    try {
        temp2.get_subdomains(4);
    } catch(::LibFred::Domain::ExceptionInvalidLabelCount&) {
        exception_thrown_from_get_subdomains_when_to_high_int_given = true;
    } catch (...) {
        BOOST_FAIL( "Unexpected exception type after DomainName::get_subdomains(too big int) called" );
    }
    BOOST_CHECK( exception_thrown_from_get_subdomains_when_to_high_int_given );

    bool exception_thrown_from_get_subdomains_when_negative_input_given = false;
    try {
        temp2.get_subdomains(-1);
    } catch(::LibFred::Domain::ExceptionInvalidLabelCount&) {
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

    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("8.4.1.0.6.4.9.7.0.2.4.4.e164.arpa"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("fred.cz"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("fred.cz."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("fr--ed.cz."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("xn--jra-ela.cz"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("fred.com"));

    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("127.0.0.1"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2fred.com"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2-fred.com"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2--fred.com"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.com"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.c-om"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m"));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("fred.com."));

    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("127.0.0.1."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2fred.com."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2-fred.com."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2--fred.com."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.com."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.c-om."));
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m."));

    // maximal domain name length if final dot ommited
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890"
    ));

    // maximal domain name length if final dot present
    BOOST_CHECK(::LibFred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890."
    ));

    // *invalid names*

    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(""));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("."));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(".cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(".fred.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred..cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred.cz.."));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("-fred.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred-.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred.-cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred.cz-"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("-fred.fred.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred-.fred.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fr ed.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fr@ed.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fr\\ed.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fr\red.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fr\ned.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fr\ted.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fr;ed.cz"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("Donald\\032E\\.\\032Eastlake\\0323rd.example."));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("j\xc3\xa1ra.cz"));

    // maximal label length exceeded (second level label)
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(
        "0123456789012345678901234567890123456789012345678901234567890123.cz"));

    // maximal label length exceeded (top level label, final dot ommited)
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(
        "cz.0123456789012345678901234567890123456789012345678901234567890123"));

    // maximal label length exceeded (top level label, final dot present)
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "0123456789012345678901234567890123456789012345678901234567890123."
    ));

    // maximal domain name length exceeded (if final dot ommited)
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "01234567890123456789012345678901234567890123456789012345678901"
    ));

    // maximal domain name length exceeded (if final dot present)
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name(
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "012345678901234567890123456789012345678901234567890123456789012."
        "01234567890123456789012345678901234567890123456789012345678901."
    ));

    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("-2fred.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2--fred-.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m-"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m-"));

    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("-2fred.com."));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2--fred-.com."));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.c-o-m-."));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m."));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("2---fred.-c-o-m-."));

    // genzone vulnerability, forbidden characters in NS name (RFC1035, section 5)
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred@txt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred\rtxt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred\ntxt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred\ttxt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred txt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred\"txt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred(txt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred)txt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred\\txt.com"));
    BOOST_CHECK(!LibFred::Domain::is_rfc1123_compliant_host_name("fred;txt.com"));
}

/**
 * test cases for specific domain name syntax checkers
 *
*/
BOOST_AUTO_TEST_CASE(test_domain_name_validator)
{
    typedef ::LibFred::Domain::DomainName DomainName;
    typedef ::LibFred::Domain::DomainNameValidator DomainNameValidator;
    ::LibFred::OperationContextCreator ctx;

    // basic check
    BOOST_CHECK( DomainNameValidator()
                    .add(::LibFred::Domain::DNCHECK_NOT_EMPTY_DOMAIN_NAME)
                    .set_zone_name(DomainName("zone"))
                    .set_ctx(ctx)
                    .exec(DomainName("domain.zone"), 1) );

    //RFC1035 preferred syntax check test cases
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.com"), 0) );
    BOOST_CHECK( !DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("2fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red5.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("fred.fred.com"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("2fred.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red5.fred.cz"), 1) );
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("f--red5.f--red5.cz"), 1) );

    //no '--' checks
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fre-d.fre-d.cz"), 1) );
    BOOST_CHECK( !DomainNameValidator().add(::LibFred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fre-d.fre--d.cz"), 1) );

    //single digit labels
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).exec(DomainName("8.4.1.0.6.4.9.7.4.0.2.4.e164.arpa"), 5) );
    BOOST_CHECK( !DomainNameValidator().add(::LibFred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).exec(DomainName("8.4.10.0.6.4.9.7.4.0.2.4.e164.arpa"), 5) );
    BOOST_CHECK( !DomainNameValidator().add(::LibFred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).exec(DomainName("8.4.1.0.6.4.9.7.a.0.2.4.e164.arpa"), 5) );

    //combined
    BOOST_CHECK(  DomainNameValidator().add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).add(::LibFred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS).exec(DomainName("fred.cz"), 1));
    BOOST_CHECK( !DomainNameValidator().add(::LibFred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY).add(::LibFred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX).exec(DomainName("8.4.1.0.6.4.9.7.4.0.2.4.e164.arpa"), 5));
}

BOOST_AUTO_TEST_SUITE_END();//TestDomainName
