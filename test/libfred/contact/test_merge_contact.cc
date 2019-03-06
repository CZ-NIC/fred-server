/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  merge contact tests with separated fixture
 */
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <map>
#include <boost/lexical_cast.hpp>

#include "util/util.hh"
#include "util/printable.hh"
#include "util/map_at.hh"
#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"

#include "test/libfred/contact/test_merge_contact_fixture.hh"

namespace Test {
namespace LibFred {
namespace Contact {
namespace MergeContact {

/**
 * @namespace ObjectCombinations
 * tests using MergeContactAutoProc for linked object configurations
 */
BOOST_AUTO_TEST_SUITE(ObjectCombinations)
/**
 * Setup merge contact test data.
 * With mergeable contacts having data from one mergeable group,
 * with enumerated linked object configurations in default set of quantities per contact, set no states to contacts and set no states to linked objects.
 */
struct merge_fixture : MergeContactAutoProc::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    merge_fixture()
    : MergeContactAutoProc::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        ""//empty db name suffix
        ,1//mergeable_contact_group_count
        ,Util::set_of<unsigned>(0)(1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(14)(15)(20)//linked_object_cases
        , Util::vector_of<std::set<std::string> > (std::set<std::string>())(std::set<std::string>())//contact_state_combinations//stateless states 0, 1
        , Util::vector_of<std::set<std::string> > (std::set<std::string>())//linked_object_state_combinations
        , init_linked_object_quantities()//linked_object_quantities
        )
    {}
};

/**
 * Merge two mergeable contacts
 *  - no linked objects
 *  - no contact states
 */
BOOST_FIXTURE_TEST_CASE(test_no_linked_objects_no_states, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as tech contact of nsset
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - nsset having source contact as tech contact
 *  .
 * Object States:
 *  - no contact states
 *  - no nsset states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 1//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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

    std::map<std::string, ::LibFred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.size() == 1); //updated nsset, tech contact changed from src contact to dst contact

    std::string nsset_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
    );
    BOOST_TEST_MESSAGE(nsset_handle);

    BOOST_TEST_MESSAGE(std::string("changed nsset fields: (\"")
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
 * Merge two mergeable contacts.
 *  - source contact as tech contact of nsset1 and nsset2
 *  - destination contact as tech contact of nsset1 and nsset2
 *  .
 *  Linked objects:
 *  - nsset1 having source contact and destination contact as tech contacts
 *  - nsset2 having destination contact and source contact as tech contacts
 *  .
 *  Object states:
 *  - no contact states
 *  - no nsset states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset_with_added_tech_contact, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 3//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 3//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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

    std::map<std::string, ::LibFred::InfoNssetDiff> changed_nssets = diff_nssets();

    for(std::map<std::string, ::LibFred::InfoNssetDiff>::const_iterator ci = changed_nssets.begin(); ci != changed_nssets.end(); ++ci)
    {
        BOOST_TEST_MESSAGE("changed_nsset handle: " << ci->first);
    }

    BOOST_CHECK(changed_nssets.size() == 2); //updated nsset, tech contact changed from src contact to dst contact

    std::string nsset1_handle= create_nsset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
        , Util::vector_of<std::string>(contact_handle_dst)
    );
    BOOST_TEST_MESSAGE(nsset1_handle);

