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

#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/domain/create_domain.h"
#include "src/epp/domain/create_domain_localized.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/fredlib/object/generate_authinfo_password.h"
#include "src/fredlib/object/object_id_handle_pair.h"

#include <vector>
#include <string>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(CreateDomain)

bool handle_oidhpair_predicate (const std::string& handle, const Fred::ObjectIdHandlePair& pair)
{
  return (handle == pair.handle);
}

bool create_invalid_registrar_id_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, HasDomainData)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            0,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_registrar_id_exception
    );
}

bool create_invalid_fqdn_zone_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::not_applicable_domain);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_fqdn_zone, HasDomainData)
{
    domain1_create_input_data.fqdn = domain1_create_input_data.fqdn + "c";

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_fqdn_zone_exception
    );
}

bool create_invalid_cz_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::not_applicable_domain);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_cz, HasDomainData)
{
    domain1_create_input_data.fqdn = "cz";

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_cz_exception
    );
}

bool create_invalid_dot_cz_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::not_applicable_domain);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_dot_cz, HasDomainData)
{
    domain1_create_input_data.fqdn = std::string(".cz");

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.fqdn ") << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_dot_cz_exception
    );
}

bool fqdn_syntax_2dot_cz_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::bad_format_fqdn);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_2dot_cz, HasDomainData)
{
    domain1_create_input_data.fqdn = "..cz";

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        fqdn_syntax_2dot_cz_exception
    );
}

bool fqdn_syntax_front_hyphen_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::bad_format_fqdn);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_front_hyphen, HasDomainData)
{
    domain1_create_input_data.fqdn = "-" + domain1_create_input_data.fqdn;

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        fqdn_syntax_front_hyphen_exception
    );
}

bool fqdn_syntax_front_dot_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::bad_format_fqdn);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_front_dot, HasDomainData)
{
    domain1_create_input_data.fqdn = "." + domain1_create_input_data.fqdn;

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        fqdn_syntax_front_dot_exception
    );
}

bool fqdn_syntax_double_dot_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::bad_format_fqdn);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_double_dot, HasDomainData)
{
    domain1_create_input_data.fqdn = "testdomain1..cz";

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        fqdn_syntax_double_dot_exception
    );
}

bool fqdn_syntax_no_dot_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::not_applicable_domain);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_no_dot, HasDomainData)
{
    domain1_create_input_data.fqdn = "testdomain1";

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        fqdn_syntax_no_dot_exception
    );
}

bool create_fail_already_existing_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_exists);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, HasDomainData)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain2_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        ),
        Epp::EppResponseFailure,
        create_fail_already_existing_exception
    );
}

bool create_fqdn_blacklisted_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::blacklisted_domain);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fqdn_blacklisted, HasDomainData)
{
    ctx.get_conn().exec_params(
        "INSERT INTO domain_blacklist (regexp,reason,valid_from,valid_to)"
        " VALUES ($1::text,$2::text,CURRENT_TIMESTAMP,NULL)",
            Database::query_param_list(domain1_create_input_data.fqdn)("test"));

    BOOST_TEST_MESSAGE("domain1_create_input_data.fqdn " << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_fqdn_blacklisted_exception
    );
}

bool create_fail_registrar_zone_access_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_registrar_zone_access, HasDomainDataAndRegistrar)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            registrar_data_not_in_zone_.id,
            42 /* TODO */
        ),
        Epp::EppResponseFailure,
        create_fail_registrar_zone_access_exception
    );
}

bool create_invalid_nsset_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_nsset);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::nsset_notexist);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_nsset, HasDomainData)
{
    domain1_create_input_data.nsset = domain1_create_input_data.nsset + "NONEXISTING";

    BOOST_TEST_MESSAGE("domain1_create_input_data.nsset " << domain1_create_input_data.nsset);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_nsset_exception
    );
}

bool create_invalid_keyset_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_keyset);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::keyset_notexist);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_keyset, HasDomainData)
{
    domain1_create_input_data.keyset = domain1_create_input_data.keyset + "NONEXISTING";

    BOOST_TEST_MESSAGE("domain1_create_input_data.keyset " << domain1_create_input_data.keyset);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_keyset_exception
    );
}

bool create_empty_registrant_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_registrant);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::registrant_notexist);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_empty_registrant, HasDomainData)
{
    domain1_create_input_data.registrant = "";

    BOOST_TEST_MESSAGE("domain1_create_input_data.registrant " << domain1_create_input_data.registrant);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_empty_registrant_exception
    );
}

bool create_invalid_registrant_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_registrant);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::registrant_notexist);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrant, HasDomainData)
{
    domain1_create_input_data.registrant = domain1_create_input_data.registrant + "NONEXISTING";

    BOOST_TEST_MESSAGE("domain1_create_input_data.registrant " << domain1_create_input_data.registrant);
    BOOST_TEST_MESSAGE("info_registrar_data_.id " << info_registrar_data_.id);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_registrant_exception
    );
}

