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
#include "util/random_data_generator.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-create-contact";

struct create_contact_fixture : public virtual Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string create_contact_handle;
    std::string contact_name;
    ::LibFred::Contact::PlaceAddress place;
    ::LibFred::ContactAddressList addresses;

    create_contact_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , create_contact_handle(std::string("TEST-CREATE-CONTACT-HANDLE") + xmark)
    , contact_name(std::string("TEST-CREATE-CONTACT NAME") + xmark)
    {
        {
            ::LibFred::OperationContextCreator ctx;
            registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        }
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar
        place.street1 = "\"Hlavní\"";
        place.street2 = "adresa";
        place.street3 = "kontaktu";
        place.city = "Testín 10";
        place.stateorprovince = "Testerovo";
        place.postalcode = "32100";
        place.country = "CZ";
        ::LibFred::ContactAddress address;
        address.street1 = "Testosteronová 5";
        address.city = "Testín pod Testerem";
        address.stateorprovince = "Testerovo";
        address.postalcode = "32100";
        address.country = "CZ";
        addresses[::LibFred::ContactAddressType::from_string("MAILING")] = address;
        addresses[::LibFred::ContactAddressType::from_string("BILLING")] = address;
        address.company_name = "Testovací & doručovací, s.r.o.";
        addresses[::LibFred::ContactAddressType::from_string("SHIPPING")] = address;
        addresses[::LibFred::ContactAddressType::from_string("SHIPPING_2")] = address;
        addresses[::LibFred::ContactAddressType::from_string("SHIPPING_3")] = address;
        BOOST_CHECK(addresses.size() == 5);
    }
    ~create_contact_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateContact, create_contact_fixture)

DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);

/**
 * test CreateContact with wrong registrar
 */
BOOST_AUTO_TEST_CASE(create_contact_wrong_registrar)
{
    ::LibFred::OperationContextCreator ctx;

    std::string bad_registrar_handle = registrar_handle+xmark;
    BOOST_CHECK_EXCEPTION(
    try
    {
        ::LibFred::CreateContact(create_contact_handle, bad_registrar_handle)
        .set_authinfo("testauthinfo")
        .set_logd_request_id(0)
        .exec(ctx);
    }
    catch(const ::LibFred::CreateContact::Exception& ex)
    {
        ex << ErrorInfo_unknown_registrar_handle("modifying const EX& by operator<<");
        //ex.set_internal_error("unable to modify const EX& by setter - ok");
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        throw;
    }
    , std::exception
    , check_std_exception);
}

/**
 * test CreateContact with wrong ssntype
 */
BOOST_AUTO_TEST_CASE(create_contact_wrong_ssntype)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK_EXCEPTION(
    try
    {
        ::LibFred::CreateContact(create_contact_handle, registrar_handle)
        .set_authinfo("testauthinfo")
        .set_logd_request_id(0)
        .set_ssntype("BAD")
        .set_ssn("any")
        .exec(ctx);
    }
    catch(const ::LibFred::CreateContact::Exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_unknown_ssntype());
        throw;
    }
    , std::exception
    , check_std_exception);
}

/**
 * test CreateContact with wrong non-shipping address
 */
BOOST_AUTO_TEST_CASE(create_contact_wrong_address)
{
    BOOST_CHECK_EXCEPTION(
    try {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::ContactAddressList bad_addresses = addresses;
        bad_addresses[::LibFred::ContactAddressType::MAILING].company_name = "Výjimečná & korespondenční, s.r.o.";
        ::LibFred::CreateContact(create_contact_handle, registrar_handle)
        .set_authinfo("testauthinfo")
        .set_addresses(bad_addresses)
        .set_logd_request_id(0)
        .exec(ctx);
    }
    catch(const ::LibFred::CreateContact::Exception& ex) {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_forbidden_company_name_setting());
        throw;
    }
    , std::exception
    , check_std_exception);

    BOOST_CHECK_EXCEPTION(
    try {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::ContactAddressList bad_addresses = addresses;
        bad_addresses[::LibFred::ContactAddressType::BILLING].company_name = "Výjimečná & fakturační, s.r.o.";
        ::LibFred::CreateContact(create_contact_handle, registrar_handle)
        .set_authinfo("testauthinfo")
        .set_addresses(bad_addresses)
        .set_logd_request_id(0)
        .exec(ctx);
    }
    catch(const ::LibFred::CreateContact::Exception& ex) {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_forbidden_company_name_setting());
        throw;
    }
    , std::exception
    , check_std_exception);
}

/**
 * test CreateContact with regular data and full available set of address types
 */
BOOST_FIXTURE_TEST_CASE(create_contact_ok, create_contact_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::CreateContact(create_contact_handle, registrar_handle)
    .set_authinfo("testauthinfo")
    .set_logd_request_id(0)
    .set_name(contact_name)
    .set_place(place)
    .set_ssntype("RC")
    .set_ssn("121110/0121")
    .set_addresses(addresses)
    .exec(ctx);
    const ::LibFred::InfoContactOutput output = ::LibFred::InfoContactByHandle(create_contact_handle).exec(ctx);
    ctx.commit_transaction();
    BOOST_CHECK(output.info_contact_data.addresses.size() == addresses.size());
    for (::LibFred::ContactAddressList::const_iterator address_ptr = output.info_contact_data.addresses.begin();
         address_ptr != output.info_contact_data.addresses.end(); ++address_ptr) {
        BOOST_CHECK(address_ptr->second == addresses[address_ptr->first]);
        BOOST_CHECK(address_ptr->second.company_name == addresses[address_ptr->first].company_name);
        BOOST_CHECK(address_ptr->second.street1 == addresses[address_ptr->first].street1);
        BOOST_CHECK(address_ptr->second.street2 == addresses[address_ptr->first].street2);
        BOOST_CHECK(address_ptr->second.street3 == addresses[address_ptr->first].street3);
        BOOST_CHECK(address_ptr->second.city == addresses[address_ptr->first].city);
        BOOST_CHECK(address_ptr->second.stateorprovince == addresses[address_ptr->first].stateorprovince);
        BOOST_CHECK(address_ptr->second.postalcode == addresses[address_ptr->first].postalcode);
        BOOST_CHECK(address_ptr->second.country == addresses[address_ptr->first].country);
    }
}


/**
 * test CreateContact with regular data and no additional address
 */
BOOST_FIXTURE_TEST_CASE(create_contact_ok_without_addresses, create_contact_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::CreateContact(create_contact_handle, registrar_handle)
        .set_name(contact_name)
        .set_place(place)
        .exec(ctx);
    const ::LibFred::InfoContactOutput output = ::LibFred::InfoContactByHandle(create_contact_handle).exec(ctx);
    ctx.commit_transaction();
    BOOST_CHECK(output.info_contact_data.handle == create_contact_handle);
    BOOST_CHECK(output.info_contact_data.name == contact_name);
    BOOST_CHECK(output.info_contact_data.place == place);
    BOOST_CHECK(output.info_contact_data.addresses.size() == 0);
}


BOOST_AUTO_TEST_SUITE_END();//TestCreateContact
