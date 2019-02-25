/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/whois/fixture_common.hh"

#include "src/backend/whois/zone_list.hh"
#include "libfred/db_settings.hh"
#include "util/random_data_generator.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_managed_zone_list)

struct managed_zone_list_fixture
: whois_impl_instance_fixture
{
    std::vector<std::string> fixture_zones;

    managed_zone_list_fixture()
    {
        ::LibFred::OperationContextCreator ctx;
        fixture_zones = Fred::Backend::Whois::get_managed_zone_list(ctx);
        RandomDataGenerator rdg;
        for (int i = 0; i < 5; ++i)
        {
            std::string name = rdg.xstring(3);
            ctx.get_conn().exec_params(
                "INSERT INTO zone (fqdn, ex_period_min, ex_period_max, val_period) "
                "VALUES ($1::text, 12, 120, 0) ", // XXX
                    Database::query_param_list(name));
            fixture_zones.push_back(name);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_managed_zone_list, managed_zone_list_fixture)
{
    std::vector<std::string> zone_vec = impl.get_managed_zone_list();
    BOOST_CHECK(zone_vec.size() == fixture_zones.size());
    BOOST_FOREACH(const std::string& it, zone_vec)
    {
        bool is_in_fixture = false;
        BOOST_FOREACH(const std::string& fz_it, fixture_zones)
        {
            if (fz_it == it)
            {
                is_in_fixture = true;
            }
        }
        BOOST_CHECK(is_in_fixture);
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_managed_zone_list
BOOST_AUTO_TEST_SUITE_END()//TestWhois
