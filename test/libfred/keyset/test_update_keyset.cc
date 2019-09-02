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
#include "libfred/registrable_object/keyset/info_keyset_impl.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/registrable_object/uuid.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-update-keyset";

struct update_keyset_fixture : public Test::instantiate_db_template
{

    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact4_handle;
    std::string admin_contact5_handle;
    std::string admin_contact6_handle;
    std::string test_keyset_handle;

    update_keyset_fixture()
    : xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , admin_contact4_handle(std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark)
    , admin_contact5_handle(std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark)
    , admin_contact6_handle(std::string("TEST-ADMIN-CONTACT6-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
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
        ::LibFred::CreateContact(admin_contact4_handle,registrar_handle)
            .set_name(admin_contact4_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("admin_contact4_handle: ") + admin_contact4_handle);

        ::LibFred::CreateContact(admin_contact5_handle,registrar_handle)
            .set_name(admin_contact5_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("admin_contact5_handle: ") + admin_contact5_handle);

        ::LibFred::CreateContact(admin_contact6_handle,registrar_handle)
            .set_name(admin_contact6_handle+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("admin_contact6_handle: ") + admin_contact6_handle);

        ::LibFred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .set_dns_keys(Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);
        BOOST_TEST_MESSAGE(std::string("test_keyset_handle: ") + test_keyset_handle);
        ctx.commit_transaction();
    }
    ~update_keyset_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateKeyset, update_keyset_fixture)

/**
 * test UpdateKeyset
 * test UpdateKeyset construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(update_keyset)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoKeysetOutput info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_1 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    //update_registrar_handle check
    BOOST_CHECK(info_data_1.info_keyset_data.update_registrar_handle.isnull());

    //update_time
    BOOST_CHECK(info_data_1.info_keyset_data.update_time.isnull());

    //history check
    BOOST_CHECK(history_info_data_1.at(0) == info_data_1);
    BOOST_CHECK(history_info_data_1.at(0).info_keyset_data.crhistoryid == info_data_1.info_keyset_data.historyid);

    //empty update
    ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_2 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_1_with_changes = info_data_1;

    //updated historyid
    BOOST_CHECK(info_data_1.info_keyset_data.historyid !=info_data_2.info_keyset_data.historyid);
    info_data_1_with_changes.info_keyset_data.historyid = info_data_2.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_2.info_keyset_data.update_registrar_handle.get_value()));
    info_data_1_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_1_with_changes.info_keyset_data.update_time = info_data_2.info_keyset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_1_with_changes == info_data_2);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_2.at(0) == info_data_2);
    BOOST_CHECK(history_info_data_2.at(1) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_2.at(1).info_keyset_data == history_info_data_1.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_2.at(1).next_historyid.get_value() == history_info_data_2.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_2.at(0).info_keyset_data.crhistoryid == info_data_2.info_keyset_data.crhistoryid);


    ::LibFred::UpdateKeyset(test_keyset_handle//const std::string& handle
        , registrar_handle//const std::string& registrar
        , Optional<std::string>("testauthinfo")//const Optional<std::string>& authinfo
        , Util::vector_of<std::string>(admin_contact5_handle) //const std::vector<std::string>& add_tech_contact
        , Util::vector_of<std::string>(admin_contact6_handle)//const std::vector<std::string>& rem_tech_contact
        , Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "key"))//const std::vector<DnsKey>& add_dns_key
        , Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"))//const std::vector<DnsKey>& rem_dns_key
        , Optional<unsigned long long>(0)//const Optional<unsigned long long> logd_request_id
        ).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_3 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_3 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_2_with_changes = info_data_2;

    //updated historyid
    BOOST_CHECK(info_data_2.info_keyset_data.historyid !=info_data_3.info_keyset_data.historyid);
    info_data_2_with_changes.info_keyset_data.historyid = info_data_3.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_3.info_keyset_data.update_registrar_handle.get_value()));
    info_data_2_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_3.info_keyset_data.sponsoring_registrar_handle));
    info_data_2_with_changes.info_keyset_data.sponsoring_registrar_handle = registrar_handle;

    //updated update_time
    info_data_2_with_changes.info_keyset_data.update_time = info_data_3.info_keyset_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_2.info_keyset_data.authinfopw != info_data_3.info_keyset_data.authinfopw);
    BOOST_CHECK(std::string("testauthinfo") == info_data_3.info_keyset_data.authinfopw);
    info_data_2_with_changes.info_keyset_data.authinfopw = std::string("testauthinfo");

    //dnskeys
    info_data_2_with_changes.info_keyset_data.dns_keys = Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "key"));

    //tech contacts
    ::LibFred::InfoContactOutput admin_contact5_info  = ::LibFred::InfoContactByHandle(admin_contact5_handle).exec(ctx);
    info_data_2_with_changes.info_keyset_data.tech_contacts =
            Util::vector_of<::LibFred::RegistrableObject::Contact::ContactReference>(
                    ::LibFred::RegistrableObject::Contact::ContactReference(
                            admin_contact5_info.info_contact_data.id,
                            admin_contact5_info.info_contact_data.handle,
                            ::LibFred::RegistrableObject::make_uuid_of<::LibFred::Object_Type::contact>()));

    //check changes made by last update
    BOOST_CHECK(info_data_2_with_changes == info_data_3);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_3.at(0) == info_data_3);
    BOOST_CHECK(history_info_data_3.at(1) == info_data_2);
    BOOST_CHECK(history_info_data_3.at(2) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_3.at(1).info_keyset_data == history_info_data_2.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_3.at(1).next_historyid.get_value() == history_info_data_3.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_3.at(0).info_keyset_data.crhistoryid == info_data_3.info_keyset_data.crhistoryid);

    ::LibFred::UpdateKeyset(test_keyset_handle//const std::string& handle
        , registrar_handle//const std::string& registrar
        , Optional<std::string>()//const Optional<std::string>& authinfo
        , std::vector<std::string>() //const std::vector<std::string>& add_tech_contact
        , std::vector<std::string>()//const std::vector<std::string>& rem_tech_contact
        , Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "key"))//const std::vector<DnsKey>& add_dns_key
        , Util::vector_of<::LibFred::DnsKey> (::LibFred::DnsKey(257, 3, 5, "key"))//const std::vector<DnsKey>& rem_dns_key
        , Optional<unsigned long long>()//const Optional<unsigned long long> logd_request_id
        ).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_4 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_4 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_3_with_changes = info_data_3;

    //updated historyid
    BOOST_CHECK(info_data_3.info_keyset_data.historyid !=info_data_4.info_keyset_data.historyid);
    info_data_3_with_changes.info_keyset_data.historyid = info_data_4.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_keyset_data.update_registrar_handle.get_value()));
    info_data_3_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_3_with_changes.info_keyset_data.update_time = info_data_4.info_keyset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_3_with_changes == info_data_4);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_4.at(0) == info_data_4);
    BOOST_CHECK(history_info_data_4.at(1) == info_data_3);
    BOOST_CHECK(history_info_data_4.at(2) == info_data_2);
    BOOST_CHECK(history_info_data_4.at(3) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_4.at(1).info_keyset_data == history_info_data_3.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_4.at(1).next_historyid.get_value() == history_info_data_4.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_4.at(0).info_keyset_data.crhistoryid == info_data_4.info_keyset_data.crhistoryid);

    //transfer password
    ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle).set_authinfo("kukauthinfo").exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_5 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_5 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_4_with_changes = info_data_4;

    //updated historyid
    BOOST_CHECK(info_data_4.info_keyset_data.historyid !=info_data_5.info_keyset_data.historyid);
    info_data_4_with_changes.info_keyset_data.historyid = info_data_5.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_keyset_data.update_registrar_handle.get_value()));
    info_data_4_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_4_with_changes.info_keyset_data.update_time = info_data_5.info_keyset_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_4.info_keyset_data.authinfopw != info_data_5.info_keyset_data.authinfopw);
    BOOST_CHECK(std::string("kukauthinfo") == info_data_5.info_keyset_data.authinfopw);
    info_data_4_with_changes.info_keyset_data.authinfopw = std::string("kukauthinfo");

    //check changes made by last update
    BOOST_CHECK(info_data_4_with_changes == info_data_5);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_5.at(0) == info_data_5);
    BOOST_CHECK(history_info_data_5.at(1) == info_data_4);
    BOOST_CHECK(history_info_data_5.at(2) == info_data_3);
    BOOST_CHECK(history_info_data_5.at(3) == info_data_2);
    BOOST_CHECK(history_info_data_5.at(4) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_5.at(1).info_keyset_data == history_info_data_4.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_5.at(1).next_historyid.get_value() == history_info_data_5.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_5.at(0).info_keyset_data.crhistoryid == info_data_5.info_keyset_data.crhistoryid);

    //add tech contact
    ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle).add_tech_contact(admin_contact4_handle).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_6 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_6 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_5_with_changes = info_data_5;

    //updated historyid
    BOOST_CHECK(info_data_5.info_keyset_data.historyid !=info_data_6.info_keyset_data.historyid);
    info_data_5_with_changes.info_keyset_data.historyid = info_data_6.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_6.info_keyset_data.update_registrar_handle.get_value()));
    info_data_5_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_5_with_changes.info_keyset_data.update_time = info_data_6.info_keyset_data.update_time;

    //add tech contact
    ::LibFred::InfoContactOutput admin_contact4_info  = ::LibFred::InfoContactByHandle(admin_contact4_handle).exec(ctx);
    info_data_5_with_changes.info_keyset_data.tech_contacts.push_back(::LibFred::ObjectIdHandlePair(
        admin_contact4_info.info_contact_data.id, admin_contact4_info.info_contact_data.handle));

    //check changes made by last update
    BOOST_CHECK(info_data_5_with_changes == info_data_6);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_6.at(0) == info_data_6);
    BOOST_CHECK(history_info_data_6.at(1) == info_data_5);
    BOOST_CHECK(history_info_data_6.at(2) == info_data_4);
    BOOST_CHECK(history_info_data_6.at(3) == info_data_3);
    BOOST_CHECK(history_info_data_6.at(4) == info_data_2);
    BOOST_CHECK(history_info_data_6.at(5) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_6.at(1).info_keyset_data == history_info_data_5.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_6.at(1).next_historyid.get_value() == history_info_data_6.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_6.at(0).info_keyset_data.crhistoryid == info_data_6.info_keyset_data.crhistoryid);

    //remove tech contact
    ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle).rem_tech_contact(admin_contact5_handle).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_7 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_7 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_6_with_changes = info_data_6;

    //updated historyid
    BOOST_CHECK(info_data_6.info_keyset_data.historyid !=info_data_7.info_keyset_data.historyid);
    info_data_6_with_changes.info_keyset_data.historyid = info_data_7.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_7.info_keyset_data.update_registrar_handle.get_value()));
    info_data_6_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_6_with_changes.info_keyset_data.update_time = info_data_7.info_keyset_data.update_time;

    //rem tech contact
    info_data_6_with_changes.info_keyset_data.tech_contacts.erase(
        std::remove(info_data_6_with_changes.info_keyset_data.tech_contacts.begin(),
            info_data_6_with_changes.info_keyset_data.tech_contacts.end(),
            ::LibFred::ObjectIdHandlePair(admin_contact5_info.info_contact_data.id,
                    admin_contact5_info.info_contact_data.handle)),
        info_data_6_with_changes.info_keyset_data.tech_contacts.end());

    //check changes made by last update
    BOOST_CHECK(info_data_6_with_changes == info_data_7);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_7.at(0) == info_data_7);
    BOOST_CHECK(history_info_data_7.at(1) == info_data_6);
    BOOST_CHECK(history_info_data_7.at(2) == info_data_5);
    BOOST_CHECK(history_info_data_7.at(3) == info_data_4);
    BOOST_CHECK(history_info_data_7.at(4) == info_data_3);
    BOOST_CHECK(history_info_data_7.at(5) == info_data_2);
    BOOST_CHECK(history_info_data_7.at(6) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_7.at(1).info_keyset_data == history_info_data_6.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_7.at(1).next_historyid.get_value() == history_info_data_7.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_7.at(0).info_keyset_data.crhistoryid == info_data_7.info_keyset_data.crhistoryid);

    //add dnskey
    ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle).add_dns_key(::LibFred::DnsKey(257, 3, 5, "key2")).add_dns_key(::LibFred::DnsKey(257, 3, 5, "key3")).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_8 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_8 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_7_with_changes = info_data_7;

    //updated historyid
    BOOST_CHECK(info_data_7.info_keyset_data.historyid !=info_data_8.info_keyset_data.historyid);
    info_data_7_with_changes.info_keyset_data.historyid = info_data_8.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_8.info_keyset_data.update_registrar_handle.get_value()));
    info_data_7_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_7_with_changes.info_keyset_data.update_time = info_data_8.info_keyset_data.update_time;

    //add dnskey
    info_data_7_with_changes.info_keyset_data.dns_keys.push_back(::LibFred::DnsKey(257, 3, 5, "key2"));
    info_data_7_with_changes.info_keyset_data.dns_keys.push_back(::LibFred::DnsKey(257, 3, 5, "key3"));

    //check changes made by last update
    BOOST_CHECK(info_data_7_with_changes == info_data_8);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_8.at(0) == info_data_8);
    BOOST_CHECK(history_info_data_8.at(1) == info_data_7);
    BOOST_CHECK(history_info_data_8.at(2) == info_data_6);
    BOOST_CHECK(history_info_data_8.at(3) == info_data_5);
    BOOST_CHECK(history_info_data_8.at(4) == info_data_4);
    BOOST_CHECK(history_info_data_8.at(5) == info_data_3);
    BOOST_CHECK(history_info_data_8.at(6) == info_data_2);
    BOOST_CHECK(history_info_data_8.at(7) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_8.at(1).info_keyset_data == history_info_data_7.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_8.at(1).next_historyid.get_value() == history_info_data_8.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_8.at(0).info_keyset_data.crhistoryid == info_data_8.info_keyset_data.crhistoryid);

    //remove dnskey
    ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle).rem_dns_key(::LibFred::DnsKey(257, 3, 5, "key")).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_9 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_9 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_8_with_changes = info_data_8;

    //updated historyid
    BOOST_CHECK(info_data_8.info_keyset_data.historyid !=info_data_9.info_keyset_data.historyid);
    info_data_8_with_changes.info_keyset_data.historyid = info_data_9.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_9.info_keyset_data.update_registrar_handle.get_value()));
    info_data_8_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_8_with_changes.info_keyset_data.update_time = info_data_9.info_keyset_data.update_time;

    //rem dnskey
    info_data_8_with_changes.info_keyset_data.dns_keys.erase(
        std::remove(info_data_8_with_changes.info_keyset_data.dns_keys.begin()
        , info_data_8_with_changes.info_keyset_data.dns_keys.end(), ::LibFred::DnsKey(257, 3, 5, "key"))
        , info_data_8_with_changes.info_keyset_data.dns_keys.end());

    //check changes made by last update
    BOOST_CHECK(info_data_8_with_changes == info_data_9);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_9.at(0) == info_data_9);
    BOOST_CHECK(history_info_data_9.at(1) == info_data_8);
    BOOST_CHECK(history_info_data_9.at(2) == info_data_7);
    BOOST_CHECK(history_info_data_9.at(3) == info_data_6);
    BOOST_CHECK(history_info_data_9.at(4) == info_data_5);
    BOOST_CHECK(history_info_data_9.at(5) == info_data_4);
    BOOST_CHECK(history_info_data_9.at(6) == info_data_3);
    BOOST_CHECK(history_info_data_9.at(7) == info_data_2);
    BOOST_CHECK(history_info_data_9.at(8) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_9.at(1).info_keyset_data == history_info_data_8.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_9.at(1).next_historyid.get_value() == history_info_data_9.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_9.at(0).info_keyset_data.crhistoryid == info_data_9.info_keyset_data.crhistoryid);

    //request_id
    ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle).set_logd_request_id(10).exec(ctx);

    ::LibFred::InfoKeysetOutput info_data_10 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    std::vector<::LibFred::InfoKeysetOutput> history_info_data_10 = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);

    //request id
    BOOST_CHECK(history_info_data_10.at(0).logd_request_id.get_value() == 10);

    ::LibFred::InfoKeysetOutput info_data_9_with_changes = info_data_9;

    //updated historyid
    BOOST_CHECK(info_data_9.info_keyset_data.historyid !=info_data_10.info_keyset_data.historyid);
    info_data_9_with_changes.info_keyset_data.historyid = info_data_10.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_10.info_keyset_data.update_registrar_handle.get_value()));
    info_data_9_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_9_with_changes.info_keyset_data.update_time = info_data_10.info_keyset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_9_with_changes == info_data_10);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_10.at(0) == info_data_10);
    BOOST_CHECK(history_info_data_10.at(1) == info_data_9);
    BOOST_CHECK(history_info_data_10.at(2) == info_data_8);
    BOOST_CHECK(history_info_data_10.at(3) == info_data_7);
    BOOST_CHECK(history_info_data_10.at(4) == info_data_6);
    BOOST_CHECK(history_info_data_10.at(5) == info_data_5);
    BOOST_CHECK(history_info_data_10.at(6) == info_data_4);
    BOOST_CHECK(history_info_data_10.at(7) == info_data_3);
    BOOST_CHECK(history_info_data_10.at(8) == info_data_2);
    BOOST_CHECK(history_info_data_10.at(9) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_10.at(1).info_keyset_data == history_info_data_9.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_10.at(1).next_historyid.get_value() == history_info_data_10.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_10.at(0).info_keyset_data.crhistoryid == info_data_10.info_keyset_data.crhistoryid);

}//update_keyset

/**
 * test UpdateKeyset with wrong handle
 */

BOOST_AUTO_TEST_CASE(update_keyset_wrong_handle)
{
    std::string bad_test_keyset_handle = std::string("bad")+test_keyset_handle;
    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(bad_test_keyset_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_keyset_handle());
        BOOST_CHECK(static_cast<std::string>(ex.get_unknown_keyset_handle()).compare(bad_test_keyset_handle) == 0);
    }
}

/**
 * test UpdateKeyset with wrong registrar
 */
BOOST_AUTO_TEST_CASE(update_keyset_wrong_registrar)
{
    std::string bad_registrar_handle = registrar_handle+xmark;
    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());

}

/**
 * test UpdateKeyset add non-existing tech contact
 */
BOOST_AUTO_TEST_CASE(update_keyset_add_wrong_tech_contact)
{
    std::string bad_tech_contact_handle = admin_contact5_handle+xmark;
    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }

    BOOST_TEST_MESSAGE(std::string("handle: ") + info_data_1.info_keyset_data.handle + " roid: " + info_data_1.info_keyset_data.roid);
    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .add_tech_contact(bad_tech_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unknown_technical_contact_handle());
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_vector_of_unknown_technical_contact_handle().at(0).compare(bad_tech_contact_handle) == 0);
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}

/**
 * test UpdateKeyset add already added tech contact
 */
BOOST_AUTO_TEST_CASE(update_keyset_add_already_added_tech_contact)
{
    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .add_tech_contact(admin_contact6_handle)//already added in fixture
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_already_set_technical_contact_handle());
        BOOST_CHECK(ex.get_vector_of_already_set_technical_contact_handle().at(0).compare(admin_contact6_handle) == 0);
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}


/**
 * test UpdateKeyset remove non-existing tech contact
 */
