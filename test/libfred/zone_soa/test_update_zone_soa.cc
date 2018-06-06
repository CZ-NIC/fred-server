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
#include "src/libfred/zone_soa/info_zone_soa.hh"
#include "src/libfred/zone_soa/update_zone_soa.hh"
#include "src/libfred/zone_soa/exceptions.hh"
#include "src/util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>


namespace Test {

struct update_zone_soa_fixture : public virtual Test::instantiate_db_template
{
    std::string fqdn;
    ::LibFred::ZoneSoa::InfoZoneSoaData zone_soa;

    update_zone_soa_fixture()
        : fqdn(RandomDataGenerator().xstring(3))
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::Zone::CreateZone(fqdn, 5, 6).exec(ctx);
        ctx.commit_transaction();

        zone_soa.ttl = 18000;
        zone_soa.hostmaster = "hostmaster@localhost";
        zone_soa.refresh = 10600;
        zone_soa.update_retr = 3600;
        zone_soa.expiry = 1209600;
        zone_soa.minimum = 7200;
        zone_soa.ns_fqdn = "localhost";
    }

    ~update_zone_soa_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateZoneSoa, update_zone_soa_fixture)

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    ::LibFred::OperationContextCreator ctx;
    zone_soa.ttl = 9000;
    BOOST_CHECK_THROW(::LibFred::ZoneSoa::UpdateZoneSoa(RandomDataGenerator().xstring(3))
                .set_ttl(zone_soa.ttl)
                .exec(ctx),
           NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone_soa)
{
    ::LibFred::OperationContextCreator ctx;
    zone_soa.ttl = 9000;
    BOOST_CHECK_THROW(::LibFred::ZoneSoa::UpdateZoneSoa(fqdn)
                .set_ttl(zone_soa.ttl)
                .exec(ctx),
           NonExistentZoneSoa);
}

BOOST_AUTO_TEST_CASE(set_no_update_zone_soa)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(::LibFred::ZoneSoa::UpdateZoneSoa(fqdn)
                .exec(ctx),
           NoZoneSoaData);
}

BOOST_AUTO_TEST_CASE(set_min_update_zone_soa)
{
    ::LibFred::OperationContextCreator ctx;

    zone_soa.zone = ::LibFred::ZoneSoa::CreateZoneSoa(fqdn).exec(ctx);
    zone_soa.minimum = 9000;
    ::LibFred::ZoneSoa::UpdateZoneSoa(fqdn)
           .set_minimum(zone_soa.minimum)
           .exec(ctx);
    BOOST_CHECK(zone_soa == ::LibFred::ZoneSoa::InfoZoneSoa(fqdn).exec(ctx));
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_CASE(set_max_update_zone_soa)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::ZoneSoa::InfoZoneSoaData zone_soa;

    zone_soa.zone = ::LibFred::ZoneSoa::CreateZoneSoa(fqdn).exec(ctx);
    zone_soa.ttl = 9000;
    zone_soa.hostmaster = "hostmster1@localhost";
    zone_soa.refresh = 5300;
    zone_soa.update_retr = 1800;
    zone_soa.expiry = 604800;
    zone_soa.minimum = 3600;
    zone_soa.ns_fqdn = "localhost1";

    ::LibFred::ZoneSoa::UpdateZoneSoa(fqdn)
            .set_ttl(zone_soa.ttl)
            .set_hostmaster(zone_soa.hostmaster)
            .set_refresh(zone_soa.refresh)
            .set_update_retr(zone_soa.update_retr)
            .set_expiry(zone_soa.expiry)
            .set_minimum(zone_soa.minimum)
            .set_ns_fqdn(zone_soa.ns_fqdn)
            .exec(ctx);

    BOOST_CHECK(zone_soa == ::LibFred::ZoneSoa::InfoZoneSoa(fqdn).exec(ctx));
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test

