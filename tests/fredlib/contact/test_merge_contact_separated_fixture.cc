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

/**
 *  @file
 *  merge contact tests with separated fixture
 */
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <map>
#include <boost/lexical_cast.hpp>

#include "util/util.h"
#include "util/printable.h"
#include "util/map_at.h"
#include "src/fredlib/opcontext.h"
#include "tests/setup/fixtures.h"
#include "tests/fredlib/util.h"

#include "test_merge_contact_fixture.h"

/**
 * @namespace TestMergeContactSeparatedFixture
 * MergeContact test with separated fixture suite
 */
BOOST_FIXTURE_TEST_SUITE(TestMergeContactSeparatedFixture, Test::Fixture::instantiate_db_template)

/**
 * @namespace ObjectCombinations
 * tests using MergeContactFixture for linked object configurations
 */
BOOST_AUTO_TEST_SUITE(ObjectCombinations)
/**
 * Setup merge contact test data.
 * With mergeable contacts having data from one mergeable group,
 * with enumerated linked object configurations in default set of quantities per contact, set no states to contacts and set no states to linked objects.
 */
struct merge_fixture : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    merge_fixture()
    : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        1//mergeable_contact_group_count
        ,Util::set_of<unsigned>(0)(1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(14)(15)(20)//linked_object_cases
        , Util::vector_of<std::set<std::string> > (std::set<std::string>())(std::set<std::string>())//contact_state_combinations//stateless states 0, 1
        , Util::vector_of<std::set<std::string> > (std::set<std::string>())//linked_object_state_combinations
        , init_linked_object_quantities()//linked_object_quantities
        )
    {}
};

/**
 * Merge two mergeable contacts with no linked objects nor object states.
 */
