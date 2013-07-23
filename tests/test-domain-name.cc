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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/domain/domain_name.h"
#include "util/util.h"

#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestDomainName)

const std::string server_name = "test-domain-name";

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
    Fred::OperationContext ctx;

    // basic check
    Fred::Domain::DomainNameValidator basic;
    basic.add(Fred::Domain::DNCHECK_NOT_EMPTY_DOMAIN_NAME).set_ctx(ctx).set_zone_name("zone");
    BOOST_CHECK(basic.exec("domain"));


    //RFC1035 preferred syntax check test cases
    Fred::Domain::DomainNameValidator rfc1035_preferred;
    rfc1035_preferred.add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX);
    rfc1035_preferred.set_ctx(ctx);

    std::vector<std::string> valid_pref_labels = list_of
                                            ("fred")
                                            ("f--red")
                                            ("f--red5")
                                            ("fred.fred");
    std::vector<std::string> temp_valid = valid_pref_labels;
    BOOST_FOREACH(const std::string& label, temp_valid) {
        valid_pref_labels.push_back(label + ".fred");
        valid_pref_labels.push_back(label + ".f--red5");
    }
    std::vector<std::string> invalid_pref_labels = list_of
                                                ("2fred")
                                                ("-fred")
                                                ("fred-");
    std::vector<std::string> temp_invalid = invalid_pref_labels;
    BOOST_FOREACH(const std::string& label, temp_invalid) {
        invalid_pref_labels.push_back(label + ".fred");
    }
    rfc1035_preferred.set_zone_name("cz");
    // ...launch
    BOOST_FOREACH(const std::string& label, valid_pref_labels) {
        BOOST_CHECK(rfc1035_preferred.exec(label));
    }
    BOOST_FOREACH(const std::string& label, invalid_pref_labels) {
        BOOST_CHECK(!rfc1035_preferred.exec(label));
    }

    rfc1035_preferred.set_zone_name("");
    BOOST_CHECK(rfc1035_preferred.exec("fred.com"));

    //no '--' checks
    Fred::Domain::DomainNameValidator double_hyphen;
    double_hyphen.add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS);
    double_hyphen.set_ctx(ctx);

    std::vector<std::string> valid_double_hyphen_labels = list_of
                                                    ("fred-.fred")
                                                    ("fre-d-.fred")
                                                    ("fre-d.fre-d");
    std::vector<std::string> invalid_double_hyphen_labels = list_of
                                                            ("fre-d--.fred")
                                                            ("fre-d.fre--d");
    double_hyphen.set_zone_name("cz");

    // ...launch
    BOOST_FOREACH(const std::string& label, valid_double_hyphen_labels) {
        BOOST_CHECK(double_hyphen.exec(label));
    }
    BOOST_FOREACH(const std::string& label, invalid_double_hyphen_labels) {
        BOOST_CHECK(!double_hyphen.exec(label));
    }

    //single digit labels
    Fred::Domain::DomainNameValidator single_digits;
    single_digits.add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS);
    single_digits.set_ctx(ctx);
    std::vector<std::string> valid_single_digits_labels = list_of
                                                            ("8.4.1.0.6.4.9.7.0");
    std::vector<std::string> invalid_single_digits_labels = list_of
                                                            ("8.4.10.0.6.4.9.7.0")
                                                            ("8.4.1..0.6.4.9.7.0")
                                                            ("8.4.1.0.6.4.9.7.a")
                                                            ("8.4.1.0.6.4.9.7.0.")
                                                            (".8.4.1.0.6.4.9.7.0");
    single_digits.set_zone_name("2.4.4.e164.arpa");
    // ...launch
    BOOST_FOREACH(const std::string& label, valid_single_digits_labels) {
        BOOST_CHECK(single_digits.exec(label));
    }
    BOOST_FOREACH(const std::string& label, invalid_single_digits_labels) {
        BOOST_CHECK(!single_digits.exec(label));
    }


    //general LDH rule
    Fred::Domain::DomainNameValidator ldh;
    ldh.add(Fred::Domain::DNCHECK_LETTERS_DIGITS_HYPHEN_CHARS_ONLY);
    ldh.set_ctx(ctx);

    ldh.set_zone_name("2.4.4.e164.arpa");
    BOOST_CHECK(ldh.exec("8.4.1.0.6.4.9.7.0"));
    ldh.set_zone_name("cz");
    BOOST_CHECK(ldh.exec("-fred.fred-.2fred.fred2.-Fred.Fred-.2Fred.Fred2"));
    BOOST_CHECK(ldh.exec("1a"));

    BOOST_CHECK(!ldh.exec("1~a"));
    BOOST_CHECK(!ldh.exec("fred@"));

    //combined
    Fred::Domain::DomainNameValidator combined1;
    combined1.add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX)
                .add(Fred::Domain::DNCHECK_NO_CONSECUTIVE_HYPHENS);
    combined1.set_zone_name("cz");
    combined1.set_ctx(ctx);
    BOOST_CHECK(combined1.exec("fred"));

    Fred::Domain::DomainNameValidator combined2;
    combined2.add(Fred::Domain::DNCHECK_SINGLE_DIGIT_LABELS_ONLY)
                .add(Fred::Domain::DNCHECK_RFC1035_PREFERRED_SYNTAX);
    combined2.set_zone_name("2.4.4.e164.arpa");
    combined2.set_ctx(ctx);
    BOOST_CHECK(!combined2.exec("8.4.1.0.6.4.9.7.0"));
}

BOOST_AUTO_TEST_SUITE_END();//TestDomainName
