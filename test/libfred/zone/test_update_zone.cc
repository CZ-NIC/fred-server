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
#include "src/libfred/zone/info_zone.hh"
#include "src/libfred/zone/update_zone.hh"
#include "src/libfred/zone/utils.hh"
#include "src/util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

const std::string server_name = "test-update-zone";

struct update_zone_fixture : public virtual Test::instantiate_db_template
{
    ::LibFred::Zone::InfoZoneData zone;
    ::LibFred::Zone::InfoZoneData enum_zone;

    update_zone_fixture()
    {
        ::LibFred::OperationContextCreator ctx;

        zone.fqdn = RandomDataGenerator().xstring(3);
        zone.ex_period_min = 1;
        zone.ex_period_max = 2;
        zone.dots_max = 1;
        zone.enum_zone = false;
        zone.warning_letter = false;

        ::LibFred::Zone::CreateZone(zone.fqdn, 6, 12)
                .exec(ctx);

        enum_zone.fqdn = "3.2.1.e164.arpa";
        enum_zone.ex_period_min = 8;
        enum_zone.ex_period_max = 9;
        enum_zone.val_period = 10;
        enum_zone.dots_max = 9;
        enum_zone.enum_zone = true;
        enum_zone.warning_letter = true;
        ::LibFred::Zone::CreateZone(enum_zone.fqdn, 6, 12)
                .exec(ctx);
        ctx.commit_transaction();
    }
    ~update_zone_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateZone, update_zone_fixture)

::LibFred::Zone::InfoZoneData get_info_zone_data(const std::string& _fqdn)
{
    ::LibFred::OperationContextCreator ctx;
    return ::LibFred::Zone::InfoZone(_fqdn).exec(ctx);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    ::LibFred::OperationContextCreator ctx;

    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZone("someNonexistentZone")
                .set_ex_period_min(zone.ex_period_min)
                .set_ex_period_max(zone.ex_period_max)
                .set_sending_warning_letter(zone.warning_letter)
                .exec(ctx),
           ::LibFred::Zone::NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_zone_val_period)
{
    ::LibFred::OperationContextCreator ctx;

    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZone(zone.fqdn)
                .set_enum_validation_period(3)
                .exec(ctx),
           ::LibFred::Zone::NotEnumZone);
}

BOOST_AUTO_TEST_CASE(set_zone_empty)
{
   ::LibFred::OperationContextCreator ctx;
   BOOST_CHECK_THROW(::LibFred::Zone::UpdateZone(zone.fqdn)
                .exec(ctx),
           ::LibFred::Zone::NoZoneData);
}

BOOST_AUTO_TEST_CASE(set_enum_zone_update_all)
{
   ::LibFred::OperationContextCreator ctx;
   ::LibFred::Zone::UpdateZone(enum_zone.fqdn)
                .set_ex_period_min(enum_zone.ex_period_min)
                .set_ex_period_max(enum_zone.ex_period_max)
                .set_enum_validation_period(enum_zone.val_period)
                .set_sending_warning_letter(enum_zone.warning_letter)
                .exec(ctx),
   ctx.commit_transaction();
   ::LibFred::Zone::InfoZoneData new_zone = get_info_zone_data(enum_zone.fqdn);

   BOOST_CHECK(enum_zone == new_zone);
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateZone

} // namespace Test
