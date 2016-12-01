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

#include "src/epp/domain/domain_renew_impl.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

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
            domain2_renew_input_data.value(),
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_fqdn_zone, HasDomainData)
{
    domain2_renew_input_data.value().fqdn = domain2_renew_input_data.value().fqdn + std::string("c");

    try{
        Epp::domain_renew_impl(
            ctx,
            domain2_renew_input_data.value(),
            info_registrar_data_.id,
            42
        );
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

BOOST_FIXTURE_TEST_CASE(renew_fail_registrar_zone_access, HasDomainDataAndRegistrar)
{
    BOOST_CHECK_THROW(
            Epp::domain_renew_impl(
                ctx,
                domain2_renew_input_data.value(),
                registrar_data_not_in_zone_.id,
                42
            ),
        Epp::AuthorizationError
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

BOOST_FIXTURE_TEST_CASE(renew_ok, HasDomainData)
{
    Epp::DomainRenewResult renew_result =
    Epp::domain_renew_impl(
        ctx,
        domain2_renew_input_data.value(),
        info_registrar_data_.id,
        42
    );

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(domain2_renew_input_data.value().fqdn).exec(ctx, "UTC").info_domain_data;

    const boost::gregorian::date expected_expiration_date_utc = boost::gregorian::from_simple_string(
            static_cast<std::string>(ctx.get_conn().exec("select (CURRENT_DATE + '2 year'::interval)::date")[0][0]));

    BOOST_TEST_MESSAGE(std::string("info_data.expiration_date: ") << info_data.expiration_date << std::string(" expected_expiration_date_utc: ") << expected_expiration_date_utc);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_utc);

    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime expected_expiration_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
            boost::posix_time::ptime(expected_expiration_date_utc));
    const boost::gregorian::date expected_expiration_local_date = expected_expiration_local_time.date();

    BOOST_TEST_MESSAGE(std::string("renew_result.exdate: ") << renew_result.exdate << std::string(" expected_expiration_local_date: ") << expected_expiration_local_date);
    BOOST_CHECK(renew_result.exdate == expected_expiration_local_date);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
