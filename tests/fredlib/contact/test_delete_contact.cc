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
#include <boost/asio/ip/address.hpp>

#include "src/fredlib/opcontext.h"
#include <fredlib/contact.h>
#include <fredlib/nsset.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-delete-contact";

struct test_contact_fixture  : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;

    test_contact_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle  = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        Fred::ContactAddress address_no_company_name;
        address_no_company_name.street1 = "Betelgeuze 42";
        address_no_company_name.city = "Orion";
        address_no_company_name.postalcode = "11150";
        address_no_company_name.country = "MZ";

        Fred::ContactAddress address_with_company_name;
        address_with_company_name = address_no_company_name;
        address_with_company_name.company_name = "Granule pro šneky s.r.o.";

        Fred::ContactAddressList addr_list;
        addr_list[Fred::ContactAddressType::from_string("MAILING")] = address_no_company_name;
        addr_list[Fred::ContactAddressType::from_string("BILLING")] = address_no_company_name;
        addr_list[Fred::ContactAddressType::from_string("SHIPPING")] = address_with_company_name;
        addr_list[Fred::ContactAddressType::from_string("SHIPPING_2")] = address_with_company_name;
        addr_list[Fred::ContactAddressType::from_string("SHIPPING_3")] = address_with_company_name;

        Fred::CreateContact(test_contact_handle,registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
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
    Fred::OperationContext ctx;
    Fred::InfoContactOutput contact_info1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    BOOST_CHECK(contact_info1.info_contact_data.delete_time.isnull());

    Fred::DeleteContactByHandle(test_contact_handle).exec(ctx);
    ctx.commit_transaction();

    std::vector<Fred::InfoContactOutput> contact_history_info1 = Fred::InfoContactHistory(
        contact_info1.info_contact_data.roid).exec(ctx);

    BOOST_CHECK(!contact_history_info1.at(0).info_contact_data.delete_time.isnull());

    Fred::InfoContactOutput contact_info1_with_change = contact_info1;
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
}//delete_contact


/**
 * test DeleteContact with wrong handle
 */

BOOST_AUTO_TEST_CASE(delete_contact_with_wrong_handle)
{
    std::string bad_test_contact_handle = std::string("bad")+test_contact_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteContactByHandle(bad_test_contact_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteContactByHandle::Exception& ex)
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

        Fred::OperationContext ctx;
        //create linked object
        std::string test_nsset_handle = std::string("TEST-NSSET-HANDLE")+xmark;
        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(test_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

       ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteContactByHandle(test_contact_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteContactByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_linked_to_contact_handle());
        BOOST_CHECK(ex.get_object_linked_to_contact_handle().compare(test_contact_handle) == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestDeleteContact
