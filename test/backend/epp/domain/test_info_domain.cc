/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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

#include "test/backend/epp/domain/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/domain/info_domain.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/session_lang.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
//#include <boost/chrono.hpp>

#include <set>
#include <vector>

namespace Test {

namespace {

std::set<std::string> vector_of_Fred_RegistrableObject_Contact_ContactReference_to_set_of_string(
        const std::vector<::LibFred::RegistrableObject::Contact::ContactReference>& admin_contacts)
{
    std::set<std::string> admin;
    for (const auto& object_id_handle_pair : admin_contacts)
    {
        admin.insert(object_id_handle_pair.handle);
    }
    return admin;
}

void check_equal(
    const ::Epp::Domain::InfoDomainOutputData& _info_domain_output_data,
    const ::LibFred::InfoDomainData& _info_domain_data)
{
    BOOST_CHECK_EQUAL(_info_domain_output_data.roid, _info_domain_data.roid);
    BOOST_CHECK_EQUAL(_info_domain_output_data.fqdn, _info_domain_data.fqdn);
    BOOST_CHECK_EQUAL(_info_domain_output_data.registrant, _info_domain_data.registrant.handle);
    //BOOST_CHECK_EQUAL(
    //    _info_domain_output_data.nsset.isnull() ? "" : info_domain_output_data.nsset.get_value(),
    //    _info_domain_data.nsset.isnull() ? "" : _info_domain_data.nsset.get_value().handle
    //);
    BOOST_CHECK_EQUAL(_info_domain_output_data.nsset.get_value_or_default(), _info_domain_data.nsset.get_value_or_default().handle);
    BOOST_CHECK_EQUAL(_info_domain_output_data.keyset.get_value_or_default(), _info_domain_data.keyset.get_value_or_default().handle);
    // states
    BOOST_CHECK_EQUAL(_info_domain_output_data.sponsoring_registrar_handle, _info_domain_data.sponsoring_registrar_handle);
    BOOST_CHECK_EQUAL(_info_domain_output_data.creating_registrar_handle, _info_domain_data.create_registrar_handle);
    BOOST_CHECK_EQUAL(_info_domain_output_data.last_update_registrar_handle, _info_domain_data.update_registrar_handle);

    BOOST_CHECK_EQUAL(_info_domain_output_data.crdate, _info_domain_data.creation_time);
    BOOST_CHECK_EQUAL(_info_domain_output_data.last_update, _info_domain_data.update_time);
    BOOST_CHECK_EQUAL(_info_domain_output_data.last_transfer, _info_domain_data.transfer_time);
    BOOST_CHECK_EQUAL(_info_domain_output_data.exdate, _info_domain_data.expiration_date);

    std::set<std::string> info_domain_data_admin_contacts =
            vector_of_Fred_RegistrableObject_Contact_ContactReference_to_set_of_string(_info_domain_data.admin_contacts);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        _info_domain_output_data.admin.begin(),
        _info_domain_output_data.admin.end(),
        info_domain_data_admin_contacts.begin(),
        info_domain_data_admin_contacts.end()
    );

    BOOST_CHECK_EQUAL(
       _info_domain_output_data.ext_enum_domain_validation.get_value_or(Epp::Domain::EnumValidationExtension()).get_valexdate(),
       _info_domain_data.enum_domain_validation.get_value_or_default().validation_expiration
   );

    BOOST_CHECK_EQUAL(
       _info_domain_output_data.ext_enum_domain_validation.get_value_or_default().get_publish(),
       _info_domain_data.enum_domain_validation.get_value_or_default().publish
   );

    BOOST_CHECK_EQUAL(_info_domain_output_data.tmpcontact.size(), 0);
}

} // namespace {anonymous}

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(InfoDomain)

bool fail_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Domain::info_domain(
            ctx,
            "domain.cz",
            DefaultInfoDomainConfigData(),
            ::Epp::Password{},
            session_with_unauthenticated_registrar.data
        ),
        ::Epp::EppResponseFailure,
        fail_invalid_registrar_id_exception
    );
}

bool fail_nonexistent_fqdn_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

bool fail_invalid_authinfo(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::invalid_authorization_information);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_nonexistent_fqdn,  supply_ctx<HasRegistrarWithSessionAndNonexistentFqdn>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Domain::info_domain(
            ctx,
            nonexistent_fqdn.fqdn,
            DefaultInfoDomainConfigData(),
            ::Epp::Password{},
            session.data
        ),
        ::Epp::EppResponseFailure,
        fail_nonexistent_fqdn_exception
    );
}

BOOST_FIXTURE_TEST_CASE(ok, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    check_equal(
        ::Epp::Domain::info_domain(
            ctx,
            domain.data.fqdn,
            DefaultInfoDomainConfigData(),
            ::Epp::Password{},
            session.data
        ),
        domain.data);
}

BOOST_FIXTURE_TEST_CASE(invalid_authinfo, supply_ctx<HasRegistrarWithSessionAndDomainWithAuthinfo>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Domain::info_domain(
            ctx,
            domain.data.fqdn,
            DefaultInfoDomainConfigData(),
            ::Epp::Password{"invalid-" + *password},
            session.data
        ),
        ::Epp::EppResponseFailure,
        fail_invalid_authinfo);
}

BOOST_FIXTURE_TEST_CASE(authinfo_ok, supply_ctx<HasRegistrarWithSessionAndDomainWithAuthinfo>)
{
    check_equal(
        ::Epp::Domain::info_domain(
            ctx,
            domain.data.fqdn,
            DefaultInfoDomainConfigData(),
            password,
            session.data
        ),
        domain.data);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Domain/InfoDomain
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Domain
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

} // namespace Test