BOOST_FIXTURE_TEST_CASE(test_no_linked_objects_no_states, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no other changes
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with linked nsset and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 1//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.size() == 1); //updated nsset, tech contact changed from src contact to dst contact

    std::string nsset_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
    );
    BOOST_MESSAGE(nsset_handle);

    BOOST_MESSAGE(std::string("changed nsset fields: (\"")
            + Util::format_container(map_at(changed_nssets,nsset_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_nssets,nsset_handle).update_time.get_value().second.isnull());

    //no other changes
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts both tech contacts of two linked nssets and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset_with_added_tech_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 3//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 3//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();

    for(std::map<std::string, Fred::InfoNssetDiff>::const_iterator ci = changed_nssets.begin(); ci != changed_nssets.end(); ++ci)
    {
        BOOST_MESSAGE("changed_nsset handle: " << ci->first);
    }

    BOOST_CHECK(changed_nssets.size() == 2); //updated nsset, tech contact changed from src contact to dst contact

    std::string nsset1_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
        , Util::vector_of<std::string>(contact_handle_dst)
    );
    BOOST_MESSAGE(nsset1_handle);

    BOOST_MESSAGE(std::string("changed nsset fields: (\"")
            + Util::format_container(map_at(changed_nssets,nsset1_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_nssets,nsset1_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_nssets,nsset1_handle).update_time.get_value().second.isnull());

    std::string nsset2_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_dst //tech contact
        , Util::vector_of<std::string>(contact_handle_src)
    );
    BOOST_MESSAGE(nsset2_handle);

    BOOST_MESSAGE(std::string("changed nsset fields: (\"")
            + Util::format_container(map_at(changed_nssets,nsset2_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_nssets,nsset2_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_nssets,nsset2_handle).update_time.get_value().second.isnull());

    //no other changes
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with linked keyset and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_keyset, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 5//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    BOOST_CHECK(diff_nssets().empty());

    //keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == 1); //updated keyset, tech contact changed from src contact to dst contact

    std::string keyset_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
    );
    BOOST_MESSAGE(keyset_handle);

    BOOST_MESSAGE(std::string("changed keyset fields: (\"")
        + Util::format_container(map_at(changed_keysets,keyset_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_keysets,keyset_handle).update_time.get_value().second.isnull());

    //no other changes
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts both tech contacts of two linked keysets and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_keyset_with_added_tech_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 7//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 7//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    BOOST_CHECK(diff_nssets().empty());

    //keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == 2); //updated keyset, tech contact changed from src contact to dst contact

    for(std::map<std::string, Fred::InfoKeysetDiff>::const_iterator ci = changed_keysets.begin(); ci != changed_keysets.end(); ++ci)
    {
        BOOST_MESSAGE("changed_keyset handle: " << ci->first);
    }

    std::string keyset1_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
        , Util::vector_of<std::string>(contact_handle_dst)
    );
    BOOST_MESSAGE(keyset1_handle);

    BOOST_MESSAGE(std::string("changed keyset fields: (\"")
        + Util::format_container(map_at(changed_keysets,keyset1_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().first.size() == 2);
    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_keysets,keyset1_handle).update_time.get_value().second.isnull());


    std::string keyset2_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_dst //tech contact
        , Util::vector_of<std::string>(contact_handle_src)
    );
    BOOST_MESSAGE(keyset2_handle);

    BOOST_MESSAGE(std::string("changed keyset fields: (\"")
        + Util::format_container(map_at(changed_keysets,keyset2_handle).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).changed_fields()
        == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).historyid.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.isset());

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().first.size() == 2);
    BOOST_CHECK(Util::set_of<std::string> (map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().first.at(0).handle)
        (map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string> (contact_handle_src)(contact_handle_dst));
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_keysets,keyset1_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_keysets,keyset2_handle).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_keysets,keyset2_handle).update_time.get_value().second.isnull());

    //no other changes
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with domain linked via owner and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_owner, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 13//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    BOOST_CHECK(diff_nssets().empty());

    //no keyset changes
    BOOST_CHECK(diff_keysets().empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
    );
    BOOST_MESSAGE(fqdn);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
        == Util::set_of<std::string>("historyid")("registrant")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().first.handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().second.handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());

    //no registrar changes
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with domain linked via admin and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_admin, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 9//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    BOOST_CHECK(diff_nssets().empty());

    //no keyset changes
    BOOST_CHECK(diff_keysets().empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_admin_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
            , 1) //owner contact
        , contact_handle_src //admin contact
    );
    BOOST_MESSAGE(fqdn);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(0).handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());

    //no registrar changes
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with five linked nssets and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset_5, merge_fixture)
{
    unsigned nsset_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 1//linked_object_case
        , 0//linked_object_state_case
        , nsset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , nsset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.size() == nsset_quantity); //updated nsset, tech contact changed from src contact to dst contact

    for(unsigned number = 0 ; number < nsset_quantity; ++number)
    {
        std::string nsset_handle= create_nsset_with_tech_contact_handle(
            0//linked_object_state_case
            , nsset_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //tech contact
        );
        BOOST_MESSAGE(nsset_handle);
        BOOST_MESSAGE(std::string("changed nsset fields: (\"")
                + Util::format_container(map_at(changed_nssets,nsset_handle).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).changed_fields()
            == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).historyid.isset());

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.isset());
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.size() == 1);
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.size() == 1);
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_nssets,nsset_handle).update_time.get_value().second.isnull());
    }

    //no other changes
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with five linked keysets and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_keyset_5, merge_fixture)
{
    unsigned keyset_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 5//linked_object_case
        , 0//linked_object_state_case
        , keyset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , keyset_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    BOOST_CHECK(diff_nssets().empty());

    //keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == keyset_quantity); //updated nsset, tech contact changed from src contact to dst contact

    for(unsigned number = 0 ; number < keyset_quantity; ++number)
    {
        std::string keyset_handle= create_keyset_with_tech_contact_handle(
            0//linked_object_state_case
            , keyset_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //tech contact
        );
        BOOST_MESSAGE(keyset_handle);

        BOOST_MESSAGE(std::string("changed keyset fields: (\"")
            + Util::format_container(map_at(changed_keysets,keyset_handle).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).changed_fields()
            == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).historyid.isset());

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.isset());
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.size() == 1);
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.size() == 1);
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_keysets,keyset_handle).update_time.get_value().second.isnull());
    }
    //no other changes
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with five domains linked via owner and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_owner_5, merge_fixture)
{
    unsigned domain_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 13//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    BOOST_CHECK(diff_nssets().empty());

    //no keyset changes
    BOOST_CHECK(diff_keysets().empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == domain_quantity); //updated domain, owner contact changed from src contact to dst contact

    for(unsigned number = 0; number < domain_quantity; ++number)
    {
        std::string fqdn= create_domain_with_owner_contact_fqdn(
            0//linked_object_state_case
            , domain_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //owner contact
        );
        BOOST_MESSAGE(fqdn);

        BOOST_MESSAGE(std::string("changed domain fields: (\"")
            + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
            == Util::set_of<std::string>("historyid")("registrant")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

        BOOST_CHECK(map_at(changed_domains,fqdn).registrant.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().first.handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().second.handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());
    }
    //no registrar changes
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with five domains linked via admin and no object states.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_admin_5, merge_fixture)
{
    unsigned domain_quantity = 5;
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 9//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no nsset changes
    BOOST_CHECK(diff_nssets().empty());

    //no keyset changes
    BOOST_CHECK(diff_keysets().empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == domain_quantity); //updated domain, owner contact changed from src contact to dst contact
    for(unsigned number = 0; number < domain_quantity; ++number)
    {
        std::string fqdn= create_domain_with_admin_contact_fqdn(
            0//linked_object_state_case
            , domain_quantity//quantity_case
            , number//number in quantity
            , create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
                , 1) //owner contact
            , contact_handle_src //admin contact
        );
        BOOST_MESSAGE(fqdn);

        BOOST_MESSAGE(std::string("changed domain fields: (\"")
            + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
            == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

        BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

        BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(0).handle == contact_handle_src);
        BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());
    }
    //no registrar changes
    BOOST_CHECK(diff_registrars().empty());
}
/**
 * Merge two mergeable contacts with linked nsset, keyset and two domains linked via admin and owner and no object states for given linked object configuration quantities.
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset_keyset_domain_via_admin_domain_via_owner, merge_fixture)
{
    unsigned accumulated_linked_object_quantity = 0;
    for(std::vector<unsigned>::const_iterator loq_ci = linked_object_quantities.begin(); loq_ci != linked_object_quantities.end(); ++loq_ci)
    {
        accumulated_linked_object_quantity += *loq_ci;

        std::string contact_handle_src = create_mergeable_contact_handle(
            registrar_vect.at(0)//registrar handle
            , 0 //grpidtag
            , 0//state_case
            , 15//linked_object_case
            , 0//linked_object_state_case
            , *loq_ci//quantity_case
        );
        BOOST_MESSAGE(contact_handle_src);
        std::string contact_handle_dst = create_mergeable_contact_handle(
            registrar_vect.at(0)//registrar handle
            , 0 //grpidtag
            , 1//state_case
            , 0//linked_object_case
            , 0//linked_object_state_case
            , *loq_ci//quantity_case
        );
        BOOST_MESSAGE(contact_handle_dst);
        try
        {
            Fred::OperationContext ctx;
            Fred::MergeContactOutput merge_data = Fred::MergeContact(
                contact_handle_src, contact_handle_dst
                , registrar_sys_handle).exec(ctx);
                ctx.commit_transaction();
                BOOST_MESSAGE(merge_data);
        }
        catch(boost::exception& ex)
        {
            BOOST_ERROR(boost::diagnostic_information(ex));
        }

        std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();

        //accumulated changed contacts 2,4,8,...
        BOOST_CHECK(changed_contacts.size() == (2+2*(loq_ci - linked_object_quantities.begin()))); //deleted src contact, updated dst contact authinfo

        BOOST_MESSAGE(std::string("changed src contact fields: (\"")
            + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

        BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
            + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
            == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
                != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

        std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();

        BOOST_CHECK(changed_nssets.size() == accumulated_linked_object_quantity); //updated nsset, tech contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string nsset_handle = create_nsset_with_tech_contact_handle(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //tech contact
            );
            BOOST_MESSAGE(nsset_handle);

            BOOST_MESSAGE(std::string("changed nsset fields: (\"")
                    + Util::format_container(map_at(changed_nssets,nsset_handle).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).changed_fields()
                == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).historyid.isset());

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.isset());
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.size() == 1);
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.size() == 1);
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_nssets,nsset_handle).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_nssets,nsset_handle).update_time.get_value().second.isnull());
        }

        std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
        BOOST_CHECK(changed_keysets.size() == accumulated_linked_object_quantity); //updated keyset, tech contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string keyset_handle= create_keyset_with_tech_contact_handle(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //tech contact
            );
            BOOST_MESSAGE(keyset_handle);

            BOOST_MESSAGE(std::string("changed keyset fields: (\"")
                + Util::format_container(map_at(changed_keysets,keyset_handle).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).changed_fields()
                == Util::set_of<std::string>("historyid")("tech_contacts")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).historyid.isset());

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.isset());
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.size() == 1);
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().first.at(0).handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.size() == 1);
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).tech_contacts.get_value().second.at(0).handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_keysets,keyset_handle).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_keysets,keyset_handle).update_time.get_value().second.isnull());
        }

        std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
        BOOST_CHECK(changed_domains.size() == (accumulated_linked_object_quantity * 2)); //updated domains, owner and admin contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string owner_fqdn= create_domain_with_owner_contact_fqdn(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //owner contact
            );
            BOOST_MESSAGE(owner_fqdn);

            BOOST_MESSAGE(std::string("changed domain fields: (\"")
                + Util::format_container(map_at(changed_domains,owner_fqdn).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).changed_fields()
                == Util::set_of<std::string>("historyid")("registrant")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).historyid.isset());

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).registrant.isset());
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).registrant.get_value().first.handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).registrant.get_value().second.handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_domains,owner_fqdn).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_domains,owner_fqdn).update_time.get_value().second.isnull());

            std::string admin_fqdn = create_domain_with_admin_contact_fqdn(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
                    , 1) //owner contact
                , contact_handle_src //admin contact
            );
            BOOST_MESSAGE(admin_fqdn);

            BOOST_MESSAGE(std::string("changed domain fields: (\"")
                + Util::format_container(map_at(changed_domains,admin_fqdn).changed_fields(), "\")(\"") + "\")");
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).changed_fields()
                == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).admin_contacts.isset());
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).admin_contacts.get_value().first.at(0).handle == contact_handle_src);
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).historyid.isset());

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_registrar_handle.isset());
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_registrar_handle.get_value().first.isnull());
            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

            BOOST_CHECK(map_at(changed_domains,admin_fqdn).update_time.get_value().first.isnull());
            BOOST_CHECK(!map_at(changed_domains,admin_fqdn).update_time.get_value().second.isnull());
        }

        BOOST_CHECK(diff_registrars().empty());
    }//for linked object quantities
}

/**
 * Try to merge nonexisting contact to existing one.
 * Check exception have set unknown_source_contact_handle.
 */
BOOST_FIXTURE_TEST_CASE(test_non_existing_src_contact, merge_fixture)
{
    std::string contact_handle_src = "NONEXISTENT";
    BOOST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);
    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_source_contact_handle());
        BOOST_CHECK(ex.get_unknown_source_contact_handle().compare(contact_handle_src) == 0);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge existing contact with linked nsset, keyset and two domains linked via admin and owner and no object states to nonexisting contact.
 * Check exception have set unknown_destination_contact_handle.
 */
BOOST_FIXTURE_TEST_CASE(test_non_existing_dst_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = "NONEXISTENT";
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_destination_contact_handle());
        BOOST_CHECK(ex.get_unknown_destination_contact_handle().compare(contact_handle_dst) == 0);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge existing contact with linked nsset, keyset and two domains linked via admin and owner and no object states to nonmergeable contact.
 * Check exception have set contacts_differ.
 */
BOOST_FIXTURE_TEST_CASE(test_different_dst_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
        , 1); //owner contact
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_contacts_differ());
        BOOST_CHECK(ex.get_contacts_differ().source_handle == contact_handle_src);
        BOOST_CHECK(ex.get_contacts_differ().destination_handle == contact_handle_dst);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge nonmergeable contact to contact with linked nsset, keyset and two domains linked via admin and owner and no object states.
 * Check exception have set contacts_differ.
 */
BOOST_FIXTURE_TEST_CASE(test_different_src_contact, merge_fixture)
{
    std::string contact_handle_src = create_non_mergeable_contact_handle(registrar_vect.at(0)//registrar handle
        , 1); //owner contact
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_contacts_differ());
        BOOST_CHECK(ex.get_contacts_differ().source_handle == contact_handle_src);
        BOOST_CHECK(ex.get_contacts_differ().destination_handle == contact_handle_dst);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge contact with linked nsset, keyset and two domains linked via admin and owner and no object states to oneself.
 * Check exception have set identical_contacts.
 */
BOOST_FIXTURE_TEST_CASE(test_identical_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = contact_handle_src;
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_identical_contacts_handle());
        BOOST_CHECK(ex.get_identical_contacts_handle() == contact_handle_src);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with two linked domains. First domain with owner src contact and admin src and dest contacts,
 * second domain with owner dest contact and admin src and dest contact.
 */
BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_owner_with_the_same_admin_and_merged_to_different_mergeable_admin_contact, merge_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    //contact changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no changes
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, Fred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        BOOST_MESSAGE("changed_domain fqdn: " << ci->first);
    }

    BOOST_CHECK(changed_domains.size() == 2); //updated domain, owner and admin contact changed from src contact to dst contact

    std::string fqdn1= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
        ,  Util::vector_of<std::string>(contact_handle_src)(contact_handle_dst)
    );
    BOOST_MESSAGE(fqdn1);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn1).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn1).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("registrant")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string>(map_at(changed_domains,fqdn1).admin_contacts.get_value().first.at(0).handle)
        (map_at(changed_domains,fqdn1).admin_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string>(contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_domains,fqdn1).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn1).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn1).registrant.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn1).registrant.get_value().first.handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn1).registrant.get_value().second.handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn1).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn1).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn1).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn1).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn1).update_time.get_value().second.isnull());

    std::string fqdn2= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_dst //owner contact
        ,  Util::vector_of<std::string>(contact_handle_dst)(contact_handle_src)
    );
    BOOST_MESSAGE(fqdn2);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn2).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn2).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string>(map_at(changed_domains,fqdn2).admin_contacts.get_value().first.at(0).handle)
        (map_at(changed_domains,fqdn2).admin_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string>(contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_domains,fqdn2).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn2).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn2).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn2).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn2).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn2).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn2).update_time.get_value().second.isnull());

    //no registrar changes
    BOOST_CHECK(diff_registrars().empty());
}

