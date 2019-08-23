/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/opcontext.hh"
#include "libfred/opexception.hh"
#include "libfred/zone/create_zone.hh"
#include "libfred/zone/info_zone.hh"
#include "libfred/zone/update_zone.hh"
#include "libfred/zone/exceptions.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/libfred/zone/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct update_zone_fixture
{
    ::LibFred::Zone::NonEnumZone non_enum_zone;
    ::LibFred::Zone::EnumZone enum_zone;

    update_zone_fixture(::LibFred::OperationContext& _ctx)
    {
        non_enum_zone.fqdn = Random::Generator().get_seq(Random::CharSet::letters(), 3);
        non_enum_zone.expiration_period_min_in_months = 1;
        non_enum_zone.expiration_period_max_in_months = 2;
        non_enum_zone.dots_max = 1;
        non_enum_zone.sending_warning_letter = false;

        non_enum_zone.id = ::LibFred::Zone::CreateZone(non_enum_zone.fqdn, 6, 12)
                .exec(_ctx);

        enum_zone.fqdn = "3.2.1.e164.arpa";
        enum_zone.expiration_period_min_in_months = 8;
        enum_zone.expiration_period_max_in_months = 9;
        enum_zone.validation_period_in_months = 10;
        enum_zone.dots_max = 9;
        enum_zone.sending_warning_letter = true;
        enum_zone.id = ::LibFred::Zone::CreateZone(enum_zone.fqdn, 6, 12)
                .exec(_ctx);
    }

    ~update_zone_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateZone, SupplyFixtureCtx<update_zone_fixture>)

::LibFred::Zone::InfoZoneData get_info_zone_data(::LibFred::OperationContext& _ctx, const std::string& _fqdn)
{
    return ::LibFred::Zone::InfoZone(_fqdn).exec(_ctx);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZone("someNonexistentZone")
                .set_expiration_period_min_in_months(non_enum_zone.expiration_period_min_in_months)
                .set_expiration_period_max_in_months(non_enum_zone.expiration_period_max_in_months)
                .set_sending_warning_letter(non_enum_zone.sending_warning_letter)
                .exec(ctx),
           ::LibFred::Zone::NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_zone_enum_validation_period)
{
    BOOST_CHECK_THROW(::LibFred::Zone::UpdateZone(non_enum_zone.fqdn)
                .set_enum_validation_period_in_months(3)
                .exec(ctx),
           ::LibFred::Zone::NotEnumZone);
}

BOOST_AUTO_TEST_CASE(set_zone_empty)
{
   BOOST_CHECK_THROW(::LibFred::Zone::UpdateZone(non_enum_zone.fqdn)
                .exec(ctx),
           ::LibFred::Zone::NoZoneData);
}

BOOST_AUTO_TEST_CASE(set_enum_zone_update_all)
{
   ::LibFred::Zone::UpdateZone(enum_zone.fqdn)
                .set_expiration_period_min_in_months(enum_zone.expiration_period_min_in_months)
                .set_expiration_period_max_in_months(enum_zone.expiration_period_max_in_months)
                .set_enum_validation_period_in_months(enum_zone.validation_period_in_months)
                .set_sending_warning_letter(enum_zone.sending_warning_letter)
                .exec(ctx);
   ::LibFred::Zone::InfoZoneData new_zone = get_info_zone_data(ctx, enum_zone.fqdn);

   BOOST_CHECK(enum_zone == boost::get<LibFred::Zone::EnumZone>(new_zone));
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
