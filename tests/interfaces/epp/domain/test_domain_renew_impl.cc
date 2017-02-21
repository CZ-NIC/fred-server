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

#include "src/epp/domain/domain_create_impl.h"
#include "src/epp/domain/domain_renew_impl.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/object_state_name.h"

#include "src/epp/domain/domain_renew.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DomainRenewImpl)

BOOST_FIXTURE_TEST_CASE(renew_invalid_registrar_id, HasDomainData)
{
    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_fqdn_zone, HasDomainData)
{
    (*domain2_renew_input_data).fqdn += "c";

    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::ObjectDoesNotExist
    );
}

BOOST_FIXTURE_TEST_CASE(renew_fail_registrar_zone_access, HasDomainDataAndRegistrar)
{
    BOOST_CHECK_THROW(
            Epp::domain_renew_impl(
                ctx,
                *domain2_renew_input_data,
                registrar_data_not_in_zone_.id,
                42
            ),
        Epp::ZoneAuthorizationError
    );
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_fqdn, HasDomainData)
{
    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            domain1_renew_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::ObjectDoesNotExist
    );
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_enum_fqdn, HasDomainData)
{
    (*domain2_renew_input_data).fqdn = "5.1.3.5.0.2.4.e164.arpa";

    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        ),
        Epp::ObjectDoesNotExist
    );
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_curexpdate, HasDomainData)
{
    (*domain2_renew_input_data).current_exdate = boost::gregorian::to_iso_extended_string(
    boost::gregorian::from_simple_string((*domain2_renew_input_data).current_exdate) + boost::gregorian::days(1));

    try{
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_cur_exp_date);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::curexpdate_not_expdate);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_neg_period, HasDomainData)
{
    (*domain2_renew_input_data).period = Epp::DomainRegistrationTime(-12,Epp::DomainRegistrationTime::Unit::month);

    try{
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_period);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::period_range);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_small_period, HasDomainData)
{
    (*domain2_renew_input_data).period = Epp::DomainRegistrationTime(1,Epp::DomainRegistrationTime::Unit::month);

    try{
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_period);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::period_policy);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_big_period, HasDomainData)
{
    (*domain2_renew_input_data).period = Epp::DomainRegistrationTime(132,Epp::DomainRegistrationTime::Unit::month);

    try{
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_period);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::period_range);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_modulo_period, HasDomainData)
{
    (*domain2_renew_input_data).period = Epp::DomainRegistrationTime(25,Epp::DomainRegistrationTime::Unit::month);

    try{
        Epp::domain_renew_impl(
            ctx,
            (*domain2_renew_input_data),
            info_registrar_data_.id,
            42
        );
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_period);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::period_policy);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_empty_valexdate_enum, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(10), false));

    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42
    );

    (*domain2_renew_input_data).fqdn = domain1_create_input_data.fqdn;

    BOOST_CHECK_NO_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        )
    );
}

BOOST_FIXTURE_TEST_CASE(renew_special_valexdate_enum, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(10), false));

    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42
    );

    (*domain2_renew_input_data).fqdn = domain1_create_input_data.fqdn;
    (*domain2_renew_input_data).enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension());

    try
    {
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValueRangeError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_ext_val_date);
        BOOST_CHECK(ex.get().rbegin()->position == 1);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::valexpdate_not_valid);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_yesterday_enum_valexdate, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(10), false));

    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42
    );

    (*domain2_renew_input_data).fqdn = domain1_create_input_data.fqdn;
    (*domain2_renew_input_data).enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date - boost::gregorian::days(1), false));

    try
    {
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValueRangeError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_ext_val_date);
        BOOST_CHECK(ex.get().rbegin()->position == 1);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::valexpdate_not_valid);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_today_enum_valexdate, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(10), false));

    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42
    );

    (*domain2_renew_input_data).fqdn = domain1_create_input_data.fqdn;
    (*domain2_renew_input_data).enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date, false));

    try
    {
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValueRangeError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_ext_val_date);
        BOOST_CHECK(ex.get().rbegin()->position == 1);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::valexpdate_not_valid);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_tomorrow_enum_valexdate, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(10), false));

    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42
    );

    (*domain2_renew_input_data).fqdn = domain1_create_input_data.fqdn;
    (*domain2_renew_input_data).enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
        Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(1), false));

    BOOST_CHECK_NO_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        ));
}

BOOST_FIXTURE_TEST_CASE(renew_max_enum_valexdate, HasDomainData)
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

    domain1_create_input_data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(10), false));

    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42
    );

    (*domain2_renew_input_data).fqdn = domain1_create_input_data.fqdn;
    (*domain2_renew_input_data).enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
        Epp::ENUMValidationExtension(max_valexdate_renew, false));

    BOOST_CHECK_NO_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        ));
}

BOOST_FIXTURE_TEST_CASE(renew_long_enum_valexdate, HasDomainData)
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

    domain1_create_input_data.fqdn = std::string("1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::days(10), false));

    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42
    );

    (*domain2_renew_input_data).fqdn = domain1_create_input_data.fqdn;
    (*domain2_renew_input_data).enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
        Epp::ENUMValidationExtension(max_valexdate_renew + boost::gregorian::days(1), false));

    try
    {
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValueRangeError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_ext_val_date);
        BOOST_CHECK(ex.get().rbegin()->position == 1);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::valexpdate_not_valid);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}


BOOST_FIXTURE_TEST_CASE(renew_nonempty_valexdate_nonenum, HasDomainData)
{
    (*domain2_renew_input_data).enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
        Epp::ENUMValidationExtension());

    try{
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42
        );
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_ext_val_date);
        BOOST_CHECK(ex.get().rbegin()->position == 1);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::valexpdate_not_used);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(renew_status_prohibited, HasDomainData)
{
    Database::Result res_id = ctx.get_conn().exec_params(
        "INSERT INTO object_state_request (object_id, state_id)"
        "  VALUES ("
        "    (SELECT id FROM object_registry WHERE name = $1::text),"
        "    (SELECT id FROM enum_object_states WHERE name = $2::text))"
        "  RETURNING object_id",
        Database::query_param_list
            ((*domain2_renew_input_data).fqdn)
            (Fred::ObjectState::SERVER_RENEW_PROHIBITED));
    Fred::PerformObjectStateRequest(static_cast<unsigned long long>(res_id[0][0])).exec(ctx);

    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42)
            , Epp::ObjectStatusProhibitsOperation
        );
}

BOOST_FIXTURE_TEST_CASE(renew_status_delete, HasDomainData)
{
    Database::Result res_id = ctx.get_conn().exec_params(
        "INSERT INTO object_state_request (object_id, state_id)"
        "  VALUES ("
        "    (SELECT id FROM object_registry WHERE name = $1::text),"
        "    (SELECT id FROM enum_object_states WHERE name = $2::text))"
        "  RETURNING object_id",
        Database::query_param_list
            ((*domain2_renew_input_data).fqdn)
            (Fred::ObjectState::DELETE_CANDIDATE));
    Fred::PerformObjectStateRequest(static_cast<unsigned long long>(res_id[0][0])).exec(ctx);

    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            *domain2_renew_input_data,
            info_registrar_data_.id,
            42)
            , Epp::ObjectStatusProhibitsOperation
        );
}


BOOST_FIXTURE_TEST_CASE(renew_ok, HasDomainData)
{
    Epp::DomainRenewResult renew_result =
    Epp::domain_renew_impl(
        ctx,
        *domain2_renew_input_data,
        info_registrar_data_.id,
        42
    );

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle((*domain2_renew_input_data).fqdn).exec(ctx, "UTC").info_domain_data;

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
