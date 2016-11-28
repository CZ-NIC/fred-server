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
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/domain/domain_create.h"

#include "src/fredlib/object/object_id_handle_pair.h"


#include <vector>
#include <string>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DomainCreateImpl)

bool handle_oidhpair_predicate (const std::string& handle, const Fred::ObjectIdHandlePair& pair)
{
  return (handle == pair.handle);
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, HasDomainData)
{
    BOOST_CHECK_THROW(
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(create_invalid_fqdn_zone, HasDomainData)
{
    domain1_create_input_data.fqdn = domain1_create_input_data.fqdn + std::string("c");

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.fqdn ") << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_fqdn);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::not_applicable_domain);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(create_invalid_fqdn_syntax, HasDomainData)
{
    domain1_create_input_data.fqdn = std::string("-") + domain1_create_input_data.fqdn;

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.fqdn ") << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValueSyntaxError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValueSyntaxError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_fqdn);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::bad_format_fqdn);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, HasDomainData)
{
    BOOST_CHECK_THROW(
        Epp::domain_create_impl(
            ctx,
            domain2_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        ),
        Epp::ObjectExists
    );
}

BOOST_FIXTURE_TEST_CASE(create_fqdn_blacklisted, HasDomainData)
{
    ctx.get_conn().exec_params(
        "INSERT INTO domain_blacklist (regexp,reason,valid_from,valid_to)"
        " VALUES ($1::text,$2::text,CURRENT_TIMESTAMP,NULL)",
            Database::query_param_list(domain1_create_input_data.fqdn)("test"));

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.fqdn ") << domain1_create_input_data.fqdn);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_fqdn);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::blacklisted_domain);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(create_fail_registrar_zone_access, HasDomainDataAndRegistrar)
{
    BOOST_CHECK_THROW(
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            registrar_data_not_in_zone_.id,
            42 /* TODO */
        ),
        Epp::AuthorizationError
    );
}

BOOST_FIXTURE_TEST_CASE(create_invalid_nsset, HasDomainData)
{
    domain1_create_input_data.nsset = domain1_create_input_data.nsset + std::string("NONEXISTING");

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.nsset ") << domain1_create_input_data.nsset);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_nsset);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::nsset_notexist);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(create_invalid_keyset, HasDomainData)
{
    domain1_create_input_data.keyset = domain1_create_input_data.keyset + std::string("NONEXISTING");

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.keyset ") << domain1_create_input_data.keyset);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_keyset);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::keyset_notexist);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(create_empty_registrant, HasDomainData)
{
    domain1_create_input_data.registrant = std::string("");

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.registrant ") << domain1_create_input_data.registrant);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::RequiredSpecificParameterMissing& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_registrant);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::registrant_notexist);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrant, HasDomainData)
{
    domain1_create_input_data.registrant = domain1_create_input_data.registrant + std::string("NONEXISTING");

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.registrant ") << domain1_create_input_data.registrant);
    BOOST_TEST_MESSAGE(std::string("info_registrar_data_.id ") << info_registrar_data_.id);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValuePolicyError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValuePolicyError");
        BOOST_CHECK(ex.get().size() == 1);
        BOOST_CHECK(ex.get().rbegin()->param == Epp::Param::domain_registrant);
        BOOST_CHECK(ex.get().rbegin()->position == 0);
        BOOST_CHECK(ex.get().rbegin()->reason == Epp::Reason::registrant_notexist);
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception type");
    }
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_negative, HasDomainData)
{
    domain1_create_input_data.period = Epp::DomainRegistrationTime(-12,Epp::DomainRegistrationTime::Unit::month);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValueRangeError");
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

BOOST_FIXTURE_TEST_CASE(create_invalid_period_toolong, HasDomainData)
{
    domain1_create_input_data.period = Epp::DomainRegistrationTime(132,Epp::DomainRegistrationTime::Unit::month);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValueRangeError& ex)
    {
        BOOST_TEST_MESSAGE("Epp::ParameterValueRangeError");
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

BOOST_FIXTURE_TEST_CASE(create_invalid_period_modulo, HasDomainData)
{
    domain1_create_input_data.period = Epp::DomainRegistrationTime(25,Epp::DomainRegistrationTime::Unit::month);

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
    }
    catch(const Epp::ParameterValuePolicyError& ex)
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

BOOST_FIXTURE_TEST_CASE(create_empty_valexdate_enum, HasDomainData)
{
    domain1_create_input_data.fqdn = std::string("1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    domain1_create_input_data.enum_validation_list = std::vector<Epp::ENUMValidationExtension>();

    BOOST_CHECK_THROW(
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        ),
        Epp::RequiredParameterMissing
    );
}


BOOST_FIXTURE_TEST_CASE(create_special_valexdate_enum, HasDomainData)
{
    domain1_create_input_data.fqdn = std::string("1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension());

    BOOST_CHECK_THROW(
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        ),
        Epp::RequiredParameterMissing
    );
}

BOOST_FIXTURE_TEST_CASE(create_nonempty_valexdate_nonenum, HasDomainData)
{
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension());

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            info_registrar_data_.id,
            42
        );
        BOOST_ERROR("exception expected");
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

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_today, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = std::string("1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date, false));//yesterday

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
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

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_yesterday, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = std::string("1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date - boost::gregorian::days(1), false));//yesterday

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
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

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_7m, HasDomainData)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    domain1_create_input_data.fqdn = std::string("1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    domain1_create_input_data.enum_validation_list = Util::vector_of<Epp::ENUMValidationExtension>(
            Epp::ENUMValidationExtension(current_local_date + boost::gregorian::months(7), false));//7 months

    try{
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
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


BOOST_FIXTURE_TEST_CASE(create_ok, HasDomainData)
{
    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42 /* TODO */
    );

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(domain1_create_input_data.fqdn).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());


    const boost::gregorian::date expected_expiration_date_utc = boost::gregorian::from_simple_string(
            static_cast<std::string>(ctx.get_conn().exec("select (CURRENT_DATE + '1 year'::interval)::date")[0][0]));

    BOOST_CHECK(info_data.fqdn == domain1_create_input_data.fqdn);
    BOOST_CHECK(info_data.registrant.handle == domain1_create_input_data.registrant);
    BOOST_CHECK(info_data.nsset.get_value().handle == domain1_create_input_data.nsset);
    BOOST_CHECK(info_data.keyset.get_value().handle == domain1_create_input_data.keyset);
    BOOST_CHECK(info_data.authinfopw == domain1_create_input_data.authinfo);
    BOOST_TEST_MESSAGE(std::string("info_data.expiration_date: ") << info_data.expiration_date << std::string(" expected_expiration_date_utc: ") << expected_expiration_date_utc);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_utc);

    BOOST_TEST_MESSAGE(std::string("info_data.admin_contacts.size(): ")<< info_data.admin_contacts.size());

    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.admin_contacts ") + domain1_create_input_data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE(std::string("domain1_create_input_data.admin_contacts ") + domain1_create_input_data.admin_contacts.at(1));

    BOOST_CHECK(info_data.admin_contacts.size() == domain1_create_input_data.admin_contacts.size());
    BOOST_CHECK(std::equal (domain1_create_input_data.admin_contacts.begin(), domain1_create_input_data.admin_contacts.end(),
            info_data.admin_contacts.begin(), handle_oidhpair_predicate));


    BOOST_CHECK(info_data.enum_domain_validation.isnull());

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
