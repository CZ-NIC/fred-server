/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/domain/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/domain/create_domain.hh"
#include "src/backend/epp/domain/renew_domain.hh"
#include "src/backend/epp/domain/renew_domain_localized.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "libfred/object/object_state.hh"
#include "libfred/object_state/perform_object_state_request.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(RenewDomain)

bool renew_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    DefaultRenewDomainInputData(),
                    DefaultRenewDomainConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            renew_invalid_registrar_id_exception);
}

bool renew_invalid_fqdn_zone_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_REQUIRE(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_fqdn_zone, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.fqdn += "c";

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_invalid_fqdn_zone_exception);
}

bool renew_fail_registrar_zone_access_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_fail_registrar_zone_access, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    Session(ctx, RegistrarNotInZone(ctx).data.id).data),
            ::Epp::EppResponseFailure,
            renew_fail_registrar_zone_access_exception);
}
bool renew_invalid_fqdn_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_fqdn, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.fqdn = InvalidFqdn().fqdn;
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_invalid_fqdn_exception);
}

bool renew_invalid_enum_fqdn_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_enum_fqdn, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.fqdn = InvalidEnumFqdn().fqdn;

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_invalid_enum_fqdn_exception);
}

bool renew_invalid_curexpdate_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_cur_exp_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::curexpdate_not_expdate);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_curexpdate, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.current_exdate = boost::gregorian::to_iso_extended_string(
    boost::gregorian::from_simple_string(renew_domain_input_data.data.current_exdate) + boost::gregorian::days(1));

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_invalid_curexpdate_exception);
}

bool renew_neg_period_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_range);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_neg_period, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(-12, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_neg_period_exception);
}

bool renew_small_period_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_policy);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_small_period, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_small_period_exception);
}

bool renew_big_period_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_range);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_big_period, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(132, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_big_period_exception);
}

bool renew_modulo_period_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_policy);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_modulo_period, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(25, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_modulo_period_exception);
}

BOOST_FIXTURE_TEST_CASE(renew_empty_valexdate_enum, supply_ctx<HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(10), false));

    ::Epp::Domain::create_domain(
        ctx,
        create_domain_input_data.data,
        DefaultCreateDomainConfigData(),
        session.data);

    renew_domain_input_data.data.fqdn = create_domain_input_data.data.fqdn;

    BOOST_CHECK_NO_THROW(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data));
}

bool renew_special_valexdate_enum_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_special_valexdate_enum, supply_ctx<HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(10), false));

    ::Epp::Domain::create_domain(
        ctx,
        create_domain_input_data.data,
        DefaultCreateDomainConfigData(),
        session.data);

    renew_domain_input_data.data.fqdn = create_domain_input_data.data.fqdn;
    renew_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension());

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_special_valexdate_enum_exception);
}

bool renew_yesterday_enum_valexdate_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_yesterday_enum_valexdate, supply_ctx<HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(10), false));

    ::Epp::Domain::create_domain(
        ctx,
        create_domain_input_data.data,
        DefaultCreateDomainConfigData(),
        session.data);

    renew_domain_input_data.data.fqdn = create_domain_input_data.data.fqdn;
    renew_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date - boost::gregorian::days(1), false));

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_yesterday_enum_valexdate_exception);
}

bool renew_today_enum_valexdate_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_today_enum_valexdate, supply_ctx<HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(10), false));

    ::Epp::Domain::create_domain(
        ctx,
        create_domain_input_data.data,
        DefaultCreateDomainConfigData(),
        session.data);

    renew_domain_input_data.data.fqdn = create_domain_input_data.data.fqdn;
    renew_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date, false));

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_today_enum_valexdate_exception);
}

BOOST_FIXTURE_TEST_CASE(renew_tomorrow_enum_valexdate, supply_ctx<HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(10), false));

    ::Epp::Domain::create_domain(
        ctx,
        create_domain_input_data.data,
        DefaultCreateDomainConfigData(),
        session.data);

    renew_domain_input_data.data.fqdn = create_domain_input_data.data.fqdn;
    renew_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
        ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(1), false));

    BOOST_CHECK_NO_THROW(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data));
}

