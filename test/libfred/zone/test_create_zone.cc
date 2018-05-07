/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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
#include "src/libfred/opcontext.hh"
#include "src/libfred/opexception.hh"
#include "src/libfred/zone/create_zone.hh"
#include "src/libfred/zone/utils.hh"
#include "src/util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

const std::string server_name = "test-create-zone";

struct create_zone_fixture : public virtual Test::instantiate_db_template
{
    std::string fqdn;
    boost::gregorian::months ex_period_min;
    boost::gregorian::months ex_period_max;

    create_zone_fixture()
        : fqdn(RandomDataGenerator().xstring(3)),
          ex_period_min(6),
          ex_period_max(12)
    {}
    ~create_zone_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateZone, create_zone_fixture)

size_t exists_new_zone(const std::string& _fqdn)
{
    ::LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
        "SELECT 1 FROM zone AS z "
            "WHERE z.fqdn = $1::varchar ",
        Database::query_param_list(_fqdn));
    return db_result.size();
}


BOOST_AUTO_TEST_CASE(set_enum_val_period)
{
    ::LibFred::OperationContextCreator ctx;

    BOOST_CHECK_THROW(::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max)
                .set_enum_validation_period(boost::gregorian::months(5))
                .exec(ctx),
           ::LibFred::Zone::NotEnumZone);
}

BOOST_AUTO_TEST_CASE(set_min_create_zone)
{
   ::LibFred::OperationContextCreator ctx;
   ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max).exec(ctx);
   ctx.commit_transaction();
   BOOST_CHECK_EQUAL(exists_new_zone(fqdn), 1);
}

BOOST_AUTO_TEST_CASE(set_max_create_zone)
{
   ::LibFred::OperationContextCreator ctx;
   ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max)
           .set_sending_warning_letter(true)
           .exec(ctx);
   ctx.commit_transaction();
   BOOST_CHECK_EQUAL(exists_new_zone(fqdn), 1);
}

BOOST_AUTO_TEST_CASE(set_min_create_enum_zone)
{
    const std::string fqdn = "0.3.4.e164.arpa";
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max).exec(ctx);
    ctx.commit_transaction();
    BOOST_CHECK_EQUAL(exists_new_zone(fqdn), 1);
}

BOOST_AUTO_TEST_CASE(set_max_create_enum_zone)
{
    const std::string fqdn = "1.2.e164.arpa";
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::Zone::CreateZone(fqdn, ex_period_min, ex_period_max)
            .set_enum_validation_period(boost::gregorian::months(5))
            .set_sending_warning_letter(false)
            .exec(ctx);
    ctx.commit_transaction();
    BOOST_CHECK_EQUAL(exists_new_zone(fqdn), 1);

}

BOOST_AUTO_TEST_SUITE_END();//TestCreateZone

} // namespace Test
