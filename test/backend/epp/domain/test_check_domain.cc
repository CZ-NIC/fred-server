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

#include "src/backend/epp/domain/check_domain.hh"
#include "libfred/registrable_object/domain/domain.hh"
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/domain/fixture.hh"
#include "test/backend/epp/util.hh"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(CheckDomain)

BOOST_FIXTURE_TEST_CASE(test_result_size_empty, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EQUAL(
        ::Epp::Domain::check_domain(
            ctx,
            std::set<std::string>(),
            DefaultCheckDomainConfigData(),
            session.data
        ).size(),
        0
    );
}

BOOST_FIXTURE_TEST_CASE(test_result_size_nonempty, supply_ctx<HasRegistrarWithSession>)
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
            ("e1").convert_to_container<std::set<std::string> >();

    BOOST_CHECK_EQUAL(
        ::Epp::Domain::check_domain(
            ctx,
            domain_fqdns,
            DefaultCheckDomainConfigData(),
            session.data
        ).size(),
        domain_fqdns.size()
    );
}

BOOST_FIXTURE_TEST_CASE(test_invalid_handle, supply_ctx<HasRegistrarWithSession>)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            ("1234567890123456789012345678901234567890123456789012345678901234.cz") // <== 64 chars
            ("-domain.cz")
            ("domain-.cz")
            ("do--main.cz")
            ("!domain.cz").convert_to_container<std::set<std::string> >();

    const std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> > check_domain_res =
        ::Epp::Domain::check_domain(
            ctx,
            domain_fqdns,
            DefaultCheckDomainConfigData(),
            session.data
        );

    for(std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = check_domain_res.begin();
        it != check_domain_res.end();
        ++it
    ) {
        BOOST_CHECK(!it->second.isnull()); // isnull() means domain is available (not registered) and its fqdn is valid
        BOOST_CHECK(!it->second.isnull() && (it->second.get_value() == ::Epp::Domain::DomainRegistrationObstruction::invalid_fqdn));
        if(!it->second.isnull()) {
            BOOST_TEST_MESSAGE(it->first << "  DomainRegistrationObstruction: " << it->second.get_value());
        }
    }
}

BOOST_FIXTURE_TEST_CASE(test_nonexistent_handle, supply_ctx<HasRegistrarWithSessionAndNonexistentFqdn>)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            (nonexistent_fqdn.fqdn).convert_to_container<std::set<std::string> >();

    const std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> > check_domain_res =
        ::Epp::Domain::check_domain(
            ctx,
            domain_fqdns,
            DefaultCheckDomainConfigData(),
            session.data
        );

    for(std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = check_domain_res.begin();
        it != check_domain_res.end();
        ++it
    ) {
        BOOST_CHECK(it->second.isnull());
    }
}

BOOST_FIXTURE_TEST_CASE(test_existing, supply_ctx<HasSystemRegistrarWithSessionAndDomain>)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            (domain.data.fqdn).convert_to_container<std::set<std::string> >();

    const std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> > check_domain_res =
        ::Epp::Domain::check_domain(
            ctx,
            domain_fqdns,
            DefaultCheckDomainConfigData(),
            session.data
        );

    for(std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = check_domain_res.begin();
        it != check_domain_res.end();
        ++it
    ) {
        BOOST_CHECK(!it->second.isnull()); // isnull() means domain is available (not registered) and its fqdn is valid
        BOOST_CHECK(!it->second.isnull() && (it->second.get_value() == ::Epp::Domain::DomainRegistrationObstruction::registered));
    }
}

BOOST_FIXTURE_TEST_CASE(test_blacklisted, supply_ctx<HasRegistrarWithSessionAndBlacklistedFqdn>)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            (blacklisted_fqdn.fqdn).convert_to_container<std::set<std::string> >();

    const std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> > check_domain_res =
        ::Epp::Domain::check_domain(
            ctx,
            domain_fqdns,
            DefaultCheckDomainConfigData(),
            session.data
        );

    for(std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = check_domain_res.begin();
        it != check_domain_res.end();
        ++it
    ) {
        BOOST_CHECK(!it->second.isnull()); // isnull() means domain is available (not registered) and its fqdn is valid
        BOOST_CHECK(!it->second.isnull() && (it->second.get_value() == ::Epp::Domain::DomainRegistrationObstruction::blacklisted));
    }
}

BOOST_FIXTURE_TEST_CASE(test_existing_and_blacklisted, supply_ctx<HasRegistrarWithSessionAndBlacklistedDomain>)
{
    const std::set<std::string> domain_fqdns
        = boost::assign::list_of
            (blacklisted_domain.data.fqdn).convert_to_container<std::set<std::string> >();

    const std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> > check_domain_res =
        ::Epp::Domain::check_domain(
            ctx,
            domain_fqdns,
            DefaultCheckDomainConfigData(),
            session.data
        );

    for(std::map<std::string, Nullable< ::Epp::Domain::DomainRegistrationObstruction::Enum> >::const_iterator it = check_domain_res.begin();
        it != check_domain_res.end();
        ++it
    ) {
        BOOST_CHECK(!it->second.isnull()); // isnull() means domain is available (not registered) and its fqdn is valid
        BOOST_CHECK(!it->second.isnull() && (it->second.get_value() == ::Epp::Domain::DomainRegistrationObstruction::registered));
    }
}

// TODO set and test flags

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