bool create_invalid_period_negative_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::period_range);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_negative, HasDomainData)
{
    domain1_create_input_data.period = Epp::Domain::DomainRegistrationTime(-12,Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_period_negative_exception
    );
}

bool create_invalid_period_toolong_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::period_range);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_toolong, HasDomainData)
{
    domain1_create_input_data.period = Epp::Domain::DomainRegistrationTime(132,Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_period_toolong_exception
    );
}

bool create_invalid_period_modulo_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::period_policy);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_modulo, HasDomainData)
{
    domain1_create_input_data.period = Epp::Domain::DomainRegistrationTime(25,Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_period_modulo_exception
    );
}

bool create_empty_valexdate_enum_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::required_parameter_missing);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_empty_valexdate_enum, HasDomainData)
{
    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = std::vector<Epp::Domain::EnumValidationExtension>();

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        ),
        Epp::EppResponseFailure,
        create_empty_valexdate_enum_exception
    );
}

bool create_special_valexdate_enum_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_special_valexdate_enum, HasDomainData)
{
    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::Domain::EnumValidationExtension>(
            Epp::Domain::EnumValidationExtension());

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        ),
        Epp::EppResponseFailure,
        create_special_valexdate_enum_exception
    );
}

bool create_nonempty_valexdate_nonenum_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::valexpdate_not_used);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_nonempty_valexdate_nonenum, HasDomainData)
{
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::Domain::EnumValidationExtension>(
            Epp::Domain::EnumValidationExtension());

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_nonempty_valexdate_nonenum_exception
    );
}

bool create_enum_valexdate_today_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_today, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::Domain::EnumValidationExtension>(
            Epp::Domain::EnumValidationExtension(current_local_date, false));//yesterday

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_enum_valexdate_today_exception
    );
}

bool create_enum_valexdate_yesterday_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_yesterday, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::Domain::EnumValidationExtension>(
            Epp::Domain::EnumValidationExtension(current_local_date - boost::gregorian::days(1), false));//yesterday

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_enum_valexdate_yesterday_exception
    );
}

bool create_enum_valexdate_7m_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_7m, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::Domain::EnumValidationExtension>(
            Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::months(7), false));//7 months

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_enum_valexdate_7m_exception
    );
}

bool create_nonexistent_admin_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_admin);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 3);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::admin_notexist);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_nonexistent_admin, HasDomainData)
{
    domain1_create_input_data.admin_contacts.push_back(*domain1_create_input_data.admin_contacts.rbegin() + "NONEXISTING");

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_nonexistent_admin_exception
    );
}

bool create_duplicated_admin_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_admin);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 3);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::duplicated_contact);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_duplicated_admin, HasDomainData)
{
    domain1_create_input_data.admin_contacts.push_back(*domain1_create_input_data.admin_contacts.rbegin());

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_duplicated_admin_exception
    );
}

BOOST_FIXTURE_TEST_CASE(create_empty_authinfopw, HasDomainData)
{
    domain1_create_input_data.authinfopw = boost::optional<std::string>("");

    Epp::Domain::create_domain(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42 /* TODO */
    );

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(domain1_create_input_data.fqdn).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //warning: timestamp conversion using local system timezone
    const boost::gregorian::date current_local_date = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
        boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec(
            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]))).date();

    const boost::gregorian::date expected_expiration_date_local = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params("select ($1::date + '1 year'::interval)::date",
                Database::query_param_list(current_local_date))[0][0]));

    BOOST_CHECK(info_data.fqdn == domain1_create_input_data.fqdn);
    BOOST_CHECK(info_data.registrant.handle == domain1_create_input_data.registrant);
    BOOST_CHECK(info_data.nsset.get_value().handle == domain1_create_input_data.nsset);
    BOOST_CHECK(info_data.keyset.get_value().handle == domain1_create_input_data.keyset);
    BOOST_CHECK(info_data.authinfopw.length() == 8);
    BOOST_CHECK(info_data.authinfopw.find_first_not_of(Fred::get_chars_allowed_in_generated_authinfopw()) == std::string::npos);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_local);

    BOOST_TEST_MESSAGE("info_data.admin_contacts.size(): "<< info_data.admin_contacts.size());

    BOOST_TEST_MESSAGE("domain1_create_input_data.admin_contacts " + domain1_create_input_data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE("domain1_create_input_data.admin_contacts " + domain1_create_input_data.admin_contacts.at(1));

    BOOST_CHECK(info_data.admin_contacts.size() == domain1_create_input_data.admin_contacts.size());
    BOOST_CHECK(std::equal (domain1_create_input_data.admin_contacts.begin(), domain1_create_input_data.admin_contacts.end(),
            info_data.admin_contacts.begin(), handle_oidhpair_predicate));

    BOOST_CHECK(info_data.enum_domain_validation.isnull());
}

