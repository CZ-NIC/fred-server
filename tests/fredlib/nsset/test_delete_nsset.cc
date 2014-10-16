/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

#include "src/fredlib/opcontext.h"
#include <fredlib/nsset.h>
#include <fredlib/contact.h>
#include <fredlib/domain.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-delete-nsset";


struct delete_nsset_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact_handle;
    std::string test_nsset_handle;
    std::string test_domain_fqdn;

    delete_nsset_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , test_nsset_handle ( std::string("TEST-DEL-NSSET-")+xmark+"-HANDLE")
    , test_domain_fqdn ( std::string("fred")+xmark+".cz")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        ctx.commit_transaction();
    }
    ~delete_nsset_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestDeleteNsset, delete_nsset_fixture)

/**
 * test DeleteNsset
 * create test nsset, delete test nsset, check erdate of test nsset is null
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(delete_nsset)
{
    Fred::OperationContext ctx;

    Fred::InfoNssetOutput nsset_info1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    BOOST_CHECK(nsset_info1.info_nsset_data.delete_time.isnull());

    Fred::DeleteNssetByHandle(test_nsset_handle).exec(ctx);
    ctx.commit_transaction();

    std::vector<Fred::InfoNssetOutput> nsset_history_info1 = Fred::InfoNssetHistory(
    nsset_info1.info_nsset_data.roid).exec(ctx);

    BOOST_CHECK(!nsset_history_info1.at(0).info_nsset_data.delete_time.isnull());

    Fred::InfoNssetOutput nsset_info1_with_change = nsset_info1;
    nsset_info1_with_change.info_nsset_data.delete_time = nsset_history_info1.at(0).info_nsset_data.delete_time;

    BOOST_CHECK(nsset_info1_with_change == nsset_history_info1.at(0));

    BOOST_CHECK(!nsset_history_info1.at(0).info_nsset_data.delete_time.isnull());

    BOOST_CHECK(nsset_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!nsset_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!nsset_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(nsset_history_info1.at(0).history_valid_from <= nsset_history_info1.at(0).history_valid_to.get_value());

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_nsset_handle))[0][0]));

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT o.id FROM object o JOIN object_registry oreg ON o.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_nsset_handle)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT k.id FROM nsset k JOIN object_registry oreg ON k.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_nsset_handle)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT ncm.contactid FROM nsset_contact_map ncm JOIN object_registry oreg ON ncm.nssetid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_nsset_handle)).size() == 0);

}//delete_nsset


/**
 * test DeleteNsset with wrong handle
 */

BOOST_AUTO_TEST_CASE(delete_nsset_with_wrong_handle)
{
    std::string bad_test_nsset_handle = std::string("bad")+test_nsset_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteNssetByHandle(bad_test_nsset_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteNssetByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_nsset_handle());
        BOOST_CHECK(ex.get_unknown_nsset_handle().compare(bad_test_nsset_handle) == 0);
    }
}

/**
 * test DeleteNsset linked
 */

BOOST_AUTO_TEST_CASE(delete_linked_nsset)
{
    {
        Fred::OperationContext ctx;
        //create linked object

        Fred::CreateDomain(test_domain_fqdn //const std::string& fqdn
            , registrar_handle //const std::string& registrar
            , admin_contact_handle //registrant
            ).set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_nsset(test_nsset_handle)
            .exec(ctx);

       ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteNssetByHandle(test_nsset_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteNssetByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_linked_to_nsset_handle());
        BOOST_CHECK(ex.get_object_linked_to_nsset_handle().compare(test_nsset_handle) == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestDeleteNsset
