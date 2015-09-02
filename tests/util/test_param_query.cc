/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#include <boost/test/unit_test.hpp>
#include <string>
#include <stack>

#include "src/fredlib/opcontext.h"
#include "util/db/param_query_composition.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-param-query";

struct param_query_fixture : public Test::Fixture::instantiate_db_template
{
    param_query_fixture()
    {
        Fred::OperationContext ctx;
        ctx.commit_transaction();//commit fixture
    }
    ~param_query_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestParamQuery, param_query_fixture)

/**
 * test query composition
*/
BOOST_AUTO_TEST_CASE(query_composition)
{
    Fred::OperationContext ctx;

    Database::ParamQuery array_lit = Database::ParamQuery("'{")
                ("\"aaa\",")
                ("\"bbb\",")
                ("\"ccc\"")
                ("}'::text[]");

    Database::ReusableParameter p_brm("brm", "text");

    Database::ParamQuery projection_query = Database::ParamQuery
            ("select ")
            .param(1, "integer")(" as f1")
            (", ")(array_lit)(" as f2")
            (", ").param(p_brm)(" as f3")
            (", ").param(p_brm)(" as f4")
            (", ").param_text("test text")(" as f5")(", ").param(p_brm)(" as f6")
            (", ").param_bigint(10)(" as f7")(", ").param(p_brm)(" as f8");

    BOOST_CHECK(projection_query.get_query().first
        == "select $1::integer as f1, '{\"aaa\",\"bbb\",\"ccc\"}'::text[] as f2"
        ", $2::text as f3, $2::text as f4, $3::text as f5, $2::text as f6"
        ", $4::bigint as f7, $2::text as f8");

    Database::ParamQuery filter_query = Database::ParamQuery
                ("select * from (")(projection_query)(") as tmp")
                (" where f6 = ").param(p_brm)
                (" and f3 = ").param(p_brm)
                (" and f5 <> ").param(p_brm);

    BOOST_CHECK(filter_query.get_query().first ==
    "select * from (select $1::integer as f1, '{\"aaa\",\"bbb\",\"ccc\"}'::text[] as f2"
        ", $2::text as f3, $2::text as f4, $3::text as f5, $2::text as f6"
        ", $4::bigint as f7, $2::text as f8"
        ") as tmp where f6 = $2::text and f3 = $2::text and f5 <> $2::text");

    std::pair<std::string,Database::query_param_list> query = filter_query.get_query();
    Database::Result result_pq = ctx.get_conn().exec_params(query.first, query.second);

    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f1"]) == "1");
    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f2"]) == "{aaa,bbb,ccc}");
    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f3"]) == "brm");
    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f4"]) == "brm");
    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f5"]) == "test text");
    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f6"]) == "brm");
    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f7"]) == "10");
    BOOST_CHECK(static_cast<std::string>(result_pq[0]["f8"]) == "brm");

    ctx.commit_transaction();
}

/**
 * test 30k params query composition
*/
BOOST_AUTO_TEST_CASE(query_composition_30k_params)
{
    Database::ParamQuery test_rep_query("SELECT ");
    Database::ParamQuery test_non_rep_query("SELECT ");

    Database::ReusableParameter p_brm("brm", "text");

    Util::HeadSeparator in_separator1("",",");
    Util::HeadSeparator in_separator2("",",");

    Database::ReusableParameter dummy_id = Database::ReusableParameter(1,"bigint");

    for (unsigned long long i = 0 ; i < 30000; ++i)
    {
        test_rep_query(in_separator1.get()).param(dummy_id);
        test_non_rep_query(in_separator2.get()).param_bigint(1);
    }

    //BOOST_MESSAGE(test_rep_query.get_query_string());
    //BOOST_MESSAGE(test_non_rep_query.get_query_string());

    BOOST_CHECK(test_rep_query.get_query().second.size() == 1);
    BOOST_CHECK(test_non_rep_query.get_query().second.size() == 30000);



}

BOOST_AUTO_TEST_SUITE_END();//TestParamQuery