    BOOST_TEST_MESSAGE(std::string("changed nsset fields: (\"")
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
    BOOST_TEST_MESSAGE(nsset2_handle);

    BOOST_TEST_MESSAGE(std::string("changed nsset fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as tech contact of keyset
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - keyset having source contact as tech contact
 *  .
 * Object States:
 *  - no contact states
 *  - no keyset states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_keyset, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 5//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == 1); //updated keyset, tech contact changed from src contact to dst contact

    std::string keyset_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
    );
    BOOST_TEST_MESSAGE(keyset_handle);

    BOOST_TEST_MESSAGE(std::string("changed keyset fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as tech contact of keyset1 and keyset2
 *  - destination contact as tech contact of keyset1 and keyset2
 *  .
 *  Linked objects:
 *  - keyset1 having source contact and destination contact as tech contacts
 *  - keyset2 having destination contact and source contact as tech contacts
 *  .
 *  Object states:
 *  - no contact states
 *  - no keyset states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_keyset_with_added_tech_contact, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 7//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 7//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == 2); //updated keyset, tech contact changed from src contact to dst contact

    for(std::map<std::string, ::LibFred::InfoKeysetDiff>::const_iterator ci = changed_keysets.begin(); ci != changed_keysets.end(); ++ci)
    {
        BOOST_TEST_MESSAGE("changed_keyset handle: " << ci->first);
    }

    std::string keyset1_handle= create_keyset_with_tech_contact_handle(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //tech contact
        , Util::vector_of<std::string>(contact_handle_dst)
    );
    BOOST_TEST_MESSAGE(keyset1_handle);

    BOOST_TEST_MESSAGE(std::string("changed keyset fields: (\"")
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
    BOOST_TEST_MESSAGE(keyset2_handle);

    BOOST_TEST_MESSAGE(std::string("changed keyset fields: (\"")
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
 * Merge two mergeable contacts.
 *  - source contact as owner of domain
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - domain having source contact as owner
 *  .
 * Object States:
 *  - no contact states
 *  - no domain states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_owner, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 13//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
    );
    BOOST_TEST_MESSAGE(fqdn);

    BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as admin of domain
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - domain having source contact as admin
 *  .
 * Object States:
 *  - no contact states
 *  - no domain states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_admin, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 9//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner contact changed from src contact to dst contact

    std::string fqdn= create_domain_with_admin_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , create_contact_handle(registrar_vect.at(0), 0, 1, 0, 15, 0, 1)//owner contact
        , contact_handle_src //admin contact
    );
    BOOST_TEST_MESSAGE(fqdn);

    BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as tech contact of five nssets
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - each nsset having source contact as tech contact
 *  .
 * Object States:
 *  - no contact states
 *  - no nsset states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset_5, merge_fixture)
{
    unsigned nsset_quantity = 5;
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 1//linked_object_case
        , 0//linked_object_state_case
        , nsset_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , nsset_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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

    std::map<std::string, ::LibFred::InfoNssetDiff> changed_nssets = diff_nssets();
    BOOST_CHECK(changed_nssets.size() == nsset_quantity); //updated nsset, tech contact changed from src contact to dst contact

    for(unsigned number = 0 ; number < nsset_quantity; ++number)
    {
        std::string nsset_handle= create_nsset_with_tech_contact_handle(
            0//linked_object_state_case
            , nsset_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //tech contact
        );
        BOOST_TEST_MESSAGE(nsset_handle);
        BOOST_TEST_MESSAGE(std::string("changed nsset fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as tech contact of five keysets
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - each keyset having source contact as tech contact
 *  .
 * Object States:
 *  - no contact states
 *  - no keyset states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_keyset_5, merge_fixture)
{
    unsigned keyset_quantity = 5;
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 5//linked_object_case
        , 0//linked_object_state_case
        , keyset_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , keyset_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoKeysetDiff> changed_keysets = diff_keysets();
    BOOST_CHECK(changed_keysets.size() == keyset_quantity); //updated nsset, tech contact changed from src contact to dst contact

    for(unsigned number = 0 ; number < keyset_quantity; ++number)
    {
        std::string keyset_handle= create_keyset_with_tech_contact_handle(
            0//linked_object_state_case
            , keyset_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //tech contact
        );
        BOOST_TEST_MESSAGE(keyset_handle);

        BOOST_TEST_MESSAGE(std::string("changed keyset fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as owner of five domains
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - each domain having source contact as owner
 *  .
 * Object States:
 *  - no contact states
 *  - no domain states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_owner_5, merge_fixture)
{
    unsigned domain_quantity = 5;
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 13//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == domain_quantity); //updated domain, owner contact changed from src contact to dst contact

    for(unsigned number = 0; number < domain_quantity; ++number)
    {
        std::string fqdn= create_domain_with_owner_contact_fqdn(
            0//linked_object_state_case
            , domain_quantity//quantity_case
            , number//number in quantity
            , contact_handle_src //owner contact
        );
        BOOST_TEST_MESSAGE(fqdn);

        BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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
 * Merge two mergeable contacts:
 *  - source contact as admin of five domains
 *  - destination contact with no linked objects
 *  .
 *  Linked objects:
 *  - each domain having source contact as admin contact
 *  .
 * Object States:
 *  - no contact states
 *  - no domain states
 */
BOOST_FIXTURE_TEST_CASE(test_linked_domain_via_admin_5, merge_fixture)
{
    unsigned domain_quantity = 5;
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 9//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , domain_quantity//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
    BOOST_CHECK(changed_domains.size() == domain_quantity); //updated domain, owner contact changed from src contact to dst contact
    for(unsigned number = 0; number < domain_quantity; ++number)
    {
        std::string fqdn= create_domain_with_admin_contact_fqdn(
            0//linked_object_state_case
            , domain_quantity//quantity_case
            , number//number in quantity
            , create_contact_handle(registrar_vect.at(0), 0, 1, 0, 15, 0, 1) //owner contact
            , contact_handle_src //admin contact
        );
        BOOST_TEST_MESSAGE(fqdn);

        BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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
 * Merge two mergeable contacts for quantity 0 with:
 *  - no linked objects
 *  - no contact states
 *  .
 * then merge two mergeable contacts for quantity 1 with:
 *  - source contact as tech contact of nsset
 *  - source contact as tech contact of keyset
 *  - source contact as admin contact of domain in variable admin_fqdn
 *  - source contact as owner contact of domain in variable owner_fqdn
 *  - destination contact with no linked objects
 *  .
 * then merge two mergeable contacts for quantity 2 with:
 *  - source contact as tech contact of two nssets
 *  - source contact as tech contact of two keysets
 *  - source contact as admin contact of two domains in variable admin_fqdn
 *  - source contact as owner contact of two domain in variable owner_fqdn
 *  - destination contact with no linked objects
 *  - no linked object states
 *  - no contact states
 *  .
 * then merge two mergeable contacts for quantity 5 with:
 *  - source contact as tech contact of five nssets
 *  - source contact as tech contact of five keysets
 *  - source contact as admin contact of five domains in variable admin_fqdn
 *  - source contact as owner contact of five domain in variable owner_fqdn
 *  - destination contact with no linked objects
 *  - no linked object states
 *  - no contact states
 *  .
 *  Linked objects:
 *  - each nsset having source contact for its quantity as tech contact
 *  - each keyset having source contact for its quantity as tech contact
 *  - each domain in variable admin_fqdn having source contact for its quantity as admin contact
 *  - each domain in variable owner_fqdn having source contact for its quantity as owner contact
 *  .
 * Object States:
 *  - no contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 */
BOOST_FIXTURE_TEST_CASE(test_linked_nsset_keyset_domain_via_admin_domain_via_owner, merge_fixture)
{
    unsigned accumulated_linked_object_quantity = 0;
    for(std::vector<unsigned>::const_iterator loq_ci = linked_object_quantities.begin(); loq_ci != linked_object_quantities.end(); ++loq_ci)
    {
        accumulated_linked_object_quantity += *loq_ci;

        std::string contact_handle_src = create_contact_handle(
            registrar_vect.at(0)//registrar handle
            , 1 //contact data
            , 0 //grpidtag
            , 0//state_case
            , 15//linked_object_case
            , 0//linked_object_state_case
            , *loq_ci//quantity_case
        );
        BOOST_TEST_MESSAGE(contact_handle_src);
        std::string contact_handle_dst = create_contact_handle(
            registrar_vect.at(0)//registrar handle
            , 1 //contact data
            , 0 //grpidtag
            , 1//state_case
            , 0//linked_object_case
            , 0//linked_object_state_case
            , *loq_ci//quantity_case
        );
        BOOST_TEST_MESSAGE(contact_handle_dst);
        try
        {
            ::LibFred::OperationContextCreator ctx;
            ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
                contact_handle_src, contact_handle_dst
                , registrar_sys_handle).exec(ctx);
                ctx.commit_transaction();
                BOOST_TEST_MESSAGE(merge_data);
        }
        catch(boost::exception& ex)
        {
            BOOST_ERROR(boost::diagnostic_information(ex));
        }

        std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();

        //accumulated changed contacts 2,4,8,...
        BOOST_CHECK(changed_contacts.size() == static_cast<std::size_t>(2+2*(loq_ci - linked_object_quantities.begin()))); //deleted src contact, updated dst contact authinfo

        BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
            + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
        BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
        BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

        BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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

        std::map<std::string, ::LibFred::InfoNssetDiff> changed_nssets = diff_nssets();

        BOOST_CHECK(changed_nssets.size() == accumulated_linked_object_quantity); //updated nsset, tech contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string nsset_handle = create_nsset_with_tech_contact_handle(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //tech contact
            );
            BOOST_TEST_MESSAGE(nsset_handle);

            BOOST_TEST_MESSAGE(std::string("changed nsset fields: (\"")
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

        std::map<std::string, ::LibFred::InfoKeysetDiff> changed_keysets = diff_keysets();
        BOOST_CHECK(changed_keysets.size() == accumulated_linked_object_quantity); //updated keyset, tech contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string keyset_handle= create_keyset_with_tech_contact_handle(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //tech contact
            );
            BOOST_TEST_MESSAGE(keyset_handle);

            BOOST_TEST_MESSAGE(std::string("changed keyset fields: (\"")
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

        std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
        BOOST_CHECK(changed_domains.size() == (accumulated_linked_object_quantity * 2)); //updated domains, owner and admin contact changed from src contact to dst contact

        for(unsigned number = 0; number < *loq_ci; ++number)//if src contact have linked object
        {
            std::string owner_fqdn= create_domain_with_owner_contact_fqdn(
                0//linked_object_state_case
                , *loq_ci//quantity_case
                , number//number in quantity
                , contact_handle_src //owner contact
            );
            BOOST_TEST_MESSAGE(owner_fqdn);

            BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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
                , create_contact_handle(registrar_vect.at(0), 0, 1, 0, 15, 0, 1) //owner contact
                , contact_handle_src //admin contact
            );
            BOOST_TEST_MESSAGE(admin_fqdn);

            BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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
 * Try to merge nonexisting source contact to existing destination contact.
 *  - no linked objects
 *  - no contact states
 *  .
 * Check exception have set unknown_source_contact_handle.
 */
BOOST_FIXTURE_TEST_CASE(test_non_existing_src_contact, merge_fixture)
{
    std::string contact_handle_src = "NONEXISTENT";
    BOOST_TEST_MESSAGE(contact_handle_src);
    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 0//linked_object_case
        , 0//linked_object_state_case
        , 0//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge existing source contact to nonexisting destination contact.
 *  - source contact as tech contact of nsset
 *  - source contact as tech contact of keyset
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  .
 *  Linked objects:
 *  - nsset having source contact as tech contact
 *  - keyset having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  .
 * Object states:
 *  - no contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set unknown_destination_contact_handle.
 */
BOOST_FIXTURE_TEST_CASE(test_non_existing_dst_contact, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = "NONEXISTENT";
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge existing source contact to destination contact with different contact data.
 *  - source contact as tech contact of nsset
 *  - source contact as tech contact of keyset
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  .
 * Linked objects:
 *  - nsset having source contact as tech contact
 *  - keyset having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  .
 * Object states:
 *  - no contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set contacts_differ.
 */
BOOST_FIXTURE_TEST_CASE(test_different_dst_contact, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(registrar_vect.at(0), 0, 1, 0, 15, 0, 1); //owner contact
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge existing source contact to destination contact with different contact data.
 *  - destination contact as tech contact of nsset
 *  - destination contact as tech contact of keyset
 *  - destination contact as admin contact of domain1
 *  - destination contact as owner contact of domain2
 *  .
 * Linked objects:
 *  - nsset having destination contact as tech contact
 *  - keyset having destination contact as tech contact
 *  - domain1 having destination contact as admin contact
 *  - domain2 having destination contact as owner contact
 *  .
 * Object states:
 *  - no contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set contacts_differ.
 */
BOOST_FIXTURE_TEST_CASE(test_different_src_contact, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(registrar_vect.at(0), 0, 1, 0, 15, 0, 1); //owner contact
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge existing source contact to to oneself.
 *  - source contact as tech contact of nsset
 *  - source contact as tech contact of keyset
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  .
 * Linked objects:
 *  - nsset having source contact as tech contact
 *  - keyset having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  .
 * Object states:
 *  - no contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set identical_contacts.
 */
BOOST_FIXTURE_TEST_CASE(test_identical_contact, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = contact_handle_src;
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Merge two mergeable contacts:
 *  - source contact as owner contact of domain fqdn1
 *  - source contact as admin contact of domain fqdn1
 *  - destination contact as admin contact of domain fqdn1
 *  - destination contact as owner contact of domain fqdn2
 *  - source contact as admin contact of domain fqdn2
 *  - destination contact as admin contact of domain fqdn2
 * .
 * Linked objects:
 *  - domain fqdn1 having source contact as admin and owner contact and destination contact as admin contact
 *  - domain fqdn2 having destination contact as admin and owner contact and source contact as admin contact
 *  .
 * Object states:
 *  - no contact states
 *  - no domain states
 *  .
 */
BOOST_FIXTURE_TEST_CASE(test_src_domain_owner_with_admin_to_different_admin, merge_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    //contact changes
    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, ::LibFred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        BOOST_TEST_MESSAGE("changed_domain fqdn: " << ci->first);
    }

    BOOST_CHECK(changed_domains.size() == 2); //updated domain, owner and admin contact changed from src contact to dst contact

    std::string fqdn1= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
        ,  Util::vector_of<std::string>(contact_handle_src)(contact_handle_dst)
    );
    BOOST_TEST_MESSAGE(fqdn1);

    BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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
    BOOST_TEST_MESSAGE(fqdn2);

    BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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

BOOST_AUTO_TEST_SUITE_END(); // Test::LibFred::MergeContact::ObjectCombinations

/**
 * @namespace StateCombinations
 * tests using MergeContactAutoProc for linked object configurations with object states
 */
BOOST_AUTO_TEST_SUITE(StateCombinations)

/**
 * Setup merge contact test data with states.
 * With mergeable contacts having data from one mergeable group,
 * with enumerated linked object configurations in default set of quantities per contact, set default states to contacts and set default states to linked objects.
 */
struct merge_with_states_fixture : MergeContactAutoProc::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    merge_with_states_fixture()
    : MergeContactAutoProc::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        ""//empty db name suffix
        ,1//mergeable_contact_group_count
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
 * Try to merge source contact with mojeidContact object state to destination contact with the same data.
 *  - source contact as tech contact of nsset1
 *  - source contact as tech contact of keyset1
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  - destination contact as tech contact of nsset2
 *  - destination contact as tech contact of keyset2
 *  - destination contact as admin contact of domain3
 *  - destination contact as owner contact of domain4
 *  .
 * Linked objects:
 *  - nsset1 having source contact as tech contact
 *  - keyset1 having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  - nsset2 having destination contact as tech contact
 *  - keyset2 having destination contact as tech contact
 *  - domain3 having destination contact as admin contact
 *  - domain4 having destination contact as owner contact
 *  .
 * Object states:
 *  - source contact in mojeidContact state
 *  - no destination contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set src_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_invalid_src_mojeid_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 6//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge source contact with serverBlocked object state to destination contact with the same data.
 *  - source contact as tech contact of nsset1
 *  - source contact as tech contact of keyset1
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  - destination contact as tech contact of nsset2
 *  - destination contact as tech contact of keyset2
 *  - destination contact as admin contact of domain3
 *  - destination contact as owner contact of domain4
 *  .
 * Linked objects:
 *  - nsset1 having source contact as tech contact
 *  - keyset1 having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  - nsset2 having destination contact as tech contact
 *  - keyset2 having destination contact as tech contact
 *  - domain3 having destination contact as admin contact
 *  - domain4 having destination contact as owner contact
 *  .
 * Object states:
 *  - source contact in serverBlocked state
 *  - no destination contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set src_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_invalid_src_serverblocked_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 5//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge source contact with serverDeleteProhibited object state to destination contact with the same data.
 *  - source contact as tech contact of nsset1
 *  - source contact as tech contact of keyset1
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  - destination contact as tech contact of nsset2
 *  - destination contact as tech contact of keyset2
 *  - destination contact as admin contact of domain3
 *  - destination contact as owner contact of domain4
 *  .
 * Linked objects:
 *  - nsset1 having source contact as tech contact
 *  - keyset1 having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  - nsset2 having destination contact as tech contact
 *  - keyset2 having destination contact as tech contact
 *  - domain3 having destination contact as admin contact
 *  - domain4 having destination contact as owner contact
 *  .
 * Object states:
 *  - source contact in serverDeleteProhibited state
 *  - no destination contact states
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set src_contact_invalid.
 */
BOOST_FIXTURE_TEST_CASE(test_invalid_src_deleteprohibited_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 4//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge source contact to destination contact with the same data and serverBlocked object state.
 *  - source contact as tech contact of nsset1
 *  - source contact as tech contact of keyset1
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  - destination contact as tech contact of nsset2
 *  - destination contact as tech contact of keyset2
 *  - destination contact as admin contact of domain3
 *  - destination contact as owner contact of domain4
 *  .
 * Linked objects:
 *  - nsset1 having source contact as tech contact
 *  - keyset1 having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  - nsset2 having destination contact as tech contact
 *  - keyset2 having destination contact as tech contact
 *  - domain3 having destination contact as admin contact
 *  - domain4 having destination contact as owner contact
 *  .
 * Object states:
 *  - no source contact states
 *  - destination contact in serverBlocked state
 *  - no nsset states
 *  - no keyset states
 *  - no domain states
 *  .
 * Check exception have set dst_contact_invalid.
 */

BOOST_FIXTURE_TEST_CASE(test_invalid_dst_serverblocked_contact, merge_with_states_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 5//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Try to merge source contact to destination contact with the same data.
 *  - source contact as tech contact of nsset1
 *  - source contact as tech contact of keyset1
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  - destination contact as tech contact of nsset2
 *  - destination contact as tech contact of keyset2
 *  - destination contact as admin contact of domain3
 *  - destination contact as owner contact of domain4
 *  .
 * Linked objects:
 *  - nsset1 having source contact as tech contact
 *  - keyset1 having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  - nsset2 having destination contact as tech contact
 *  - keyset2 having destination contact as tech contact
 *  - domain3 having destination contact as admin contact
 *  - domain4 having destination contact as owner contact
 *  .
 * Object states:
 *  - no contact states
 *  - no nsset states
 *  - no keyset states
 *  - domain1 in serverBlocked object state
 *  .
 * Check exception have set object_blocked.
 */
BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_admin_serverblocked, merge_with_states_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 18//linked_object_case
        , 2//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_blocked());
        BOOST_CHECK(ex.get_object_blocked() == create_domain_with_admin_contact_fqdn(
            2 //linked_object_state_case
            , 1 //quantity_case
            , 0 //number in quantity
            , create_contact_handle(registrar_vect.at(0), 0, 1, 0, 15, 0, 1) //owner contact
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
 * Try to merge source contact to destination contact with the same data.
 *  - source contact as tech contact of nsset1
 *  - source contact as tech contact of keyset1
 *  - source contact as admin contact of domain1
 *  - source contact as owner contact of domain2
 *  - destination contact as tech contact of nsset2
 *  - destination contact as tech contact of keyset2
 *  - destination contact as admin contact of domain3
 *  - destination contact as owner contact of domain4
 *  .
 * Linked objects:
 *  - nsset1 having source contact as tech contact
 *  - keyset1 having source contact as tech contact
 *  - domain1 having source contact as admin contact
 *  - domain2 having source contact as owner contact
 *  - nsset2 having destination contact as tech contact
 *  - keyset2 having destination contact as tech contact
 *  - domain3 having destination contact as admin contact
 *  - domain4 having destination contact as owner contact
 *  .
 * Object states:
 *  - no contact states
 *  - no nsset states
 *  - no keyset states
 *  - domain2 in serverBlocked object state
 *  .
 * Check exception have set object_blocked.
 */
BOOST_FIXTURE_TEST_CASE(test_src_contact_linked_domain_via_owner_serverblocked, merge_with_states_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 19//linked_object_case
        , 2//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 1//state_case
        , 15//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_ERROR(merge_data);
    }
    catch(::LibFred::MergeContact::Exception& ex)
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
 * Merge two mergeable contacts:
 *  - source contact as owner contact of domain fqdn1
 *  - source contact as admin contact of domain fqdn1
 *  - destination contact as admin contact of domain fqdn1
 *  - destination contact as owner contact of domain fqdn2
 *  - unrelated contact (state case 1) as admin contact of domain fqdn2
 *  - destination contact as admin contact of domain fqdn2
 * .
 * Linked objects:
 *  - domain fqdn1 having source contact as admin and owner contact and destination contact as admin contact
 *  - domain fqdn2 having destination contact as admin and owner contact and unrelated contact (state case 1) as admin contact
 *  .
 * Object states:
 *  - source contact in serverUpdateProhibited state
 *  - no domain states
 *  .
 */
BOOST_FIXTURE_TEST_CASE(test_src_updproh_domain_owner_and_admin_to_other_admin, merge_with_states_fixture)
{
    std::string contact_handle_src = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 2//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_src);

    std::string contact_handle_dst = create_contact_handle(
        registrar_vect.at(0)//registrar handle
        , 1 //contact data
        , 0 //grpidtag
        , 0//state_case
        , 20//linked_object_case
        , 0//linked_object_state_case
        , 1//quantity_case
    );
    BOOST_TEST_MESSAGE(contact_handle_dst);

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::MergeContactOutput merge_data = ::LibFred::MergeContact(
            contact_handle_src, contact_handle_dst
            , registrar_sys_handle).exec(ctx);
            ctx.commit_transaction();
            BOOST_TEST_MESSAGE(merge_data);
    }
    catch(boost::exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    //contact changes
    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();
    BOOST_CHECK(changed_contacts.size() == 2); //deleted src contact, updated dst contact authinfo

    //src contact
    BOOST_TEST_MESSAGE(std::string("changed src contact fields: (\"")
        + Util::format_container(map_at(changed_contacts,contact_handle_src).changed_fields(), "\")(\"") + "\")");
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).changed_fields() == Util::set_of<std::string>("delete_time"));

    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.isset());
    BOOST_CHECK(map_at(changed_contacts,contact_handle_src).delete_time.get_value().first.isnull());
    BOOST_CHECK(!map_at(changed_contacts,contact_handle_src).delete_time.get_value().second.isnull());

    //dst contact
    BOOST_TEST_MESSAGE(std::string("changed dst contact fields: (\"")
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
    std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, ::LibFred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        BOOST_TEST_MESSAGE("changed_domain fqdn: " << ci->first);
    }

    BOOST_CHECK(changed_domains.size() == 1); //updated domain, owner and admin contact changed from src contact to dst contact

    std::string fqdn1= create_domain_with_owner_contact_fqdn(
        0//linked_object_state_case
        , 1//quantity_case
        , 0//number in quantity
        , contact_handle_src //owner contact
        ,  Util::vector_of<std::string>(contact_handle_src)(contact_handle_dst)
    );
    BOOST_TEST_MESSAGE(fqdn1);

    BOOST_TEST_MESSAGE(std::string("changed domain fields: (\"")
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

    //no registrar changes
    BOOST_CHECK(diff_registrars().empty());
}

BOOST_AUTO_TEST_SUITE_END(); // Test::LibFred::MergeContact::StateCombinations

} // namespace Test::LibFred::MergeContact
} // namespace Test::LibFred::Contact
} // namespace Test::LibFred
} // namespace Test