BOOST_AUTO_TEST_CASE(update_keyset_rem_wrong_tech_contact)
{
    std::string bad_tech_contact_handle = admin_contact6_handle+xmark;

    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .rem_tech_contact(bad_tech_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unknown_technical_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unknown_technical_contact_handle().at(0).compare(bad_tech_contact_handle) == 0);
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}

/**
 * test UpdateKeyset remove existing unassigned tech contact
 */
BOOST_AUTO_TEST_CASE(update_keyset_rem_unassigned_tech_contact)
{
    std::string bad_tech_contact_handle = admin_contact4_handle;
    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .rem_tech_contact(bad_tech_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unassigned_technical_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unassigned_technical_contact_handle().at(0).compare(bad_tech_contact_handle) == 0);
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}


/**
 * test UpdateKeyset add already added dnskey
 */
BOOST_AUTO_TEST_CASE(update_keyset_add_already_added_dnskey)
{
    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .add_dns_key(::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"))
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_already_set_dns_key());
        BOOST_CHECK(ex.get_vector_of_already_set_dns_key().at(0) == ::LibFred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"));
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}

/**
 * test UpdateKeyset remove unassigned dnskey
 */
BOOST_AUTO_TEST_CASE(update_keyset_unassigned_dnskey)
{
    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .rem_dns_key(::LibFred::DnsKey(257, 3, 5, "unassignedkey"))
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unassigned_dns_key());
        BOOST_CHECK(ex.get_vector_of_unassigned_dns_key().at(0) == ::LibFred::DnsKey(257, 3, 5, "unassignedkey"));
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}