BOOST_AUTO_TEST_SUITE_END();//ObjectCombinations

/**
 * @namespace StateCombinations
 * tests using MergeContactFixture for linked object configurations with object states
 */
BOOST_AUTO_TEST_SUITE(StateCombinations)

/**
 * Setup merge contact test data with states.
 * With mergeable contacts having data from one mergeable group,
 * with enumerated linked object configurations in default set of quantities per contact, set default states to contacts and set default states to linked objects.
 */
struct merge_with_states_fixture : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    merge_with_states_fixture()
    : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        1//mergeable_contact_group_count
        ,Util::set_of<unsigned>(15)(18)(19)(20)//linked_object_cases
        , init_set_of_contact_state_combinations()//contact_state_combinations//stateless states 0, 1
        , init_set_of_linked_object_state_combinations()//linked_object_state_combinations
        , init_linked_object_quantities()//linked_object_quantities
        )
    {}
};

/**
 * No merge, no changes check.
 */
BOOST_FIXTURE_TEST_CASE(test_merge_with_states_fixture, merge_with_states_fixture)
{
    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge contact with linked nsset, keyset and two domains linked via admin and owner and  mojeidContact object state to otherwise mergeable contact.
 * Check exception have set src_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_invalid_src_mojeid_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 6//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid() == contact_handle_src);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}


/**
 * Try to merge contact with linked nsset, keyset and two domains linked via admin and owner
 * and serverBlocked object state to otherwise mergeable contact.
 * Check exception have set src_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_invalid_src_serverblocked_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 5//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid() == contact_handle_src);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge contact with linked nsset, keyset and two domains linked via admin and owner
 * and serverDeleteProhibited object state to otherwise mergeable contact.
 * Check exception have set src_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_invalid_src_deleteprohibited_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 4//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_src_contact_invalid());
        BOOST_CHECK(ex.get_src_contact_invalid() == contact_handle_src);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge contact to otherwise mergeable contact with linked nsset, keyset and two domains linked via admin and owner
 * and serverBlocked object state.
 * Check exception have set dst_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_invalid_dst_serverblocked_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 5//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_dst_contact_invalid());
        BOOST_CHECK(ex.get_dst_contact_invalid() == contact_handle_dst);
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge contact with domain in serverBlocked object state linked via admin.
 * Check exception have set object_blocked.
 */
BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_admin_serverblocked, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 18//linked_object_case
        , 2//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_blocked());
        BOOST_CHECK(ex.get_object_blocked() == create_domain_with_admin_contact_fqdn(
            2 //linked_object_state_case
            , 1 //quantity_case
            , 0 //number in quantity
            , create_non_mergeable_contact_handle(registrar_vect.at(0), 1) //owner contact
            , contact_handle_src//admin contact
            ));
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Try to merge contact with domain in serverBlocked object state linked via owner.
 * Check exception have set dst_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_owner_serverblocked, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 19//linked_object_case
        , 2//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(Fred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_blocked());
        BOOST_CHECK(ex.get_object_blocked() == create_domain_with_owner_contact_fqdn(
            2 //linked_object_state_case
            , 1 //quantity_case
            , 0 //number in quantity
            , contact_handle_src//owner contact
            ));
    }

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());
}

