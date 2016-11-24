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