/**
 * test InfoKeysetHistoryByRoid
 * create and update test keyset
 * compare successive states from info keyset with states from info keyset history
 * check initial and next historyid in info keyset history
 * check valid_from and valid_to in info keyset history
 */
BOOST_AUTO_TEST_CASE(info_keyset_history_test)
{
    ::LibFred::InfoKeysetOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    }
    //call update
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::InfoKeysetOutput info_data_2;
    std::vector<::LibFred::InfoKeysetOutput> history_info_data;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
        history_info_data = ::LibFred::InfoKeysetHistoryByRoid(info_data_1.info_keyset_data.roid).exec(ctx);
    }

    BOOST_CHECK(history_info_data.at(0) == info_data_2);
    BOOST_CHECK(history_info_data.at(1) == info_data_1);

    BOOST_CHECK(history_info_data.at(1).next_historyid.get_value() == history_info_data.at(0).info_keyset_data.historyid);

    BOOST_CHECK(history_info_data.at(1).history_valid_from < history_info_data.at(1).history_valid_to.get_value());
    BOOST_CHECK(history_info_data.at(1).history_valid_to.get_value() <= history_info_data.at(0).history_valid_from);
    BOOST_CHECK(history_info_data.at(0).history_valid_to.isnull());

    BOOST_CHECK(history_info_data.at(1).info_keyset_data.crhistoryid == history_info_data.at(1).info_keyset_data.historyid);
}



BOOST_AUTO_TEST_SUITE_END();//TestUpdateKeyset
