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
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/delete_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_diff.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/delete_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_diff.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-update-contact";

struct update_contact_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;
    ::LibFred::Contact::PlaceAddress place;
    ::LibFred::ContactAddressList addresses;

    update_contact_fixture()
    : xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::ContactAddress address;
        address.street1 = "Měnitelná 1";
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
        ::LibFred::CreateContact(test_contact_handle,registrar_handle)
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
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoContactOutput info_data_1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<::LibFred::InfoContactOutput> history_info_data_1 = ::LibFred::InfoContactHistoryByRoid(info_data_1.info_contact_data.roid).exec(ctx);

    //update_registrar_handle check
    BOOST_CHECK(info_data_1.info_contact_data.update_registrar_handle.isnull());

    //update_time
    BOOST_CHECK(info_data_1.info_contact_data.update_time.isnull());

    //history check
    BOOST_CHECK(history_info_data_1.at(0) == info_data_1);

    BOOST_TEST_MESSAGE(std::string("history_info_data_1.at(0).info_contact_data.crhistoryid: ") + boost::lexical_cast<std::string>(history_info_data_1.at(0).info_contact_data.crhistoryid ));
    BOOST_TEST_MESSAGE(std::string("info_data_1.info_contact_data.historyid: ") + boost::lexical_cast<std::string>(info_data_1.info_contact_data.historyid ));

    BOOST_CHECK(history_info_data_1.at(0).info_contact_data.crhistoryid == info_data_1.info_contact_data.historyid);

    //empty update
    ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle).exec(ctx);

    ::LibFred::InfoContactOutput info_data_2 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<::LibFred::InfoContactOutput> history_info_data_2 = ::LibFred::InfoContactHistoryByRoid(info_data_1.info_contact_data.roid).exec(ctx);

    ::LibFred::InfoContactOutput info_data_1_with_changes = info_data_1;

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

    ::LibFred::UpdateContactByHandle(test_contact_handle//handle
            , registrar_handle//registrar
            , Optional< std::string >()//authinfo
            , Optional< Nullable< std::string > >()//name
            , Optional< Nullable< std::string > >()//organization
            , Optional< Nullable< ::LibFred::Contact::PlaceAddress > >()//place
            , Optional< Nullable< std::string > >()//telephone
            , Optional< Nullable< std::string > >()//fax
            , Optional< Nullable< std::string > >()//email
            , Optional< Nullable< std::string > >()//notifyemail
            , Optional< Nullable< std::string > >()//vat
            , Optional< Nullable< ::LibFred::PersonalIdUnion > >()//personal_id
            , ::LibFred::ContactAddressToUpdate()//addresses
            , Optional<bool>()//disclosename
            , Optional<bool>()//discloseorganization
            , Optional<bool>()//discloseaddress
            , Optional<bool>()//disclosetelephone
            , Optional<bool>()//disclosefax
            , Optional<bool>()//discloseemail
            , Optional<bool>()//disclosevat
            , Optional<bool>()//discloseident
            , Optional<bool>()//disclosenotifyemail
            , Optional< Nullable< bool > >()//domain_expiration_letter_flag
            , Optional<unsigned long long>() //logd_request_id
            ).exec(ctx);

    ::LibFred::InfoContactOutput info_data_3 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<::LibFred::InfoContactOutput> history_info_data_3 = ::LibFred::InfoContactHistoryByRoid(info_data_1.info_contact_data.roid).exec(ctx);

    ::LibFred::InfoContactOutput info_data_2_with_changes = info_data_2;

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

    ::LibFred::Contact::PlaceAddress place;
    place.street1 = "Str 1";
    place.street2 = std::string("str2");
    place.city = "Prague";
    place.postalcode = "11150";
    place.country = "Czech Republic";
    ::LibFred::ContactAddressToUpdate addresses_to_update;
    addresses_to_update.remove< ::LibFred::ContactAddressType::SHIPPING >();
    addresses_to_update.remove< ::LibFred::ContactAddressType::SHIPPING_2 >();
    addresses_to_update.remove< ::LibFred::ContactAddressType::MAILING >();
    ::LibFred::ContactAddress new_address = addresses[::LibFred::ContactAddressType::MAILING];
    new_address.company_name = Optional< std::string >();
    new_address.street1 = "Změněná 1";
    addresses_to_update.update< ::LibFred::ContactAddressType::MAILING >(new_address);
    ::LibFred::UpdateContactByHandle(test_contact_handle//handle
            , registrar_handle//registrar
                , Optional<std::string>("passwd")//authinfo
                , Optional< Nullable< std::string > >("Test Name")//name
                , Optional< Nullable< std::string > >("Test o.r.g.")//organization
                , place//place
                , Optional< Nullable< std::string > >("+420.123456789")//telephone
                , Optional< Nullable< std::string > >()//fax
                , Optional< Nullable< std::string > >("test@nic.cz")//email
                , Optional< Nullable< std::string > >("notif-test@nic.cz")//notifyemail
                , Optional< Nullable< std::string > >("7805962556")//vat is TID
                , Optional< Nullable< ::LibFred::PersonalIdUnion > >(::LibFred::PersonalIdUnion::get_ICO("7805962556"))//ssn
                , addresses_to_update//addresses MAILING change, BILLING don't touch, SHIPPING remove, SHIPPING_2 remove
                , Optional<bool>(true)//disclosename
                , Optional<bool>(true)//discloseorganization
                , Optional<bool>(true)//discloseaddress
                , Optional<bool>(true)//disclosetelephone
                , Optional<bool>(false)//disclosefax
                , Optional<bool>(true)//discloseemail
                , Optional<bool>(true)//disclosevat
                , Optional<bool>(true)//discloseident
                , Optional<bool>(false)//disclosenotifyemail
                , Optional<Nullable<bool> >()//domain_expiration_letter_flag
                , Optional<unsigned long long>(0) //logd_request_id
                ).exec(ctx);

    ::LibFred::InfoContactOutput info_data_4 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<::LibFred::InfoContactOutput> history_info_data_4 = ::LibFred::InfoContactHistoryByRoid(info_data_1.info_contact_data.roid).exec(ctx);

    ::LibFred::InfoContactOutput info_data_3_with_changes = info_data_3;

    //updated addresses
    BOOST_CHECK(info_data_4.info_contact_data.addresses.size() == 3);//MAILING, BILLING, SHIPPING_3
    BOOST_CHECK(info_data_4.info_contact_data.addresses[::LibFred::ContactAddressType::MAILING] == new_address);
    BOOST_CHECK(info_data_4.info_contact_data.addresses[::LibFred::ContactAddressType::BILLING] == addresses[::LibFred::ContactAddressType::BILLING]);
    BOOST_CHECK(info_data_4.info_contact_data.addresses.find(::LibFred::ContactAddressType::SHIPPING) == info_data_4.info_contact_data.addresses.end());
    BOOST_CHECK(info_data_4.info_contact_data.addresses.find(::LibFred::ContactAddressType::SHIPPING_2) == info_data_4.info_contact_data.addresses.end());

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
    BOOST_CHECK(info_data_3_with_changes != info_data_4);
    info_data_3_with_changes.info_contact_data.addresses.erase(::LibFred::ContactAddressType::SHIPPING);
    info_data_3_with_changes.info_contact_data.addresses.erase(::LibFred::ContactAddressType::SHIPPING_2);
    info_data_3_with_changes.info_contact_data.addresses[::LibFred::ContactAddressType::MAILING] = new_address;
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
    ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle)
    .set_authinfo("passw")
    .set_name("Test Name")
    .set_organization("Test o.r.g.")
    .set_place(place)
    .set_telephone("+420.123456789")
    .set_fax("")
    .set_email("test@nic.cz")
    .set_notifyemail("notif-test@nic.cz")
    .set_vat("7805962556")
    .set_personal_id(::LibFred::PersonalIdUnion::get_ICO("7805962556"))
    .set_address< ::LibFred::ContactAddressType::MAILING >(new_address)
    .set_address< ::LibFred::ContactAddressType::SHIPPING >(new_address)
    .set_address< ::LibFred::ContactAddressType::SHIPPING_2 >(new_address)
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

    ::LibFred::InfoContactOutput info_data_5 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<::LibFred::InfoContactOutput> history_info_data_5 = ::LibFred::InfoContactHistoryByRoid(info_data_1.info_contact_data.roid).exec(ctx);

    ::LibFred::InfoContactOutput info_data_4_with_changes = info_data_4;

    //updated addresses
    BOOST_CHECK(info_data_5.info_contact_data.addresses.size() == 5);//MAILING, BILLING, SHIPPING, SHIPPING_2, SHIPPING_3
    BOOST_CHECK(info_data_5.info_contact_data.addresses[::LibFred::ContactAddressType::MAILING] == new_address);
    BOOST_CHECK(info_data_5.info_contact_data.addresses[::LibFred::ContactAddressType::BILLING] == addresses[::LibFred::ContactAddressType::BILLING]);
    BOOST_CHECK(info_data_5.info_contact_data.addresses[::LibFred::ContactAddressType::SHIPPING] == new_address);
    BOOST_CHECK(info_data_5.info_contact_data.addresses[::LibFred::ContactAddressType::SHIPPING_2] == new_address);

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
    BOOST_CHECK(info_data_4_with_changes != info_data_5);
    info_data_4_with_changes.info_contact_data.addresses[::LibFred::ContactAddressType::MAILING] = new_address;
    info_data_4_with_changes.info_contact_data.addresses[::LibFred::ContactAddressType::SHIPPING] = new_address;
    info_data_4_with_changes.info_contact_data.addresses[::LibFred::ContactAddressType::SHIPPING_2] = new_address;
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
    ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle)
    .reset_address< ::LibFred::ContactAddressType::MAILING >()
    .reset_address< ::LibFred::ContactAddressType::BILLING >()
    .reset_address< ::LibFred::ContactAddressType::SHIPPING >()
    .reset_address< ::LibFred::ContactAddressType::SHIPPING_2 >()
    .reset_address< ::LibFred::ContactAddressType::SHIPPING_3 >()
    .exec(ctx);

    ::LibFred::InfoContactOutput info_data_6 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);

    //updated addresses
    BOOST_CHECK(info_data_6.info_contact_data.addresses.empty());//no additional address

    //insert more then one additional address (INSERT ... VALUES (...),(...),...)
    ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle)
    .set_address< ::LibFred::ContactAddressType::MAILING >(addresses[::LibFred::ContactAddressType::MAILING])
    .set_address< ::LibFred::ContactAddressType::BILLING >(addresses[::LibFred::ContactAddressType::BILLING])
    .set_address< ::LibFred::ContactAddressType::SHIPPING >(addresses[::LibFred::ContactAddressType::SHIPPING])
    .set_address< ::LibFred::ContactAddressType::SHIPPING_2 >(addresses[::LibFred::ContactAddressType::SHIPPING_2])
    .set_address< ::LibFred::ContactAddressType::SHIPPING_3 >(addresses[::LibFred::ContactAddressType::SHIPPING_3])
    .exec(ctx);

    ::LibFred::InfoContactOutput info_data_7 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);

    //updated addresses
    BOOST_CHECK(info_data_7.info_contact_data.addresses.size() == 5);//MAILING, BILLING, SHIPPING, SHIPING_2, SHIPPING_3
    BOOST_CHECK(info_data_7.info_contact_data.addresses[::LibFred::ContactAddressType::MAILING] == addresses[::LibFred::ContactAddressType::MAILING]);
    BOOST_CHECK(info_data_7.info_contact_data.addresses[::LibFred::ContactAddressType::BILLING] == addresses[::LibFred::ContactAddressType::BILLING]);
    BOOST_CHECK(info_data_7.info_contact_data.addresses[::LibFred::ContactAddressType::SHIPPING] == addresses[::LibFred::ContactAddressType::SHIPPING]);
    BOOST_CHECK(info_data_7.info_contact_data.addresses[::LibFred::ContactAddressType::SHIPPING_2] == addresses[::LibFred::ContactAddressType::SHIPPING_2]);
    BOOST_CHECK(info_data_7.info_contact_data.addresses[::LibFred::ContactAddressType::SHIPPING_3] == addresses[::LibFred::ContactAddressType::SHIPPING_3]);

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
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateContactByHandle(bad_test_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateContactByHandle::ExceptionType& ex)
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
    ::LibFred::InfoContactOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateContactByHandle(test_contact_handle, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateContactByHandle::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    ::LibFred::InfoContactOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContactByHandle with wrong ssntype
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle_wrong_ssntype)
{
    ::LibFred::InfoContactOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    ::LibFred::InfoContactOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContactByHandle with wrong country
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle_wrong_country)
{
    ::LibFred::InfoContactOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        ::LibFred::Contact::PlaceAddress place;
        place.country = "bad-country";
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle)
        .set_place(place)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateContactByHandle::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_country());
        BOOST_CHECK(ex.get_unknown_country().compare("bad-country") == 0);
    }

    ::LibFred::InfoContactOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContactById
 */