/**
 * Merge two mergeable contacts with two linked domains. First domain in serverUpdateProhibited object state with owner src contact and admin src and dest contacts,
 * second domain with owner dest contact and admin src and dest contact.
 */
BOOST_FIXTURE_TEST_CASE(test_src_contact_updateprohibited_linked_domain_via_owner_with_the_same_admin_and_merged_to_different_mergeable_admin_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 2//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_mergeable_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 0 //grpidtag
        , 0//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_MESSAGE(contact_handle_dst);

    try
    {
        Fred::OperationContext ctx;
        Fred::MergeContactOutput merge_data = Fred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    //contact changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_MESSAGE(std::string("changed dst contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_dst).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).changed_fields()
        == Util::set_of<std::string>("update_time")("authinfopw")("historyid")("update_registrar_handle"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_dst).update_time.get_value().second.isnull());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().first
            != map_at(changed_contacts,contact_handle_dst).authinfopw.get_value().second);

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).historyid.isset());

    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_dst).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    //no changes
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, Fred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        BOOST_MESSAGE("changed_domain fqdn: " << ci->first);
    }

    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner and admin contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
        ,  Util::vector_of<std::string>(contact_handle_src)(contact_handle_dst)
    );
    BOOST_MESSAGE(fqdn);

    BOOST_MESSAGE(std::string("changed domain fields: (\"")
        + Util::format_container(map_at(changed_domains,fqdn).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_domains,fqdn).changed_fields()
        == Util::set_of<std::string>("admin_contacts")("historyid")("registrant")("update_registrar_handle")("update_time"));

    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().first.size() == 2);

    BOOST_CHECK(Util::set_of<std::string>(map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(0).handle)
        (map_at(changed_domains,fqdn).admin_contacts.get_value().first.at(1).handle)
        == Util::set_of<std::string>(contact_handle_src)(contact_handle_dst));

    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.size() == 1);
    BOOST_CHECK(map_at(changed_domains,fqdn).admin_contacts.get_value().second.at(0).handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).historyid.isset());

    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().first.handle == contact_handle_src);
    BOOST_CHECK(map_at(changed_domains,fqdn).registrant.get_value().second.handle == contact_handle_dst);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.isset());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().first.isnull());
    BOOST_CHECK(map_at(changed_domains,fqdn).update_registrar_handle.get_value().second.get_value() == registrar_sys_handle);

    BOOST_CHECK(map_at(changed_domains,fqdn).update_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_domains,fqdn).update_time.get_value().second.isnull());

    //no registrar changes
    BOOST_CHECK(diff_registrars().empty());
}

BOOST_AUTO_TEST_SUITE_END();//StateCombinations
BOOST_AUTO_TEST_SUITE_END();//TestMergeContactSeparatedFixture