BOOST_FIXTURE_TEST_CASE(create_authinfopw_not_set, HasDomainData)
{
    domain1_create_input_data.authinfopw = boost::optional<std::string>();

    Epp::Domain::create_domain(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42 /* TODO */
    );

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(domain1_create_input_data.fqdn).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //warning: timestamp conversion using local system timezone
    const boost::gregorian::date current_local_date = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
        boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec(
            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]))).date();

    const boost::gregorian::date expected_expiration_date_local = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params("select ($1::date + '1 year'::interval)::date",
                Database::query_param_list(current_local_date))[0][0]));

    BOOST_CHECK(info_data.fqdn == domain1_create_input_data.fqdn);
    BOOST_CHECK(info_data.registrant.handle == domain1_create_input_data.registrant);
    BOOST_CHECK(info_data.nsset.get_value().handle == domain1_create_input_data.nsset);
    BOOST_CHECK(info_data.keyset.get_value().handle == domain1_create_input_data.keyset);
    BOOST_CHECK(info_data.authinfopw.length() == 8);
    BOOST_CHECK(info_data.authinfopw.find_first_not_of(Fred::get_chars_allowed_in_generated_authinfopw()) == std::string::npos);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_local);

    BOOST_TEST_MESSAGE("info_data.admin_contacts.size(): "<< info_data.admin_contacts.size());

    BOOST_TEST_MESSAGE("domain1_create_input_data.admin_contacts " + domain1_create_input_data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE("domain1_create_input_data.admin_contacts " + domain1_create_input_data.admin_contacts.at(1));

    BOOST_CHECK(info_data.admin_contacts.size() == domain1_create_input_data.admin_contacts.size());
    BOOST_CHECK(std::equal (domain1_create_input_data.admin_contacts.begin(), domain1_create_input_data.admin_contacts.end(),
            info_data.admin_contacts.begin(), handle_oidhpair_predicate));

    BOOST_CHECK(info_data.enum_domain_validation.isnull());
}

BOOST_FIXTURE_TEST_CASE(create_invalid_domain_by_system_registrar_success, HasDomainDataAndSystemRegistrar)
{
    domain1_create_input_data.fqdn = std::string("xn--j--ra-xqa.cz"); // já--ra.cz

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.fqdn ") << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << system_registrar_data_.id);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.system ") << system_registrar_data_.system.get_value_or(false));

    Epp::Domain::create_domain(
        ctx,
        domain1_create_input_data,
        system_registrar_data_.id,
        42
    );
}

bool create_invalid_domain_by_system_registrar_fail_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::bad_format_fqdn);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_domain_by_system_registrar_fail, HasDomainDataAndSystemRegistrar)
{
    domain1_create_input_data.fqdn = "-" + domain1_create_input_data.fqdn;

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.fqdn ") << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << system_registrar_data_.id);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.system ") << system_registrar_data_.system.get_value_or(false));

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            system_registrar_data_.id,
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_domain_by_system_registrar_fail_exception
    );
}

BOOST_FIXTURE_TEST_CASE(create_ok, HasDomainData)
{
    BOOST_CHECK_NO_THROW(
        Epp::Domain::create_domain(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        );
    )

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(domain1_create_input_data.fqdn).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());


    //warning: timestamp conversion using local system timezone
    const boost::gregorian::date current_local_date = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
        boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec(
            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]))).date();

    const boost::gregorian::date expected_expiration_date_local = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params("select ($1::date + '1 year'::interval)::date",
                Database::query_param_list(current_local_date))[0][0]));

    BOOST_CHECK(info_data.fqdn == domain1_create_input_data.fqdn);
    BOOST_CHECK(info_data.registrant.handle == domain1_create_input_data.registrant);
    BOOST_CHECK(info_data.nsset.get_value().handle == domain1_create_input_data.nsset);
    BOOST_CHECK(info_data.keyset.get_value().handle == domain1_create_input_data.keyset);
    BOOST_CHECK(info_data.authinfopw == domain1_create_input_data.authinfopw);
    BOOST_TEST_MESSAGE("info_data.expiration_date: " << info_data.expiration_date << " expected_expiration_date_local: " << expected_expiration_date_local);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_local);

    BOOST_TEST_MESSAGE("info_data.admin_contacts.size(): "<< info_data.admin_contacts.size());

    BOOST_TEST_MESSAGE("domain1_create_input_data.admin_contacts " + domain1_create_input_data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE("domain1_create_input_data.admin_contacts " + domain1_create_input_data.admin_contacts.at(1));

    BOOST_CHECK(info_data.admin_contacts.size() == domain1_create_input_data.admin_contacts.size());
    BOOST_CHECK(std::equal (domain1_create_input_data.admin_contacts.begin(), domain1_create_input_data.admin_contacts.end(),
            info_data.admin_contacts.begin(), handle_oidhpair_predicate));

    BOOST_CHECK(info_data.enum_domain_validation.isnull());

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