BOOST_FIXTURE_TEST_CASE(renew_max_enum_valexdate, supply_ctx<HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    const boost::gregorian::date max_valexdate_renew = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params(
            "SELECT $1::date + '6 month'::interval",
            Database::query_param_list(current_local_date + boost::gregorian::days(10)))[0][0]));

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(10), false));

    ::Epp::Domain::create_domain(
        ctx,
        create_domain_input_data.data,
        DefaultCreateDomainConfigData(),
        session.data);

    renew_domain_input_data.data.fqdn = create_domain_input_data.data.fqdn;
    renew_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
        ::Epp::Domain::EnumValidationExtension(max_valexdate_renew, false));

    BOOST_CHECK_NO_THROW(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data));
}

bool renew_long_enum_valexdate_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_long_enum_valexdate, supply_ctx<HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    const boost::gregorian::date max_valexdate_renew = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params(
            "SELECT $1::date + '6 month'::interval",
            Database::query_param_list(current_local_date + boost::gregorian::days(10)))[0][0]));

    create_domain_input_data.data.fqdn = std::string("1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::days(10), false));

    ::Epp::Domain::create_domain(
        ctx,
        create_domain_input_data.data,
        DefaultCreateDomainConfigData(),
        session.data);

    renew_domain_input_data.data.fqdn = create_domain_input_data.data.fqdn;
    renew_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
        ::Epp::Domain::EnumValidationExtension(max_valexdate_renew + boost::gregorian::days(1), false));

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_long_enum_valexdate_exception);
}

bool renew_nonempty_valexdate_nonenum_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_used);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_nonempty_valexdate_nonenum, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    renew_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
        ::Epp::Domain::EnumValidationExtension());

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_nonempty_valexdate_nonenum_exception);
}

bool renew_status_prohibited_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_status_prohibited, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    Database::Result res_id = ctx.get_conn().exec_params(
        "INSERT INTO object_state_request (object_id, state_id)"
        "  VALUES ("
        "    (SELECT id FROM object_registry WHERE name = $1::text),"
        "    (SELECT id FROM enum_object_states WHERE name = $2::text))"
        "  RETURNING object_id",
        Database::query_param_list
            (renew_domain_input_data.data.fqdn)
            (Conversion::Enums::to_db_handle(::LibFred::Object_State::server_renew_prohibited)));
    ::LibFred::PerformObjectStateRequest(static_cast<unsigned long long>(res_id[0][0])).exec(ctx);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_status_prohibited_exception);
}

bool renew_status_delete_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(renew_status_delete, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    Database::Result res_id = ctx.get_conn().exec_params(
        "INSERT INTO object_state_request (object_id, state_id)"
        "  VALUES ("
        "    (SELECT id FROM object_registry WHERE name = $1::text),"
        "    (SELECT id FROM enum_object_states WHERE name = $2::text))"
        "  RETURNING object_id",
        Database::query_param_list
            (renew_domain_input_data.data.fqdn)
            (Conversion::Enums::to_db_handle(::LibFred::Object_State::delete_candidate)));
    ::LibFred::PerformObjectStateRequest(static_cast<unsigned long long>(res_id[0][0])).exec(ctx);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            renew_status_delete_exception);
}

BOOST_FIXTURE_TEST_CASE(renew_ok, supply_ctx<HasRegistrarWithSessionAndDomainAndRenewDomainInputData>)
{
    ::Epp::Domain::RenewDomainResult renew_result =
            ::Epp::Domain::renew_domain(
                    ctx,
                    renew_domain_input_data.data,
                    DefaultRenewDomainConfigData(),
                    session.data);

    ::LibFred::InfoDomainData info_data = ::LibFred::InfoDomainByFqdn(renew_domain_input_data.data.fqdn).exec(ctx, "UTC").info_domain_data;

    //warning: timestamp conversion using local system timezone
    const boost::gregorian::date current_local_date = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
        boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec(
            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]))).date();

    const boost::gregorian::date expected_expiration_date_local = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params("select ($1::date + '2 year'::interval)::date",
                Database::query_param_list(current_local_date))[0][0]));

    BOOST_TEST_MESSAGE(std::string("info_data.expiration_date: ") << info_data.expiration_date << std::string(" expected_expiration_date_local: ") << expected_expiration_date_local);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_local);
    BOOST_CHECK(renew_result.exdate == expected_expiration_date_local);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
