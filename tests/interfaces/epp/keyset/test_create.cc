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

#include "tests/interfaces/epp/keyset/fixture.h"
#include "src/epp/parameter_errors.h"
#include "src/epp/keyset/create.h"
#include "src/epp/keyset/limits.h"
#include "src/fredlib/registrar/create_registrar.h"

#include <sstream>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Keyset)

namespace {

template < class T0, class T1, class T2 >
bool is_nondecreasing(T0 a0, T1 a1, T2 a2)
{
    return (a0 <= boost::numeric_cast< T0 >(a1)) && (boost::numeric_cast< T2 >(a1) <= a2);
}

std::string create_successfully(const Test::ObjectsProvider &objects_provider)
{
    try {
        Fred::OperationContextCreator ctx;
        const unsigned long long registrar_id = objects_provider.get_registrar_a().id;
        BOOST_REQUIRE(0 < registrar_id);
        const std::string keyset_handle = objects_provider.get_keyset_handle< Fred::KeySet::HandleState::available >(ctx);
        static const unsigned number_of_contacts = (1 + Epp::KeySet::max_number_of_tech_contacts) / 2;
        std::vector< std::string > tech_contacts;
        tech_contacts.reserve(number_of_contacts);
        for (unsigned idx = 0; idx < number_of_contacts; ++idx) {
            tech_contacts.push_back(objects_provider.get_contact(idx).handle);
        }
        static const unsigned long long logd_request_id = 12345;
        std::vector< Epp::KeySet::DnsKey > dns_keys;
        dns_keys.push_back(Epp::KeySet::DnsKey(0, 3, 1, "bla="));
        BOOST_CHECK(is_nondecreasing(Epp::KeySet::min_number_of_dns_keys, dns_keys.size(), Epp::KeySet::max_number_of_dns_keys));
        const Epp::KeysetCreateResult result = Epp::keyset_create(
            ctx,
            keyset_handle,
            Optional< std::string >(),
            tech_contacts,
            std::vector< Epp::KeySet::DsRecord >(),
            dns_keys,
            registrar_id,
            logd_request_id);
        BOOST_REQUIRE(0 < result.id);
        ctx.commit_transaction();
        return keyset_handle;
    }
    catch (const Epp::AuthErrorServerClosingConnection&) {
        std::cout << "catch: AuthErrorServerClosingConnection" << std::endl;
        throw;
    }
    catch (const Epp::ParameterErrors&) {
        std::cout << "catch: ParameterErrors" << std::endl;
        throw;
    }
    catch (const Fred::CreateKeyset::Exception&) {
        std::cout << "catch: Fred::CreateKeyset::Exception" << std::endl;
        throw;
    }
    catch (const std::exception &e) {
        std::cout << "catch: " << e.what() << std::endl;
        throw;
    }
    catch (...) {
        std::cout << "catch: unknown error" << std::endl;
        throw;
    }
}

}

BOOST_FIXTURE_TEST_CASE(create, Test::ObjectsProvider)
{
    const std::string handle_of_created_keyset = Keyset::create_successfully(*this);
}

BOOST_AUTO_TEST_SUITE_END();
