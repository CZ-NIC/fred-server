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
#include "src/libfred/zone/info_zone_data.hh"
#include "src/libfred/zone/info_zone.hh"
#include "src/libfred/zone/utils.hh"
#include "src/util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

const std::string server_name = "test-info-zone";

struct info_zone_fixture : public virtual Test::instantiate_db_template
{
    ::LibFred::Zone::InfoZoneData info_zone_data;
    ::LibFred::Zone::InfoZoneData info_zone_data_enum;

    info_zone_fixture()
    {
        ::LibFred::OperationContextCreator ctx;

        info_zone_data.fqdn = RandomDataGenerator().xstring(3);
        info_zone_data.ex_period_max = boost::gregorian::months(6);
        info_zone_data.ex_period_min = boost::gregorian::months(8);
        info_zone_data.val_period = boost::gregorian::months(0);
        info_zone_data.dots_max = 1;
        info_zone_data.enum_zone = false;
        info_zone_data.warning_letter = false;

        ::LibFred::Zone::CreateZone(info_zone_data.fqdn, *(info_zone_data.ex_period_min), *(info_zone_data.ex_period_max))
                .exec(ctx);

        info_zone_data_enum.fqdn = "3.2.1.e164.arpa";
        info_zone_data_enum.ex_period_max = boost::gregorian::months(12);
        info_zone_data_enum.ex_period_min = boost::gregorian::months(24);
        info_zone_data_enum.val_period = boost::gregorian::months(4);
        info_zone_data_enum.dots_max = 9;
        info_zone_data_enum.enum_zone = true;
        info_zone_data_enum.warning_letter = true;

        ::LibFred::Zone::CreateZone(info_zone_data_enum.fqdn, *(info_zone_data_enum.ex_period_min), *(info_zone_data_enum.ex_period_max))
                .set_enum_validation_period(*(info_zone_data_enum.val_period))
                .set_sending_warning_letter(info_zone_data_enum.warning_letter)
                .exec(ctx);
        ctx.commit_transaction();
    }
    ~info_zone_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestInfoZone, info_zone_fixture)

BOOST_AUTO_TEST_CASE(set_nonexistent_zone)
{
    ::LibFred::OperationContextCreator ctx;

    std::string fqdn = RandomDataGenerator().xstring(5);
    BOOST_CHECK_THROW(::LibFred::Zone::InfoZone(fqdn)
                .exec(ctx),
           ::LibFred::Zone::NonExistentZone);
}

BOOST_AUTO_TEST_CASE(set_info_zone)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::Zone::InfoZoneData zone_info = ::LibFred::Zone::InfoZone(info_zone_data.fqdn).exec(ctx);
    BOOST_CHECK(info_zone_data == zone_info);
}

BOOST_AUTO_TEST_CASE(set_info_enum_zone)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::Zone::InfoZoneData zone_info_enum = ::LibFred::Zone::InfoZone(info_zone_data_enum.fqdn).exec(ctx);
    BOOST_CHECK(info_zone_data_enum == zone_info_enum);
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateZone

} // namespace Test