BOOST_AUTO_TEST_CASE(update_contact_by_id)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoContactOutput info_data_1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    ::LibFred::Contact::PlaceAddress place = info_data_1.info_contact_data.place.get_value();
    place.street3 = Optional<std::string>("test street 3");
    ::LibFred::UpdateContactById(info_data_1.info_contact_data.id,registrar_handle).set_place(place).exec(ctx);
    ctx.commit_transaction();
}

/**
 * test UpdateContactByHandle with company_name
 */
BOOST_AUTO_TEST_CASE(update_contact_by_handle_company_name)
{
    ::LibFred::InfoContactOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    ::LibFred::ContactAddress address = info_data_1.info_contact_data.
                                       get_address< ::LibFred::ContactAddressType::SHIPPING >();
    address.company_name = "Company GmbH.";
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle)
            .set_address< ::LibFred::ContactAddressType::SHIPPING >(address)
            .exec(ctx);
        const ::LibFred::InfoContactOutput info_data_2 = ::LibFred::InfoContactByHandle(test_contact_handle).
                                                        exec(ctx);
        BOOST_CHECK(address == info_data_2.info_contact_data.
                                   get_address< ::LibFred::ContactAddressType::SHIPPING >());
        ctx.commit_transaction();
    }

    try {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle)
            .set_address< ::LibFred::ContactAddressType::BILLING >(address)
            .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateContactByHandle::ExceptionType& ex) {
        BOOST_CHECK(ex.is_set_forbidden_company_name_setting());
        BOOST_CHECK(ex.get_forbidden_company_name_setting() ==
                    ::LibFred::ContactAddressType::to_string(::LibFred::ContactAddressType::BILLING));
    }
    catch(const std::exception &ex) {
        BOOST_ERROR("unexpected exception thrown: " + std::string(ex.what()));
    }

    try {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateContactByHandle(test_contact_handle, registrar_handle)
            .set_address< ::LibFred::ContactAddressType::MAILING >(address)
            .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateContactByHandle::ExceptionType& ex) {
        BOOST_CHECK(ex.is_set_forbidden_company_name_setting());
        BOOST_CHECK(ex.get_forbidden_company_name_setting() ==
                    ::LibFred::ContactAddressType::to_string(::LibFred::ContactAddressType::MAILING));
    }
    catch(const std::exception &ex) {
        BOOST_ERROR("unexpected exception thrown: " + std::string(ex.what()));
    }
}

/**
 * test UpdateContactById with wrong database id
 */
BOOST_AUTO_TEST_CASE(update_contact_by_id_wrong_id)
{
    ::LibFred::InfoContactOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateContactById(0, registrar_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateContactById::ExceptionType& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_id());
        BOOST_CHECK(ex.get_unknown_contact_id() == 0);
    }

    ::LibFred::InfoContactOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

BOOST_AUTO_TEST_SUITE_END();//TestUpdateContact
