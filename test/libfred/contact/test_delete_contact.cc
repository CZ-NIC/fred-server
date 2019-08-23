/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/delete_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_diff.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/asio/ip/address.hpp>
#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-delete-contact";

struct test_contact_fixture  : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;

    test_contact_fixture()
    :xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle  = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        ::LibFred::ContactAddress address_no_company_name;
        address_no_company_name.street1 = "Betelgeuze 42";
        address_no_company_name.city = "Orion";
        address_no_company_name.postalcode = "11150";
        address_no_company_name.country = "MZ";

        ::LibFred::ContactAddress address_with_company_name;
        address_with_company_name = address_no_company_name;
        address_with_company_name.company_name = "Granule pro šneky s.r.o.";

        ::LibFred::ContactAddressList addr_list;
        addr_list[::LibFred::ContactAddressType::from_string("MAILING")] = address_no_company_name;
        addr_list[::LibFred::ContactAddressType::from_string("BILLING")] = address_no_company_name;
        addr_list[::LibFred::ContactAddressType::from_string("SHIPPING")] = address_with_company_name;
        addr_list[::LibFred::ContactAddressType::from_string("SHIPPING_2")] = address_with_company_name;
        addr_list[::LibFred::ContactAddressType::from_string("SHIPPING_3")] = address_with_company_name;

        ::LibFred::CreateContact(test_contact_handle,registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .set_addresses(addr_list)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestDeleteContact, test_contact_fixture)

/**
 * test DeleteContact
 * create test contact, delete test contact, check erdate of test contact is null
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(delete_contact)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoContactOutput contact_info1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    BOOST_CHECK(contact_info1.info_contact_data.delete_time.isnull());

    ::LibFred::DeleteContactByHandle(test_contact_handle).exec(ctx);

    std::vector<::LibFred::InfoContactOutput> contact_history_info1 = ::LibFred::InfoContactHistoryByRoid(
        contact_info1.info_contact_data.roid).exec(ctx);

    BOOST_CHECK(!contact_history_info1.at(0).info_contact_data.delete_time.isnull());

    ::LibFred::InfoContactOutput contact_info1_with_change = contact_info1;
    contact_info1_with_change.info_contact_data.delete_time = contact_history_info1.at(0).info_contact_data.delete_time;

    BOOST_CHECK(contact_info1_with_change == contact_history_info1.at(0));

    BOOST_CHECK(!contact_history_info1.at(0).info_contact_data.delete_time.isnull());

    BOOST_CHECK(contact_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!contact_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!contact_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(contact_history_info1.at(0).history_valid_from <= contact_history_info1.at(0).history_valid_to.get_value());

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_contact_handle))[0][0]));
    ctx.commit_transaction();
}//delete_contact


/**
 * test DeleteContact with wrong handle
 */

BOOST_AUTO_TEST_CASE(delete_contact_with_wrong_handle)
{
    std::string bad_test_contact_handle = std::string("bad")+test_contact_handle;
    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::DeleteContactByHandle(bad_test_contact_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::DeleteContactByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_handle());
        BOOST_CHECK(ex.get_unknown_contact_handle().compare(bad_test_contact_handle) == 0);
    }
}

/**
 * test DeleteContact linked
 */

BOOST_AUTO_TEST_CASE(delete_linked_contact)
{
    {
        namespace ip = boost::asio::ip;

        ::LibFred::OperationContextCreator ctx;
        //create linked object
        std::string test_nsset_handle = std::string("TEST-NSSET-HANDLE")+xmark;
        ::LibFred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(test_contact_handle))
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

       ctx.commit_transaction();
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::DeleteContactByHandle(test_contact_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::DeleteContactByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_linked_to_contact_handle());
        BOOST_CHECK(ex.get_object_linked_to_contact_handle().compare(test_contact_handle) == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestDeleteContact
