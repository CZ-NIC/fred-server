/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#include <string>
#include <utility>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar_diff.h"
#include "src/fredlib/registrar/info_registrar_impl.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"

#include "random_data_generator.h"

/**
 *  @file
 *  test registrar info
 */

#include <boost/test/unit_test.hpp>

#include "tests/setup/fixtures.h"


struct test_info_registrar_fixture : virtual public Test::Fixture::instantiate_db_template
{
    std::string xmark;
    std::string test_registrar_handle;

    test_info_registrar_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , test_registrar_handle(std::string("TEST-REGISTRAR-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;

        Fred::CreateRegistrar(test_registrar_handle).set_name(std::string("TEST-REGISTRAR NAME")+xmark)
            .set_name(std::string("TEST-REGISTRAR NAME")+xmark)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~test_info_registrar_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestInfoRegistrar, test_info_registrar_fixture)

/**
 * test call InfoRegistrar
*/
BOOST_AUTO_TEST_CASE(info_registrar)
{
    Fred::OperationContext ctx;
    Fred::InfoRegistrarOutput registrar_info1 = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);
    Fred::InfoRegistrarOutput registrar_info2 = Fred::InfoRegistrarByHandle(test_registrar_handle).set_lock().exec(ctx);

    BOOST_CHECK(registrar_info1 == registrar_info2);

    BOOST_MESSAGE(std::string("test registrar id: ") + boost::lexical_cast<std::string>(registrar_info1.info_registrar_data.id));

    Fred::InfoRegistrarOutput registrar_info3 = Fred::InfoRegistrarById(registrar_info1.info_registrar_data.id).exec(ctx);

    BOOST_CHECK(registrar_info1 == registrar_info3);

    std::vector<Fred::InfoRegistrarOutput> registrar_info4 = Fred::InfoRegistrarAllExceptSystem().exec(ctx);

    BOOST_CHECK(
        std::find_if(registrar_info4.begin(), registrar_info4.end(),
            boost::bind(&Fred::InfoRegistrarOutput::info_registrar_data, _1) == registrar_info1.info_registrar_data) != registrar_info4.end());

}

/**
 * test wrong handle with InfoRegistrarByHandle
 */
BOOST_AUTO_TEST_CASE(info_registrar_wrong_handle)
{
    Fred::OperationContext ctx;
    std::string bad_registrar_handle = "BAD_REGISTRAR_HANDLE";

    try
    {
        Fred::InfoRegistrarByHandle(bad_registrar_handle).exec(ctx);
        BOOST_ERROR("unreported bad_registrar_handle");
    }
    catch(const Fred::InfoRegistrarByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_MESSAGE(bad_registrar_handle);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }
}

/**
 * test wrong id with InfoRegistrarById
 */
BOOST_AUTO_TEST_CASE(info_registrar_wrong_id)
{
    Fred::OperationContext ctx;
    unsigned long long bad_registrar_id = 0;

    try
    {
        Fred::InfoRegistrarById(bad_registrar_id).exec(ctx);
        BOOST_ERROR("unreported bad_registrar_id");
    }
    catch(const Fred::InfoRegistrarById::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_id());
        BOOST_MESSAGE(bad_registrar_id);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_registrar_id() == bad_registrar_id);
    }
}


/**
 * test call InfoRegistrarDiff
*/
BOOST_AUTO_TEST_CASE(info_registrar_diff)
{
    Fred::OperationContext ctx;
    Fred::InfoRegistrarOutput registrar_info1 = Fred::InfoRegistrarByHandle(test_registrar_handle).exec(ctx);
    Fred::InfoRegistrarOutput registrar_info2 = Fred::InfoRegistrarByHandle(test_registrar_handle).set_lock().exec(ctx);

    Fred::InfoRegistrarDiff test_diff, test_empty_diff;

    //differing data
    test_diff.id = std::make_pair(1ull,2ull);
    test_diff.handle = std::make_pair(std::string("testhandle1"),std::string("testhandle2"));
    test_diff.name = std::make_pair(Nullable<std::string>(),Nullable<std::string>("testname2"));
    test_diff.organization = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.street1 = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.street2 = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.street3 = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.city = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.stateorprovince = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.postalcode = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.country = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.telephone = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.fax = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.email = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.url = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.system= std::make_pair(Nullable<bool>(),Nullable<bool>(false));
    test_diff.ico = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.dic = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.variable_symbol = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.payment_memo_regex = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.vat_payer= std::make_pair(false,true);

    BOOST_MESSAGE(test_diff.to_string());
    BOOST_MESSAGE(test_empty_diff.to_string());

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_MESSAGE(Fred::diff_registrar_data(registrar_info1.info_registrar_data,registrar_info2.info_registrar_data).to_string());
}

BOOST_AUTO_TEST_SUITE_END();//TestInfoRegistrar
