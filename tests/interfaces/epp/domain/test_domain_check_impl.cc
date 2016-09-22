/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "src/epp/domain/domain_check_impl.h"
#include "src/fredlib/domain/domain.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DomainCheckImpl)

BOOST_FIXTURE_TEST_CASE(test_result_size_empty, HasInfoRegistrarData)
{
    BOOST_CHECK_EQUAL(
        Epp::Domain::domain_check_impl(
            ctx,
            std::set<std::string>(),
            info_registrar_data.id
        ).size(),
        0
    );
}

BOOST_FIXTURE_TEST_CASE(test_result_size_nonempty, HasInfoRegistrarData)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            ("a")
            ("b")
            ("c")
            ("d")
            ("e")
            ("a1")
            ("b1")
            ("c1")
            ("d1")
            ("e1");

    BOOST_CHECK_EQUAL(
        Epp::Domain::domain_check_impl(
            ctx,
            domain_fqdns,
            info_registrar_data.id
        ).size(),
        domain_fqdns.size()
    );
}

BOOST_FIXTURE_TEST_CASE(test_invalid_handle, HasInfoRegistrarData)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            ("")
            ("1234567890123456789012345678901234567890123456789012345678901234.cz") // <== 64 chars
            ("-domain.cz")
            ("domain-.cz")
            ("do--main.cz")
            ("!domain.cz");

    const std::map<std::string, Nullable<Epp::Domain::DomainRegistrationObstruction::Enum> > domain_check_impl_res =
        Epp::Domain::domain_check_impl(
            ctx,
            domain_fqdns,
            info_registrar_data.id
        );

    for(std::map<std::string, Nullable<Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = domain_check_impl_res.begin();
        it != domain_check_impl_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::Domain::DomainRegistrationObstruction::invalid_fqdn);
    }
}

BOOST_FIXTURE_TEST_CASE(test_nonexistent_handle, HasInfoDomainDataOfNonexistentDomain)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            (info_domain_data.fqdn);

    const std::map<std::string, Nullable<Epp::Domain::DomainRegistrationObstruction::Enum> > domain_check_impl_res =
        Epp::Domain::domain_check_impl(
            ctx,
            domain_fqdns,
            info_registrar_data.id
        );

    for(std::map<std::string, Nullable<Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = domain_check_impl_res.begin();
        it != domain_check_impl_res.end();
        ++it
    ) {
        BOOST_CHECK( it->second.isnull() );
    }
}

BOOST_FIXTURE_TEST_CASE(test_existing, HasInfoDomainData)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            (info_domain_data.fqdn);

    const std::map<std::string, Nullable<Epp::Domain::DomainRegistrationObstruction::Enum> > domain_check_impl_res =
        Epp::Domain::domain_check_impl(
            ctx,
            domain_fqdns,
            info_registrar_data.id
        );

    for(std::map<std::string, Nullable<Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = domain_check_impl_res.begin();
        it != domain_check_impl_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.get_value() == Epp::Domain::DomainRegistrationObstruction::registered);
    }
}

// TODO blacklist domain and check blacklisted
// TODO set and test flags

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
