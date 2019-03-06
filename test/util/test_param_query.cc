/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
#include <boost/test/unit_test.hpp>
#include <string>
#include <stack>

#include "libfred/opcontext.hh"
#include "util/db/param_query_composition.hh"
#include "test/setup/fixtures.hh"

BOOST_AUTO_TEST_SUITE(TestParamQuery)

/**
 * test query composition
*/
BOOST_FIXTURE_TEST_CASE(query_composition, Test::instantiate_db_template)
{
    ::LibFred::OperationContextCreator ctx;

    Database::ParamQuery array_lit = Database::ParamQuery("'{")
                ("\"aaa\",")
                ("\"bbb\",")
                ("\"ccc\"")
                ("}'::text[]");

    const Database::ReusableParameter p_brm("brm", "text");

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

    Database::Result result_pq = ctx.get_conn().exec_params(filter_query);

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

    const Database::ReusableParameter p_brm("brm", "text");

    Util::HeadSeparator in_separator1("",",");
    Util::HeadSeparator in_separator2("",",");

    const Database::ReusableParameter dummy_id = Database::ReusableParameter(1,"bigint");

    for (unsigned long long i = 0 ; i < 30000; ++i)
    {
        test_rep_query(in_separator1.get()).param(dummy_id);
        test_non_rep_query(in_separator2.get()).param_bigint(1);
    }

    //BOOST_TEST_MESSAGE(test_rep_query.get_query_string());
    //BOOST_TEST_MESSAGE(test_non_rep_query.get_query_string());

    BOOST_CHECK(test_rep_query.get_query().second.size() == 1);
    BOOST_CHECK(test_non_rep_query.get_query().second.size() == 30000);

}


struct reusable_parameter_fixture
{
    const Database::ReusableParameter dummy_reusable_parameter_1;
    const Database::ReusableParameter dummy_reusable_parameter_2;
    Database::ReusableParameter dummy_reusable_parameter_3;
    Database::ReusableParameter dummy_reusable_parameter_4;
    Database::ReusableParameter dummy_reusable_parameter_5;
    reusable_parameter_fixture()
    : dummy_reusable_parameter_1(1,"integer")
    , dummy_reusable_parameter_2("test text","text")
    , dummy_reusable_parameter_3(Database::NullQueryParam, "text")
    , dummy_reusable_parameter_4(dummy_reusable_parameter_2)
    , dummy_reusable_parameter_5(dummy_reusable_parameter_3)
    {
        dummy_reusable_parameter_5 = dummy_reusable_parameter_1;
    }
    ~reusable_parameter_fixture()
    {}
};


BOOST_FIXTURE_TEST_CASE(test_reusable_parameter, reusable_parameter_fixture)
{
    BOOST_CHECK(dummy_reusable_parameter_1.get_lid() != dummy_reusable_parameter_2.get_lid());
    BOOST_CHECK(dummy_reusable_parameter_2.get_lid() != dummy_reusable_parameter_3.get_lid());
    BOOST_CHECK(dummy_reusable_parameter_2.get_lid() == dummy_reusable_parameter_4.get_lid());
    BOOST_CHECK(dummy_reusable_parameter_1.get_lid() == dummy_reusable_parameter_5.get_lid());
    BOOST_CHECK(dummy_reusable_parameter_2.get_lid() != dummy_reusable_parameter_5.get_lid());
    BOOST_CHECK(dummy_reusable_parameter_3.get_lid() != dummy_reusable_parameter_5.get_lid());

    BOOST_CHECK(dummy_reusable_parameter_1.get_type() == "integer");
    BOOST_CHECK(dummy_reusable_parameter_2.get_type() == "text");
    BOOST_CHECK(dummy_reusable_parameter_3.get_type() == "text");
    BOOST_CHECK(dummy_reusable_parameter_4.get_type() == "text");
    BOOST_CHECK(dummy_reusable_parameter_5.get_type() == "integer");

    BOOST_CHECK(dummy_reusable_parameter_1.get_value().to_string() == "1");
    BOOST_CHECK(dummy_reusable_parameter_2.get_value().to_string() == "test text");
    BOOST_CHECK(dummy_reusable_parameter_3.get_value().to_string() == "null");
    BOOST_CHECK(dummy_reusable_parameter_4.get_value().to_string() == "test text");
    BOOST_CHECK(dummy_reusable_parameter_5.get_value().to_string() == "1");
}

struct param_query_fixture
{
    Database::ParamQuery test_query_1;
    Database::ParamQuery test_query_2;
    Database::ParamQuery test_query_3;
    Database::ParamQuery test_query_4;
    Database::ParamQuery test_query_5;
    Database::ParamQuery test_query_6;
    Database::ParamQuery test_query_7;
    Database::ParamQuery test_query_8;
    const Database::ReusableParameter dummy_reusable_parameter_1;
    Database::ParamQuery test_query_9;
    Database::ParamQuery test_query_10;

    param_query_fixture()
    : test_query_1()
    , test_query_2("select 1")
    , test_query_3("select ")
    , test_query_4(test_query_3)
    , test_query_5(test_query_3)
    , test_query_6(test_query_3)
    , test_query_7(test_query_3)
    , test_query_8(test_query_3)
    , dummy_reusable_parameter_1(1,"integer")
    , test_query_9(test_query_3)
    , test_query_10(test_query_3)
    {
        test_query_3.param(42,"integer");

        test_query_4.param_bigint(42);

        test_query_5.param_bool(true);
        test_query_5(", ").param_bool(false);

        test_query_6.param_date("2015-01-01");
        test_query_6(", ").param_timestamp("2015-01-01 01:02:03");

        test_query_7.param_numeric("10.1");
        test_query_7(", ").param_numeric("10.2");

        test_query_8("(")(test_query_3)(") AS a, (")(test_query_3)(") AS b");

        test_query_9.param(dummy_reusable_parameter_1)(", ").param(dummy_reusable_parameter_1);

        test_query_10.param_text("test text");
    }
    ~param_query_fixture()
    {}
};

BOOST_FIXTURE_TEST_CASE(test_param_query, param_query_fixture)
{
    BOOST_CHECK(test_query_1.get_query().first.empty());
    BOOST_CHECK(test_query_1.get_query().second.empty());

    BOOST_CHECK(test_query_2.get_query().first == "select 1");
    BOOST_CHECK(test_query_2.get_query().second.empty());

    BOOST_CHECK(test_query_3.get_query().first == "select $1::integer");
    BOOST_CHECK(test_query_3.get_query().second.size() == 1);
    BOOST_CHECK(test_query_3.get_query().second.at(0).to_string() == "42");

    BOOST_CHECK(test_query_4.get_query().first == "select $1::bigint");
    BOOST_CHECK(test_query_4.get_query().second.size() == 1);
    BOOST_CHECK(test_query_4.get_query().second.at(0).to_string() == "42");

    BOOST_CHECK(test_query_5.get_query().first == "select $1::bool, $2::bool");
    BOOST_CHECK(test_query_5.get_query().second.size() == 2);
    BOOST_CHECK(test_query_5.get_query().second.at(0).to_string() == "1");
    BOOST_CHECK(test_query_5.get_query().second.at(1).to_string() == "0");

    BOOST_CHECK(test_query_6.get_query().first == "select $1::date, $2::timestamp");
    BOOST_CHECK(test_query_6.get_query().second.size() == 2);
    BOOST_CHECK(test_query_6.get_query().second.at(0).to_string() == "2015-01-01");
    BOOST_CHECK(test_query_6.get_query().second.at(1).to_string() == "2015-01-01 01:02:03");

    BOOST_CHECK(test_query_7.get_query().first == "select $1::numeric, $2::numeric");
    BOOST_CHECK(test_query_7.get_query().second.size() == 2);
    BOOST_CHECK(test_query_7.get_query().second.at(0).to_string() == "10.1");
    BOOST_CHECK(test_query_7.get_query().second.at(1).to_string() == "10.2");

    BOOST_CHECK(test_query_8.get_query().first == "select (select $1::integer) AS a, (select $2::integer) AS b");
    BOOST_CHECK(test_query_8.get_query().second.size() == 2);
    BOOST_CHECK(test_query_8.get_query().second.at(0).to_string() == "42");
    BOOST_CHECK(test_query_8.get_query().second.at(1).to_string() == "42");

    BOOST_CHECK(test_query_9.get_query().first == "select $1::integer, $1::integer");
    BOOST_CHECK(test_query_9.get_query().second.size() == 1);
    BOOST_CHECK(test_query_9.get_query().second.at(0).to_string() == "1");

    BOOST_CHECK(test_query_10.get_query().first == "select $1::text");
    BOOST_CHECK(test_query_10.get_query().second.size() == 1);
    BOOST_CHECK(test_query_10.get_query().second.at(0).to_string() == "test text");

}

BOOST_AUTO_TEST_SUITE_END();
