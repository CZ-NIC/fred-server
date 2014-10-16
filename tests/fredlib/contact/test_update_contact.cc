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
#include <fredlib/contact.h>
#include <fredlib/nsset.h>
#include <fredlib/keyset.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-update-contact";

struct update_contact_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;
    Fred::Contact::PlaceAddress place;
    Fred::ContactAddress address;
    Fred::ContactAddressList addresses;

    update_contact_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        address.company_name = "Testovací, s.r.o.";
        address.street1 = "Měnitelná 1";
        address.city = "Testín pod Testerem";
        address.stateorprovince = "Testerovo";
        address.postalcode = "32100";
        address.country = "CZ";
        addresses[Fred::ContactAddressType::from_string("MAILING")] = address;
        addresses[Fred::ContactAddressType::from_string("BILLING")] = address;
        addresses[Fred::ContactAddressType::from_string("SHIPPING")] = address;
        BOOST_CHECK(addresses.size() == 3);
        Fred::CreateContact(test_contact_handle,registrar_handle)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_addresses(addresses)
            .set_discloseaddress(true)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~update_contact_fixture()
    {}
};


BOOST_FIXTURE_TEST_SUITE(TestUpdateContact, update_contact_fixture)

/**
 * test UpdateContactByHandle
 * test UpdateContactByHandle construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle)
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput info_data_1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactOutput> history_info_data_1 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid).exec(ctx);

    //update_registrar_handle check
    BOOST_CHECK(info_data_1.info_contact_data.update_registrar_handle.isnull());

    //update_time
    BOOST_CHECK(info_data_1.info_contact_data.update_time.isnull());

    //history check
    BOOST_CHECK(history_info_data_1.at(0) == info_data_1);

    BOOST_MESSAGE(std::string("history_info_data_1.at(0).info_contact_data.crhistoryid: ") + boost::lexical_cast<std::string>(history_info_data_1.at(0).info_contact_data.crhistoryid ));
    BOOST_MESSAGE(std::string("info_data_1.info_contact_data.historyid: ") + boost::lexical_cast<std::string>(info_data_1.info_contact_data.historyid ));

    BOOST_CHECK(history_info_data_1.at(0).info_contact_data.crhistoryid == info_data_1.info_contact_data.historyid);

    //empty update
    Fred::UpdateContactByHandle(test_contact_handle, registrar_handle).exec(ctx);

    Fred::InfoContactOutput info_data_2 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactOutput> history_info_data_2 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid).exec(ctx);

    Fred::InfoContactOutput info_data_1_with_changes = info_data_1;

    //updated historyid
    BOOST_CHECK(info_data_1.info_contact_data.historyid !=info_data_2.info_contact_data.historyid);
    info_data_1_with_changes.info_contact_data.historyid = info_data_2.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == info_data_2.info_contact_data.update_registrar_handle.get_value());
    info_data_1_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_1_with_changes.info_contact_data.update_time = info_data_2.info_contact_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_1_with_changes == info_data_2);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_2.at(0) == info_data_2);
    BOOST_CHECK(history_info_data_2.at(1) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_2.at(1).info_contact_data == history_info_data_1.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_2.at(1).next_historyid.get_value() == history_info_data_2.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_2.at(0).info_contact_data.crhistoryid == info_data_2.info_contact_data.crhistoryid);

    Fred::UpdateContactByHandle(test_contact_handle//handle
            , registrar_handle//registrar
            , Optional<std::string>()//sponsoring registrar
            , Optional<std::string>()//authinfo
            , Optional<std::string>()//name
            , Optional<std::string>()//organization
            , Optional< Fred::Contact::PlaceAddress >()//place
            , Optional<std::string>()//telephone
            , Optional<std::string>()//fax
            , Optional<std::string>()//email
            , Optional<std::string>()//notifyemail
            , Optional<std::string>()//vat
            , Optional<std::string>()//ssntype
            , Optional<std::string>()//ssn
            , Fred::ContactAddressToUpdate()//addresses
            , Optional<bool>()//disclosename
            , Optional<bool>()//discloseorganization
            , Optional<bool>()//discloseaddress
            , Optional<bool>()//disclosetelephone
            , Optional<bool>()//disclosefax
            , Optional<bool>()//discloseemail
            , Optional<bool>()//disclosevat
            , Optional<bool>()//discloseident
            , Optional<bool>()//disclosenotifyemail
            , Optional<unsigned long long>() //logd_request_id
            ).exec(ctx);

    Fred::InfoContactOutput info_data_3 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactOutput> history_info_data_3 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid).exec(ctx);

    Fred::InfoContactOutput info_data_2_with_changes = info_data_2;

    //updated historyid
    BOOST_CHECK(info_data_2.info_contact_data.historyid !=info_data_3.info_contact_data.historyid);
    info_data_2_with_changes.info_contact_data.historyid = info_data_3.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == info_data_3.info_contact_data.update_registrar_handle.get_value());
    info_data_2_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_2_with_changes.info_contact_data.update_time = info_data_3.info_contact_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_2_with_changes == info_data_3);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_3.at(0) == info_data_3);
    BOOST_CHECK(history_info_data_3.at(1) == info_data_2);
    BOOST_CHECK(history_info_data_3.at(2) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_3.at(1).info_contact_data == history_info_data_2.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_3.at(1).next_historyid.get_value() == history_info_data_3.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_3.at(0).info_contact_data.crhistoryid == info_data_3.info_contact_data.crhistoryid);

    Fred::Contact::PlaceAddress place;
    place.street1 = "Str 1";
    place.street2 = std::string("str2");
    place.city = "Prague";
    place.postalcode = "11150";
    place.country = "Czech Republic";
    Fred::ContactAddressToUpdate addresses_to_update;
    addresses_to_update.remove< Fred::ContactAddressType::SHIPPING >();
    addresses_to_update.remove< Fred::ContactAddressType::MAILING >();
    Fred::ContactAddress new_address = address;
    new_address.company_name = Optional< std::string >();
    new_address.street1 = "Změněná 1";
    addresses_to_update.update< Fred::ContactAddressType::MAILING >(new_address);
    Fred::UpdateContactByHandle(test_contact_handle//handle
            , registrar_handle//registrar
                , Optional<std::string>(registrar_handle)//sponsoring registrar
                , Optional<std::string>("passwd")//authinfo
                , Optional<std::string>("Test Name")//name
                , Optional<std::string>("Test o.r.g.")//organization
                , place//place
                , Optional<std::string>("+420.123456789")//telephone
                , Optional<std::string>()//fax
                , Optional<std::string>("test@nic.cz")//email
                , Optional<std::string>("notif-test@nic.cz")//notifyemail
                , Optional<std::string>("7805962556")//vat is TID
                , Optional<std::string>("ICO")//ssntype
                , Optional<std::string>("7805962556")//ssn
                , addresses_to_update//addresses MAILING change, BILLING don't touch, SHIPPING remove
                , Optional<bool>(true)//disclosename
                , Optional<bool>(true)//discloseorganization
                , Optional<bool>(true)//discloseaddress
                , Optional<bool>(true)//disclosetelephone
                , Optional<bool>(false)//disclosefax
                , Optional<bool>(true)//discloseemail
                , Optional<bool>(true)//disclosevat
                , Optional<bool>(true)//discloseident
                , Optional<bool>(false)//disclosenotifyemail
                , Optional<unsigned long long>(0) //logd_request_id
                ).exec(ctx);

    Fred::InfoContactOutput info_data_4 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactOutput> history_info_data_4 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid).exec(ctx);

    Fred::InfoContactOutput info_data_3_with_changes = info_data_3;

    //updated addresses
    BOOST_CHECK(info_data_4.info_contact_data.addresses.size() == 2);//MAILING, BILLING
    BOOST_CHECK(info_data_4.info_contact_data.addresses[Fred::ContactAddressType::MAILING] == new_address);
    BOOST_CHECK(info_data_4.info_contact_data.addresses[Fred::ContactAddressType::BILLING] == address);
    BOOST_CHECK(info_data_4.info_contact_data.addresses.find(Fred::ContactAddressType::SHIPPING) == info_data_4.info_contact_data.addresses.end());

    //updated historyid
    BOOST_CHECK(info_data_3.info_contact_data.historyid !=info_data_4.info_contact_data.historyid);
    info_data_3_with_changes.info_contact_data.historyid = info_data_4.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == info_data_4.info_contact_data.update_registrar_handle.get_value());
    info_data_3_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_contact_data.sponsoring_registrar_handle));
    info_data_3_with_changes.info_contact_data.sponsoring_registrar_handle = registrar_handle;

    //updated update_time
    info_data_3_with_changes.info_contact_data.update_time = info_data_4.info_contact_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_3.info_contact_data.authinfopw != info_data_4.info_contact_data.authinfopw);
    BOOST_CHECK(std::string("passwd") == info_data_4.info_contact_data.authinfopw);
    info_data_3_with_changes.info_contact_data.authinfopw = std::string("passwd");
    info_data_3_with_changes.info_contact_data.name = std::string("Test Name");
    info_data_3_with_changes.info_contact_data.organization = std::string("Test o.r.g.");
    place.country = "CZ";
    info_data_3_with_changes.info_contact_data.place = place;
    info_data_3_with_changes.info_contact_data.telephone = std::string("+420.123456789");
    info_data_3_with_changes.info_contact_data.email = std::string("test@nic.cz");
    info_data_3_with_changes.info_contact_data.notifyemail = std::string("notif-test@nic.cz");
    info_data_3_with_changes.info_contact_data.vat = std::string("7805962556");
    info_data_3_with_changes.info_contact_data.ssntype = std::string("ICO");
    info_data_3_with_changes.info_contact_data.ssn = std::string("7805962556");

    info_data_3_with_changes.info_contact_data.disclosename = true;
    info_data_3_with_changes.info_contact_data.discloseorganization = true;
    info_data_3_with_changes.info_contact_data.discloseaddress = true;
    info_data_3_with_changes.info_contact_data.disclosetelephone = true;
    info_data_3_with_changes.info_contact_data.disclosefax = false;
    info_data_3_with_changes.info_contact_data.discloseemail = true;
    info_data_3_with_changes.info_contact_data.disclosevat = true;
    info_data_3_with_changes.info_contact_data.discloseident = true;
    info_data_3_with_changes.info_contact_data.disclosenotifyemail = false;

    //check changes made by last update
    BOOST_CHECK(info_data_3_with_changes == info_data_4);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_4.at(0) == info_data_4);
    BOOST_CHECK(history_info_data_4.at(1) == info_data_3);
    BOOST_CHECK(history_info_data_4.at(2) == info_data_2);
    BOOST_CHECK(history_info_data_4.at(3) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_4.at(1).info_contact_data == history_info_data_3.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_4.at(1).next_historyid.get_value() == history_info_data_4.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_4.at(0).info_contact_data.crhistoryid == info_data_4.info_contact_data.crhistoryid);

    place.street3 = Optional<std::string>("");
    new_address.street1 = "Vrácená 1";
    Fred::UpdateContactByHandle(test_contact_handle, registrar_handle)
    .set_sponsoring_registrar(registrar_handle)
    .set_authinfo("passw")
    .set_name("Test Name")
    .set_organization("Test o.r.g.")
    .set_place(place)
    .set_telephone("+420.123456789")
    .set_fax("")
    .set_email("test@nic.cz")
    .set_notifyemail("notif-test@nic.cz")
    .set_vat("7805962556")
    .set_ssntype("ICO")
    .set_ssn("7805962556")
    .set_address< Fred::ContactAddressType::MAILING >(new_address)
    .set_address< Fred::ContactAddressType::SHIPPING >(new_address)
    .set_disclosename(true)
    .set_discloseorganization(true)
    .set_discloseaddress(true)
    .set_disclosetelephone(true)
    .set_disclosefax(false)
    .set_discloseemail(true)
    .set_disclosevat(true)
    .set_discloseident(true)
    .set_disclosenotifyemail(false)
    .set_logd_request_id(4)
    .exec(ctx);

    Fred::InfoContactOutput info_data_5 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactOutput> history_info_data_5 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid).exec(ctx);

    Fred::InfoContactOutput info_data_4_with_changes = info_data_4;

    //updated addresses
    BOOST_CHECK(info_data_5.info_contact_data.addresses.size() == 3);//MAILING, BILLING, SHIPPING
    BOOST_CHECK(info_data_5.info_contact_data.addresses[Fred::ContactAddressType::MAILING] == new_address);
    BOOST_CHECK(info_data_5.info_contact_data.addresses[Fred::ContactAddressType::BILLING] == address);
    BOOST_CHECK(info_data_5.info_contact_data.addresses[Fred::ContactAddressType::SHIPPING] == new_address);

    //updated historyid
    BOOST_CHECK(info_data_4.info_contact_data.historyid !=info_data_5.info_contact_data.historyid);
    info_data_4_with_changes.info_contact_data.historyid = info_data_5.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == info_data_5.info_contact_data.update_registrar_handle.get_value());
    info_data_4_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_contact_data.sponsoring_registrar_handle));
    info_data_4_with_changes.info_contact_data.sponsoring_registrar_handle = registrar_handle;

    //updated update_time
    info_data_4_with_changes.info_contact_data.update_time = info_data_5.info_contact_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_4.info_contact_data.authinfopw != info_data_5.info_contact_data.authinfopw);
    BOOST_CHECK(std::string("passw") == info_data_5.info_contact_data.authinfopw);
    info_data_4_with_changes.info_contact_data.authinfopw = std::string("passw");

    //empty string in street3 and fax
    place = info_data_4_with_changes.info_contact_data.place.get_value();
    place.street3 = std::string("");
    info_data_4_with_changes.info_contact_data.place = place;
    info_data_4_with_changes.info_contact_data.fax = std::string("");

    //check logd request_id
    BOOST_CHECK(4 == history_info_data_5.at(0).logd_request_id.get_value());

    //check changes made by last update
    BOOST_CHECK(info_data_4_with_changes == info_data_5);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_5.at(0) == info_data_5);
    BOOST_CHECK(history_info_data_5.at(1) == info_data_4);
    BOOST_CHECK(history_info_data_5.at(2) == info_data_3);
    BOOST_CHECK(history_info_data_5.at(3) == info_data_2);
    BOOST_CHECK(history_info_data_5.at(4) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_5.at(1).info_contact_data == history_info_data_4.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_5.at(1).next_historyid.get_value() == history_info_data_5.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_5.at(0).info_contact_data.crhistoryid == info_data_5.info_contact_data.crhistoryid);

    //remove more then one additional address (DELETE ... WHERE ... type IN ($2,$3,...))
    Fred::UpdateContactByHandle(test_contact_handle, registrar_handle)
    .reset_address< Fred::ContactAddressType::MAILING >()
    .reset_address< Fred::ContactAddressType::BILLING >()
    .reset_address< Fred::ContactAddressType::SHIPPING >()
    .exec(ctx);

    Fred::InfoContactOutput info_data_6 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);

    //updated addresses
    BOOST_CHECK(info_data_6.info_contact_data.addresses.empty());//no additional address

    //insert more then one additional address (INSERT ... VALUES (...),(...),...)
    Fred::UpdateContactByHandle(test_contact_handle, registrar_handle)
    .set_address< Fred::ContactAddressType::MAILING >(address)
    .set_address< Fred::ContactAddressType::BILLING >(address)
    .set_address< Fred::ContactAddressType::SHIPPING >(address)
    .exec(ctx);

    Fred::InfoContactOutput info_data_7 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);

    //updated addresses
    BOOST_CHECK(info_data_7.info_contact_data.addresses.size() == 3);//MAILING, BILLING, SHIPPING
    BOOST_CHECK(info_data_7.info_contact_data.addresses[Fred::ContactAddressType::MAILING] == address);
    BOOST_CHECK(info_data_7.info_contact_data.addresses[Fred::ContactAddressType::BILLING] == address);
    BOOST_CHECK(info_data_7.info_contact_data.addresses[Fred::ContactAddressType::SHIPPING] == address);

    ctx.commit_transaction();
}//update_contact_by_handle

/**
 * test UpdateContactByHandle with wrong handle
 */

BOOST_AUTO_TEST_CASE(update_contact_by_handle_wrong_handle)
{
    std::string bad_test_contact_handle = std::string("bad")+test_contact_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContactByHandle(bad_test_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::UpdateContactByHandle::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_handle());
        BOOST_CHECK(static_cast<std::string>(ex.get_unknown_contact_handle()).compare(bad_test_contact_handle) == 0);
    }
}

/**
 * test UpdateContactByHandle with wrong registrar
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle_wrong_registrar)
{
    std::string bad_registrar_handle = registrar_handle+xmark;
    Fred::InfoContactOutput info_data_1;
    {
        Fred::OperationContext ctx;
        info_data_1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContactByHandle(test_contact_handle, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::UpdateContactByHandle::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoContactOutput info_data_2;
    {
        Fred::OperationContext ctx;
        info_data_2 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContactByHandle with wrong sponsoring registrar
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle_wrong_sponsoring_registrar)
{
    std::string bad_registrar_handle = registrar_handle+xmark;
    Fred::InfoContactOutput info_data_1;
    {
        Fred::OperationContext ctx;
        info_data_1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContactByHandle(test_contact_handle, registrar_handle)
            .set_sponsoring_registrar(bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::UpdateContactByHandle::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_sponsoring_registrar_handle());
        BOOST_CHECK(ex.get_unknown_sponsoring_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoContactOutput info_data_2;
    {
        Fred::OperationContext ctx;
        info_data_2 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}


/**
 * test UpdateContactByHandle with wrong ssntype
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle_wrong_ssntype)
{
    Fred::InfoContactOutput info_data_1;
    {
        Fred::OperationContext ctx;
        info_data_1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContactByHandle(test_contact_handle, registrar_handle)
        .set_ssntype("bad-ssntype")
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::UpdateContactByHandle::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_ssntype());
        BOOST_CHECK(ex.get_unknown_ssntype().compare("bad-ssntype") == 0);
    }

    Fred::InfoContactOutput info_data_2;
    {
        Fred::OperationContext ctx;
        info_data_2 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContactByHandle with wrong country
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle_wrong_country)
{
    Fred::InfoContactOutput info_data_1;
    {
        Fred::OperationContext ctx;
        info_data_1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        Fred::Contact::PlaceAddress place;
        place.country = "bad-country";
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContactByHandle(test_contact_handle, registrar_handle)
        .set_place(place)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::UpdateContactByHandle::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_country());
        BOOST_CHECK(ex.get_unknown_country().compare("bad-country") == 0);
    }

    Fred::InfoContactOutput info_data_2;
    {
        Fred::OperationContext ctx;
        info_data_2 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContactById
 */

BOOST_AUTO_TEST_CASE(update_contact_by_id)
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput info_data_1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    Fred::Contact::PlaceAddress place = info_data_1.info_contact_data.place.get_value();
    place.street3 = Optional<std::string>("test street 3");
    Fred::UpdateContactById(info_data_1.info_contact_data.id,registrar_handle).set_place(place).exec(ctx);
    ctx.commit_transaction();
}

/**
 * test UpdateContactById with wrong database id
 */
BOOST_AUTO_TEST_CASE(update_contact_by_id_wrong_id)
{
    Fred::InfoContactOutput info_data_1;
    {
        Fred::OperationContext ctx;
        info_data_1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContactById(0, registrar_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::UpdateContactById::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_id());
        BOOST_CHECK(ex.get_unknown_contact_id() == 0);
    }

    Fred::InfoContactOutput info_data_2;
    {
        Fred::OperationContext ctx;
        info_data_2 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

BOOST_AUTO_TEST_SUITE_END();//TestUpdateContact
