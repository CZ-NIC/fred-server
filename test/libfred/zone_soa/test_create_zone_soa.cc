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
#include "src/libfred/zone_soa/create_zone_soa.hh"
#include "src/libfred/zone_soa/exceptions.hh"
#include "src/util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>


namespace Test {

struct create_zone_soa_fixture : public virtual Test::instantiate_db_template
{
    std::string fqdn;

    create_zone_soa_fixture()
        : fqdn(RandomDataGenerator().xstring(3))
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::Zone::CreateZone(fqdn, 5, 6).exec(ctx);
        ctx.commit_transaction();
    }

    ~create_zone_soa_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateZoneSoa, create_zone_soa_fixture)

size_t exists_new_zone_soa(const std::string& _fqdn)
{
    ::LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
            "SELECT COUNT(1) FROM zone AS z "
            "LEFT JOIN zone_soa AS zs ON zs.zone=z.id "
            "WHERE z.fqdn = LOWER($1::text) ",
            Database::query_param_list(_fqdn));
    return db_result.size();
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    ::LibFred::OperationContextCreator ctx;

    BOOST_CHECK_THROW(::LibFred::ZoneSoa::CreateZoneSoa(RandomDataGenerator().xstring(3))
                .exec(ctx),
           NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_existing_zone_soa)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::ZoneSoa::CreateZoneSoa(fqdn).exec(ctx);
    BOOST_CHECK_THROW(::LibFred::ZoneSoa::CreateZoneSoa(fqdn)
                .exec(ctx),
           AlreadyExistingZoneSoa);
}

BOOST_AUTO_TEST_CASE(set_min_create_zone_soa)
{
   ::LibFred::OperationContextCreator ctx;
   ::LibFred::ZoneSoa::CreateZoneSoa(fqdn).exec(ctx);
   ctx.commit_transaction();
   BOOST_CHECK_EQUAL(exists_new_zone_soa(fqdn), 1);
}

BOOST_AUTO_TEST_CASE(set_max_create_zone_soa)
{
   ::LibFred::OperationContextCreator ctx;
   ::LibFred::ZoneSoa::CreateZoneSoa(fqdn)
           .set_ttl(9000)
           .set_hostmaster("hostmaster1@localhost")
           .set_refresh(5300)
           .set_update_retr(1800)
           .set_expiry(604800)
           .set_minimum(3600)
           .set_ns_fqdn("localhost1")
           .exec(ctx);
   ctx.commit_transaction();
   BOOST_CHECK_EQUAL(exists_new_zone_soa(fqdn), 1);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test

