/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#include "test/backend/epp/util.hh"
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/keyset/fixture.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/keyset/info_keyset.hh"

#include <boost/test/unit_test.hpp>

#include <algorithm>

namespace LibFred {

bool operator==(const DnsKey& lhs, const ::Epp::Keyset::DnsKey& rhs)
{
    return ::Epp::Keyset::DnsKey{lhs.get_flags(), lhs.get_protocol(), lhs.get_alg(), lhs.get_key()} == rhs;
}

namespace RegistrableObject {

bool operator==(const Contact::ContactReference& lhs, const std::string& rhs)
{
    return lhs.handle == rhs;
}

}//namespace LibFred::RegistrableObject
}//namespace LibFred

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Keyset)
BOOST_AUTO_TEST_SUITE(InfoKeyset)

namespace {

void check_equal(const ::Epp::Keyset::InfoKeysetOutputData& keyset_data, const ::LibFred::InfoKeysetData& info_data)
{
    BOOST_CHECK_EQUAL(keyset_data.handle, info_data.handle);

    BOOST_CHECK_EQUAL(keyset_data.ds_records.size(), 0);
    BOOST_REQUIRE_EQUAL(keyset_data.dns_keys.size(), info_data.dns_keys.size());
    std::for_each(begin(keyset_data.dns_keys), end(keyset_data.dns_keys), [&](auto&& dns_key)
    {
        BOOST_CHECK_EQUAL(std::count(begin(info_data.dns_keys), end(info_data.dns_keys), dns_key), 1);
    });

    BOOST_CHECK_EQUAL(keyset_data.tech_contacts.size(), info_data.tech_contacts.size());
    std::for_each(begin(keyset_data.tech_contacts), end(keyset_data.tech_contacts), [&](auto&& tech_contact)
    {
        BOOST_CHECK_EQUAL(std::count(begin(info_data.tech_contacts), end(info_data.tech_contacts), tech_contact), 1);
    });
}

bool fail_invalid_authinfo(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::invalid_authorization_information);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

}//namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(invalid_authinfo, supply_ctx<HasRegistrarWithSessionAndKeysetWithAuthinfo>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Keyset::info_keyset(
            ctx,
            keyset.data.handle,
            DefaultInfoKeysetConfigData(),
            ::Epp::Password{"invalid-" + *password},
            session.data
        ),
        ::Epp::EppResponseFailure,
        fail_invalid_authinfo);
}

BOOST_FIXTURE_TEST_CASE(authinfo_ok, supply_ctx<HasRegistrarWithSessionAndKeysetWithAuthinfo>)
{
    check_equal(
        ::Epp::Keyset::info_keyset(
            ctx,
            keyset.data.handle,
            DefaultInfoKeysetConfigData(),
            password,
            session.data
        ),
        keyset.data);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Keyset/InfoKeyset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Keyset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

} // namespace Test
