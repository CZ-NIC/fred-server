/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
 *  @file test-update-domain.cc
 *  UpdateDomain tests
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
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/info_domain_impl.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
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

#include <boost/asio/ip/address.hpp>
#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-update-domain";

struct update_domain_fixture : virtual public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_fqdn;
    std::string test_enum_fqdn;

    update_domain_fixture()
    : xmark(Random::Generator().get_seq(Random::CharSet::digits(), 9))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_fqdn(std::string("fred")+xmark+".cz")
    , test_enum_fqdn(std::string()+xmark.at(0)+'.'+xmark.at(1)+'.'+xmark.at(2)+'.'
                                        +xmark.at(3)+'.'+xmark.at(4)+'.'+xmark.at(5)+'.'
                                        +xmark.at(6)+'.'+xmark.at(7)+'.'+xmark.at(8)+".0.2.4.e164.arpa")
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateContact(registrant_contact_handle,registrar_handle)
                .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

        ::LibFred::CreateDomain(
                test_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);

        ::LibFred::CreateDomain(
                test_enum_fqdn//const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(boost::gregorian::from_string("2012-01-21"))
        .set_enum_publish_flag(false)
        .exec(ctx);

        ctx.commit_transaction();
    }
    ~update_domain_fixture()
    {}
};

struct update_domain_admin_nsset_keyset_fixture
: virtual update_domain_fixture, virtual public Test::instantiate_db_template
{
    std::string admin_contact_handle;
    std::string admin_contact1_handle;
    std::string test_nsset_handle;
    std::string test_keyset_handle;

    update_domain_admin_nsset_keyset_fixture()
    : admin_contact_handle(std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark)
    , admin_contact1_handle(std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark)
    , test_nsset_handle(std::string("TEST-D-NSSET-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-D-KEYSET-HANDLE")+xmark)
    {
        namespace ip = boost::asio::ip;
        ::LibFred::OperationContextCreator ctx;

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateContact(admin_contact1_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact2_handle))
                .exec(ctx);

        ::LibFred::CreateKeyset(test_keyset_handle, registrar_handle)
                //.set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .exec(ctx);

        ctx.commit_transaction();
    }

    ~update_domain_admin_nsset_keyset_fixture(){}
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateDomain, update_domain_fixture)

/**
 * test UpdateDomain::Exception
 * test create and throw exception with special data
 */
BOOST_FIXTURE_TEST_CASE(update_domain_exception, Test::instantiate_db_template)
{
    //good path exception
    BOOST_CHECK_THROW (BOOST_THROW_EXCEPTION(::LibFred::UpdateDomain::Exception().set_unknown_domain_fqdn("badfqdn.cz"));
    , ::LibFred::OperationException);

    //bad path exception exception
    BOOST_CHECK_THROW ( BOOST_THROW_EXCEPTION(::LibFred::InternalError("test error"));
    , std::exception);

}

/**
 * test UpdateDomain
 * test UpdateDomain construction and methods calls with precreated data
 * calls in test shouldn't throw
 * check info domain against history
 */
BOOST_FIXTURE_TEST_CASE(update_domain, update_domain_admin_nsset_keyset_fixture )
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoDomainOutput info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_1 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    //update_registrar_handle check
    BOOST_CHECK(info_data_1.info_domain_data.update_registrar_handle.isnull());

    //update_time
    BOOST_CHECK(info_data_1.info_domain_data.update_time.isnull());

    //history check
    BOOST_CHECK(history_info_data_1.at(0) == info_data_1);
    BOOST_CHECK(history_info_data_1.at(0).info_domain_data.crhistoryid == info_data_1.info_domain_data.historyid);

    //call update using big ctor
    ::LibFred::UpdateDomain(test_fqdn//fqdn
            , registrar_handle//registrar
            , registrant_contact_handle //registrant - owner
            , std::string("testauthinfo1") //authinfo
            , Nullable<std::string>()//unset nsset - set to null
            , Optional<Nullable<std::string> >()//dont change keyset
            , Util::vector_of<std::string> (admin_contact1_handle)(registrant_contact_handle) //add admin contacts
            , Util::vector_of<std::string> (admin_contact2_handle) //remove admin contacts
            , Optional<boost::gregorian::date>()//exdate
            , Optional<boost::gregorian::date>()//enumvalexdate
            , Optional<bool>()//enum publish
            , Optional<unsigned long long>() //request_id not set
            ).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_2 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_1_with_changes = info_data_1;

    //updated authinfopw
    BOOST_CHECK(info_data_1.info_domain_data.authinfopw != info_data_2.info_domain_data.authinfopw);
    BOOST_CHECK(std::string("testauthinfo1") == info_data_2.info_domain_data.authinfopw);
    info_data_1_with_changes.info_domain_data.authinfopw = std::string("testauthinfo1");

    //updated historyid
    BOOST_CHECK(info_data_1.info_domain_data.historyid !=info_data_2.info_domain_data.historyid);
    info_data_1_with_changes.info_domain_data.historyid = info_data_2.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == info_data_2.info_domain_data.update_registrar_handle.get_value());
    info_data_1_with_changes.info_domain_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_2.info_domain_data.sponsoring_registrar_handle));
    info_data_1_with_changes.info_domain_data.sponsoring_registrar_handle = registrar_handle;

    //updated registrant_handle
    BOOST_CHECK(registrant_contact_handle == info_data_2.info_domain_data.registrant.handle);
    info_data_1_with_changes.info_domain_data.registrant = info_data_2.info_domain_data.registrant;

    //updated update_time
    info_data_1_with_changes.info_domain_data.update_time = info_data_2.info_domain_data.update_time;

    //updated admin contacts

    ::LibFred::InfoContactOutput admin_contact_info  = ::LibFred::InfoContactByHandle(admin_contact_handle).exec(ctx);
    ::LibFred::InfoContactOutput admin_contact1_info = ::LibFred::InfoContactByHandle(admin_contact1_handle).exec(ctx);
    ::LibFred::InfoContactOutput registrant_contact_info = ::LibFred::InfoContactByHandle(registrant_contact_handle).exec(ctx);
    ::LibFred::InfoContactOutput admin_contact2_info = ::LibFred::InfoContactByHandle(admin_contact2_handle).exec(ctx);

    info_data_1_with_changes.info_domain_data.admin_contacts.push_back(::LibFred::ObjectIdHandlePair(
            admin_contact1_info.info_contact_data.id, admin_contact1_info.info_contact_data.handle));
    info_data_1_with_changes.info_domain_data.admin_contacts.push_back(::LibFred::ObjectIdHandlePair(
            registrant_contact_info.info_contact_data.id, registrant_contact_info.info_contact_data.handle));
    info_data_1_with_changes.info_domain_data.admin_contacts
        .erase(std::remove(info_data_1_with_changes.info_domain_data.admin_contacts.begin()
            , info_data_1_with_changes.info_domain_data.admin_contacts.end(), ::LibFred::ObjectIdHandlePair(
                    admin_contact2_info.info_contact_data.id, admin_contact2_info.info_contact_data.handle))
            , info_data_1_with_changes.info_domain_data.admin_contacts.end());

    //check changes made by last update
    BOOST_CHECK(info_data_1_with_changes == info_data_2);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_2.at(0) == info_data_2);
    BOOST_CHECK(history_info_data_2.at(1) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_2.at(1).info_domain_data == history_info_data_1.at(0).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_2.at(1).next_historyid.get_value() == history_info_data_2.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_2.at(0).info_domain_data.crhistoryid == info_data_2.info_domain_data.crhistoryid);

    //call update using small ctor and set custom params
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
    .set_authinfo("testauthinfo")
    .set_registrant(registrant_contact_handle)
    .add_admin_contact(admin_contact_handle)
    .rem_admin_contact(registrant_contact_handle)
    .rem_admin_contact(admin_contact1_handle)
    .exec(ctx);

    ::LibFred::InfoDomainOutput info_data_3 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_3 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_2_with_changes = info_data_2;

    //updated authinfopw
    BOOST_CHECK(info_data_2.info_domain_data.authinfopw != info_data_3.info_domain_data.authinfopw);
    BOOST_CHECK(std::string("testauthinfo") == info_data_3.info_domain_data.authinfopw);
    info_data_2_with_changes.info_domain_data.authinfopw = std::string("testauthinfo");

    //updated historyid
    BOOST_CHECK(info_data_2.info_domain_data.historyid !=info_data_3.info_domain_data.historyid);
    info_data_2_with_changes.info_domain_data.historyid = info_data_3.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_3.info_domain_data.update_registrar_handle.get_value()));

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_3.info_domain_data.sponsoring_registrar_handle));

    //updated registrant_handle
    BOOST_CHECK(registrant_contact_handle == info_data_3.info_domain_data.registrant.handle);
    info_data_2_with_changes.info_domain_data.registrant = info_data_3.info_domain_data.registrant;

    //updated update_time
    info_data_2_with_changes.info_domain_data.update_time = info_data_3.info_domain_data.update_time;

    //updated admin contacts
    info_data_2_with_changes.info_domain_data.admin_contacts.push_back(::LibFred::ObjectIdHandlePair(
        admin_contact_info.info_contact_data.id, admin_contact_info.info_contact_data.handle));

    info_data_2_with_changes.info_domain_data.admin_contacts
        .erase(std::remove(info_data_2_with_changes.info_domain_data.admin_contacts.begin()
            , info_data_2_with_changes.info_domain_data.admin_contacts.end()
                , ::LibFred::ObjectIdHandlePair(registrant_contact_info.info_contact_data.id
                    , registrant_contact_info.info_contact_data.handle))
            , info_data_2_with_changes.info_domain_data.admin_contacts.end());

    info_data_2_with_changes.info_domain_data.admin_contacts
        .erase(std::remove(info_data_2_with_changes.info_domain_data.admin_contacts.begin()
            , info_data_2_with_changes.info_domain_data.admin_contacts.end()
                , ::LibFred::ObjectIdHandlePair(admin_contact1_info.info_contact_data.id
                    , admin_contact1_info.info_contact_data.handle))
            , info_data_2_with_changes.info_domain_data.admin_contacts.end());


    //check changes made by last update
    BOOST_CHECK(info_data_2_with_changes == info_data_3);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_3.at(0) == info_data_3);
    BOOST_CHECK(history_info_data_3.at(1) == info_data_2);
    BOOST_CHECK(history_info_data_3.at(2) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_3.at(2).info_domain_data == history_info_data_2.at(1).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_3.at(1).next_historyid.get_value() == history_info_data_3.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_3.at(0).info_domain_data.crhistoryid == info_data_3.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_authinfo("testauthinfo").exec(ctx);

    ::LibFred::InfoDomainOutput info_data_4 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_4 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_3_with_changes = info_data_3;

    //updated authinfopw
    BOOST_CHECK(std::string("testauthinfo") == info_data_4.info_domain_data.authinfopw);
    info_data_3_with_changes.info_domain_data.authinfopw = std::string("testauthinfo");

    //updated historyid
    BOOST_CHECK(info_data_3.info_domain_data.historyid !=info_data_4.info_domain_data.historyid);
    info_data_3_with_changes.info_domain_data.historyid = info_data_4.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_domain_data.update_registrar_handle.get_value()));

    //updated update_time
    info_data_3_with_changes.info_domain_data.update_time = info_data_4.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_3_with_changes == info_data_4);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_4.at(0) == info_data_4);
    BOOST_CHECK(history_info_data_4.at(1) == info_data_3);
    BOOST_CHECK(history_info_data_4.at(2) == info_data_2);
    BOOST_CHECK(history_info_data_4.at(3) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_4.at(3).info_domain_data == history_info_data_3.at(2).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_4.at(1).next_historyid.get_value() == history_info_data_4.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_4.at(0).info_domain_data.crhistoryid == info_data_4.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_registrant(registrant_contact_handle).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_5 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_5 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_4_with_changes = info_data_4;

    //updated historyid
    BOOST_CHECK(info_data_4.info_domain_data.historyid !=info_data_5.info_domain_data.historyid);
    info_data_4_with_changes.info_domain_data.historyid = info_data_5.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_domain_data.update_registrar_handle.get_value()));

    //updated registrant_handle
    BOOST_CHECK(registrant_contact_handle == info_data_5.info_domain_data.registrant.handle);
    info_data_4_with_changes.info_domain_data.registrant = info_data_5.info_domain_data.registrant;

    //updated update_time
    info_data_4_with_changes.info_domain_data.update_time = info_data_5.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_4_with_changes == info_data_5);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_5.at(0) == info_data_5);
    BOOST_CHECK(history_info_data_5.at(1) == info_data_4);
    BOOST_CHECK(history_info_data_5.at(2) == info_data_3);
    BOOST_CHECK(history_info_data_5.at(3) == info_data_2);
    BOOST_CHECK(history_info_data_5.at(4) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_5.at(4).info_domain_data == history_info_data_4.at(3).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_5.at(1).next_historyid.get_value() == history_info_data_5.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_5.at(0).info_domain_data.crhistoryid == info_data_5.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).add_admin_contact(admin_contact1_handle).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_6 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_6 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_5_with_changes = info_data_5;

    //updated historyid
    BOOST_CHECK(info_data_5.info_domain_data.historyid !=info_data_6.info_domain_data.historyid);
    info_data_5_with_changes.info_domain_data.historyid = info_data_6.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_6.info_domain_data.update_registrar_handle.get_value()));

    //updated admin contacts
    info_data_5_with_changes.info_domain_data.admin_contacts.push_back(::LibFred::ObjectIdHandlePair(
        admin_contact1_info.info_contact_data.id, admin_contact1_info.info_contact_data.handle));

    //updated update_time
    info_data_5_with_changes.info_domain_data.update_time = info_data_6.info_domain_data.update_time;

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
    BOOST_CHECK(history_info_data_6.at(5).info_domain_data == history_info_data_5.at(4).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_6.at(1).next_historyid.get_value() == history_info_data_6.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_6.at(0).info_domain_data.crhistoryid == info_data_6.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).rem_admin_contact(admin_contact_handle).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_7 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_7 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_6_with_changes = info_data_6;

    //updated historyid
    BOOST_CHECK(info_data_6.info_domain_data.historyid !=info_data_7.info_domain_data.historyid);
    info_data_6_with_changes.info_domain_data.historyid = info_data_7.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_7.info_domain_data.update_registrar_handle.get_value()));

    //updated admin contacts
    info_data_6_with_changes.info_domain_data.admin_contacts
        .erase(std::remove(info_data_6_with_changes.info_domain_data.admin_contacts.begin()
            , info_data_6_with_changes.info_domain_data.admin_contacts.end()
            , ::LibFred::ObjectIdHandlePair(admin_contact_info.info_contact_data.id
                    , admin_contact_info.info_contact_data.handle))
            , info_data_6_with_changes.info_domain_data.admin_contacts.end());

    //updated update_time
    info_data_6_with_changes.info_domain_data.update_time = info_data_7.info_domain_data.update_time;

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
    BOOST_CHECK(history_info_data_7.at(6).info_domain_data == history_info_data_6.at(5).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_7.at(1).next_historyid.get_value() == history_info_data_7.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_7.at(0).info_domain_data.crhistoryid == info_data_7.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_nsset(test_nsset_handle).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_8 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_8 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_7_with_changes = info_data_7;

    //updated historyid
    BOOST_CHECK(info_data_7.info_domain_data.historyid !=info_data_8.info_domain_data.historyid);
    info_data_7_with_changes.info_domain_data.historyid = info_data_8.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_8.info_domain_data.update_registrar_handle.get_value()));

    //set nsset
    ::LibFred::InfoNssetOutput test_nsset_info = ::LibFred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    info_data_7_with_changes.info_domain_data.nsset = Nullable<::LibFred::ObjectIdHandlePair>(
        ::LibFred::ObjectIdHandlePair(test_nsset_info.info_nsset_data.id, test_nsset_info.info_nsset_data.handle));

    //updated update_time
    info_data_7_with_changes.info_domain_data.update_time = info_data_8.info_domain_data.update_time;

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
    BOOST_CHECK(history_info_data_8.at(7).info_domain_data == history_info_data_7.at(6).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_8.at(1).next_historyid.get_value() == history_info_data_8.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_8.at(0).info_domain_data.crhistoryid == info_data_8.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_nsset(Nullable<std::string>()).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_9 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_9 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_8_with_changes = info_data_8;

    //updated historyid
    BOOST_CHECK(info_data_8.info_domain_data.historyid !=info_data_9.info_domain_data.historyid);
    info_data_8_with_changes.info_domain_data.historyid = info_data_9.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_9.info_domain_data.update_registrar_handle.get_value()));

    //set nsset
    info_data_8_with_changes.info_domain_data.nsset = Nullable<::LibFred::ObjectIdHandlePair>();

    //updated update_time
    info_data_8_with_changes.info_domain_data.update_time = info_data_9.info_domain_data.update_time;

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
    BOOST_CHECK(history_info_data_9.at(8).info_domain_data == history_info_data_8.at(7).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_9.at(1).next_historyid.get_value() == history_info_data_9.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_9.at(0).info_domain_data.crhistoryid == info_data_9.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_nsset(test_nsset_handle).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_10 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_10 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_9_with_changes = info_data_9;

    //updated historyid
    BOOST_CHECK(info_data_9.info_domain_data.historyid !=info_data_10.info_domain_data.historyid);
    info_data_9_with_changes.info_domain_data.historyid = info_data_10.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_10.info_domain_data.update_registrar_handle.get_value()));

    //set nsset
    info_data_9_with_changes.info_domain_data.nsset = Nullable<::LibFred::ObjectIdHandlePair>(
            ::LibFred::ObjectIdHandlePair(test_nsset_info.info_nsset_data.id, test_nsset_info.info_nsset_data.handle));

    //updated update_time
    info_data_9_with_changes.info_domain_data.update_time = info_data_10.info_domain_data.update_time;

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
    BOOST_CHECK(history_info_data_10.at(9).info_domain_data == history_info_data_9.at(8).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_10.at(1).next_historyid.get_value() == history_info_data_10.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_10.at(0).info_domain_data.crhistoryid == info_data_10.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).unset_nsset().exec(ctx);

    ::LibFred::InfoDomainOutput info_data_11 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_11 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_10_with_changes = info_data_10;

    //updated historyid
    BOOST_CHECK(info_data_10.info_domain_data.historyid !=info_data_11.info_domain_data.historyid);
    info_data_10_with_changes.info_domain_data.historyid = info_data_11.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_11.info_domain_data.update_registrar_handle.get_value()));

    //set nsset
    info_data_10_with_changes.info_domain_data.nsset = Nullable<::LibFred::ObjectIdHandlePair>();

    //updated update_time
    info_data_10_with_changes.info_domain_data.update_time = info_data_11.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_10_with_changes == info_data_11);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_11.at(0) == info_data_11);
    BOOST_CHECK(history_info_data_11.at(1) == info_data_10);
    BOOST_CHECK(history_info_data_11.at(2) == info_data_9);
    BOOST_CHECK(history_info_data_11.at(3) == info_data_8);
    BOOST_CHECK(history_info_data_11.at(4) == info_data_7);
    BOOST_CHECK(history_info_data_11.at(5) == info_data_6);
    BOOST_CHECK(history_info_data_11.at(6) == info_data_5);
    BOOST_CHECK(history_info_data_11.at(7) == info_data_4);
    BOOST_CHECK(history_info_data_11.at(8) == info_data_3);
    BOOST_CHECK(history_info_data_11.at(9) == info_data_2);
    BOOST_CHECK(history_info_data_11.at(10) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_11.at(10).info_domain_data == history_info_data_10.at(9).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_11.at(1).next_historyid.get_value() == history_info_data_11.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_11.at(0).info_domain_data.crhistoryid == info_data_11.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_nsset(Nullable<std::string>(test_nsset_handle)).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_12 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_12 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_11_with_changes = info_data_11;

    //updated historyid
    BOOST_CHECK(info_data_11.info_domain_data.historyid !=info_data_12.info_domain_data.historyid);
    info_data_11_with_changes.info_domain_data.historyid = info_data_12.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_12.info_domain_data.update_registrar_handle.get_value()));

    //set nsset
    info_data_11_with_changes.info_domain_data.nsset = Nullable<::LibFred::ObjectIdHandlePair>(
            ::LibFred::ObjectIdHandlePair(test_nsset_info.info_nsset_data.id, test_nsset_info.info_nsset_data.handle));

    //updated update_time
    info_data_11_with_changes.info_domain_data.update_time = info_data_12.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_11_with_changes == info_data_12);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_12.at(0) == info_data_12);
    BOOST_CHECK(history_info_data_12.at(1) == info_data_11);
    BOOST_CHECK(history_info_data_12.at(2) == info_data_10);
    BOOST_CHECK(history_info_data_12.at(3) == info_data_9);
    BOOST_CHECK(history_info_data_12.at(4) == info_data_8);
    BOOST_CHECK(history_info_data_12.at(5) == info_data_7);
    BOOST_CHECK(history_info_data_12.at(6) == info_data_6);
    BOOST_CHECK(history_info_data_12.at(7) == info_data_5);
    BOOST_CHECK(history_info_data_12.at(8) == info_data_4);
    BOOST_CHECK(history_info_data_12.at(9) == info_data_3);
    BOOST_CHECK(history_info_data_12.at(10) == info_data_2);
    BOOST_CHECK(history_info_data_12.at(11) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_12.at(11).info_domain_data == history_info_data_11.at(10).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_12.at(1).next_historyid.get_value() == history_info_data_12.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_12.at(0).info_domain_data.crhistoryid == info_data_12.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_nsset(test_nsset_handle).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_13 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_13 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_12_with_changes = info_data_12;

    //updated historyid
    BOOST_CHECK(info_data_12.info_domain_data.historyid !=info_data_13.info_domain_data.historyid);
    info_data_12_with_changes.info_domain_data.historyid = info_data_13.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_13.info_domain_data.update_registrar_handle.get_value()));

    //set nsset
    info_data_12_with_changes.info_domain_data.nsset = Nullable<::LibFred::ObjectIdHandlePair>(
            ::LibFred::ObjectIdHandlePair(test_nsset_info.info_nsset_data.id, test_nsset_info.info_nsset_data.handle));

    //updated update_time
    info_data_12_with_changes.info_domain_data.update_time = info_data_13.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_12_with_changes == info_data_13);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_13.at(0) == info_data_13);
    BOOST_CHECK(history_info_data_13.at(1) == info_data_12);
    BOOST_CHECK(history_info_data_13.at(2) == info_data_11);
    BOOST_CHECK(history_info_data_13.at(3) == info_data_10);
    BOOST_CHECK(history_info_data_13.at(4) == info_data_9);
    BOOST_CHECK(history_info_data_13.at(5) == info_data_8);
    BOOST_CHECK(history_info_data_13.at(6) == info_data_7);
    BOOST_CHECK(history_info_data_13.at(7) == info_data_6);
    BOOST_CHECK(history_info_data_13.at(8) == info_data_5);
    BOOST_CHECK(history_info_data_13.at(9) == info_data_4);
    BOOST_CHECK(history_info_data_13.at(10) == info_data_3);
    BOOST_CHECK(history_info_data_13.at(11) == info_data_2);
    BOOST_CHECK(history_info_data_13.at(12) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_13.at(12).info_domain_data == history_info_data_12.at(11).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_13.at(1).next_historyid.get_value() == history_info_data_13.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_13.at(0).info_domain_data.crhistoryid == info_data_13.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_keyset(Nullable<std::string>()).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_14 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_14 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_13_with_changes = info_data_13;

    //updated historyid
    BOOST_CHECK(info_data_13.info_domain_data.historyid !=info_data_14.info_domain_data.historyid);
    info_data_13_with_changes.info_domain_data.historyid = info_data_14.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_14.info_domain_data.update_registrar_handle.get_value()));

    //set keyset
    info_data_13_with_changes.info_domain_data.keyset = Nullable<::LibFred::ObjectIdHandlePair>();

    //updated update_time
    info_data_13_with_changes.info_domain_data.update_time = info_data_14.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_13_with_changes == info_data_14);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_14.at(0) == info_data_14);
    BOOST_CHECK(history_info_data_14.at(1) == info_data_13);
    BOOST_CHECK(history_info_data_14.at(2) == info_data_12);
    BOOST_CHECK(history_info_data_14.at(3) == info_data_11);
    BOOST_CHECK(history_info_data_14.at(4) == info_data_10);
    BOOST_CHECK(history_info_data_14.at(5) == info_data_9);
    BOOST_CHECK(history_info_data_14.at(6) == info_data_8);
    BOOST_CHECK(history_info_data_14.at(7) == info_data_7);
    BOOST_CHECK(history_info_data_14.at(8) == info_data_6);
    BOOST_CHECK(history_info_data_14.at(9) == info_data_5);
    BOOST_CHECK(history_info_data_14.at(10) == info_data_4);
    BOOST_CHECK(history_info_data_14.at(11) == info_data_3);
    BOOST_CHECK(history_info_data_14.at(12) == info_data_2);
    BOOST_CHECK(history_info_data_14.at(13) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_14.at(13).info_domain_data == history_info_data_13.at(12).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_14.at(1).next_historyid.get_value() == history_info_data_14.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_14.at(0).info_domain_data.crhistoryid == info_data_14.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_keyset(test_keyset_handle).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_15 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_15 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_14_with_changes = info_data_14;

    //updated historyid
    BOOST_CHECK(info_data_14.info_domain_data.historyid !=info_data_15.info_domain_data.historyid);
    info_data_14_with_changes.info_domain_data.historyid = info_data_15.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_15.info_domain_data.update_registrar_handle.get_value()));

    //set keyset
    ::LibFred::InfoKeysetOutput test_keyset_info = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    info_data_14_with_changes.info_domain_data.keyset = Nullable<::LibFred::ObjectIdHandlePair>(
        ::LibFred::ObjectIdHandlePair(test_keyset_info.info_keyset_data.id, test_keyset_info.info_keyset_data.handle));

    //updated update_time
    info_data_14_with_changes.info_domain_data.update_time = info_data_15.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_14_with_changes == info_data_15);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_15.at(0) == info_data_15);
    BOOST_CHECK(history_info_data_15.at(1) == info_data_14);
    BOOST_CHECK(history_info_data_15.at(2) == info_data_13);
    BOOST_CHECK(history_info_data_15.at(3) == info_data_12);
    BOOST_CHECK(history_info_data_15.at(4) == info_data_11);
    BOOST_CHECK(history_info_data_15.at(5) == info_data_10);
    BOOST_CHECK(history_info_data_15.at(6) == info_data_9);
    BOOST_CHECK(history_info_data_15.at(7) == info_data_8);
    BOOST_CHECK(history_info_data_15.at(8) == info_data_7);
    BOOST_CHECK(history_info_data_15.at(9) == info_data_6);
    BOOST_CHECK(history_info_data_15.at(10) == info_data_5);
    BOOST_CHECK(history_info_data_15.at(11) == info_data_4);
    BOOST_CHECK(history_info_data_15.at(12) == info_data_3);
    BOOST_CHECK(history_info_data_15.at(13) == info_data_2);
    BOOST_CHECK(history_info_data_15.at(14) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_15.at(14).info_domain_data == history_info_data_14.at(13).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_15.at(1).next_historyid.get_value() == history_info_data_15.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_15.at(0).info_domain_data.crhistoryid == info_data_15.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).unset_keyset().exec(ctx);

    ::LibFred::InfoDomainOutput info_data_16 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_16 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_15_with_changes = info_data_15;

    //updated historyid
    BOOST_CHECK(info_data_15.info_domain_data.historyid !=info_data_16.info_domain_data.historyid);
    info_data_15_with_changes.info_domain_data.historyid = info_data_16.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_16.info_domain_data.update_registrar_handle.get_value()));

    //set keyset
    info_data_15_with_changes.info_domain_data.keyset = Nullable<::LibFred::ObjectIdHandlePair>();

    //updated update_time
    info_data_15_with_changes.info_domain_data.update_time = info_data_16.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_15_with_changes == info_data_16);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_16.at(0) == info_data_16);
    BOOST_CHECK(history_info_data_16.at(1) == info_data_15);
    BOOST_CHECK(history_info_data_16.at(2) == info_data_14);
    BOOST_CHECK(history_info_data_16.at(3) == info_data_13);
    BOOST_CHECK(history_info_data_16.at(4) == info_data_12);
    BOOST_CHECK(history_info_data_16.at(5) == info_data_11);
    BOOST_CHECK(history_info_data_16.at(6) == info_data_10);
    BOOST_CHECK(history_info_data_16.at(7) == info_data_9);
    BOOST_CHECK(history_info_data_16.at(8) == info_data_8);
    BOOST_CHECK(history_info_data_16.at(9) == info_data_7);
    BOOST_CHECK(history_info_data_16.at(10) == info_data_6);
    BOOST_CHECK(history_info_data_16.at(11) == info_data_5);
    BOOST_CHECK(history_info_data_16.at(12) == info_data_4);
    BOOST_CHECK(history_info_data_16.at(13) == info_data_3);
    BOOST_CHECK(history_info_data_16.at(14) == info_data_2);
    BOOST_CHECK(history_info_data_16.at(15) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_16.at(15).info_domain_data == history_info_data_15.at(14).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_16.at(1).next_historyid.get_value() == history_info_data_16.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_16.at(0).info_domain_data.crhistoryid == info_data_16.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_keyset(Nullable<std::string>(test_keyset_handle)).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_17 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_17 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_16_with_changes = info_data_16;

    //updated historyid
    BOOST_CHECK(info_data_16.info_domain_data.historyid !=info_data_17.info_domain_data.historyid);
    info_data_16_with_changes.info_domain_data.historyid = info_data_17.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_17.info_domain_data.update_registrar_handle.get_value()));

    //set keyset
    info_data_16_with_changes.info_domain_data.keyset = Nullable<::LibFred::ObjectIdHandlePair>(
            ::LibFred::ObjectIdHandlePair(test_keyset_info.info_keyset_data.id, test_keyset_info.info_keyset_data.handle));

    //updated update_time
    info_data_16_with_changes.info_domain_data.update_time = info_data_17.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_16_with_changes == info_data_17);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_17.at(0) == info_data_17);
    BOOST_CHECK(history_info_data_17.at(1) == info_data_16);
    BOOST_CHECK(history_info_data_17.at(2) == info_data_15);
    BOOST_CHECK(history_info_data_17.at(3) == info_data_14);
    BOOST_CHECK(history_info_data_17.at(4) == info_data_13);
    BOOST_CHECK(history_info_data_17.at(5) == info_data_12);
    BOOST_CHECK(history_info_data_17.at(6) == info_data_11);
    BOOST_CHECK(history_info_data_17.at(7) == info_data_10);
    BOOST_CHECK(history_info_data_17.at(8) == info_data_9);
    BOOST_CHECK(history_info_data_17.at(9) == info_data_8);
    BOOST_CHECK(history_info_data_17.at(10) == info_data_7);
    BOOST_CHECK(history_info_data_17.at(11) == info_data_6);
    BOOST_CHECK(history_info_data_17.at(12) == info_data_5);
    BOOST_CHECK(history_info_data_17.at(13) == info_data_4);
    BOOST_CHECK(history_info_data_17.at(14) == info_data_3);
    BOOST_CHECK(history_info_data_17.at(15) == info_data_2);
    BOOST_CHECK(history_info_data_17.at(16) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_17.at(16).info_domain_data == history_info_data_16.at(15).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_17.at(1).next_historyid.get_value() == history_info_data_17.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_17.at(0).info_domain_data.crhistoryid == info_data_17.info_domain_data.crhistoryid);

    //call update using small ctor and set one custom param
    ::LibFred::UpdateDomain(test_fqdn, registrar_handle).set_logd_request_id(3).exec(ctx);


    ::LibFred::InfoDomainOutput info_data_18 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    std::vector<::LibFred::InfoDomainOutput> history_info_data_18 = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);

    ::LibFred::InfoDomainOutput info_data_17_with_changes = info_data_17;

    //updated historyid
    BOOST_CHECK(info_data_17.info_domain_data.historyid !=info_data_18.info_domain_data.historyid);
    info_data_17_with_changes.info_domain_data.historyid = info_data_18.info_domain_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_18.info_domain_data.update_registrar_handle.get_value()));

    //updated update_time
    info_data_17_with_changes.info_domain_data.update_time = info_data_18.info_domain_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_17_with_changes == info_data_18);
    BOOST_CHECK(history_info_data_18.at(0).logd_request_id.get_value() == 3);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_18.at(0) == info_data_18);
    BOOST_CHECK(history_info_data_18.at(1) == info_data_17);
    BOOST_CHECK(history_info_data_18.at(2) == info_data_16);
    BOOST_CHECK(history_info_data_18.at(3) == info_data_15);
    BOOST_CHECK(history_info_data_18.at(4) == info_data_14);
    BOOST_CHECK(history_info_data_18.at(5) == info_data_13);
    BOOST_CHECK(history_info_data_18.at(6) == info_data_12);
    BOOST_CHECK(history_info_data_18.at(7) == info_data_11);
    BOOST_CHECK(history_info_data_18.at(8) == info_data_10);
    BOOST_CHECK(history_info_data_18.at(9) == info_data_9);
    BOOST_CHECK(history_info_data_18.at(10) == info_data_8);
    BOOST_CHECK(history_info_data_18.at(11) == info_data_7);
    BOOST_CHECK(history_info_data_18.at(12) == info_data_6);
    BOOST_CHECK(history_info_data_18.at(13) == info_data_5);
    BOOST_CHECK(history_info_data_18.at(14) == info_data_4);
    BOOST_CHECK(history_info_data_18.at(15) == info_data_3);
    BOOST_CHECK(history_info_data_18.at(16) == info_data_2);
    BOOST_CHECK(history_info_data_18.at(17) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_18.at(17).info_domain_data == history_info_data_17.at(16).info_domain_data);

    //check historyid
    BOOST_CHECK(history_info_data_18.at(1).next_historyid.get_value() == history_info_data_18.at(0).info_domain_data.historyid);
    BOOST_CHECK(history_info_data_18.at(0).info_domain_data.crhistoryid == info_data_18.info_domain_data.crhistoryid);


    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "SELECT o.authinfopw = $1::text "
        //" AND "
        " FROM object_registry oreg "
        " JOIN object o ON o.id = oreg.id "
        " WHERE oreg.name = $2::text"
        ,Database::query_param_list("testauthinfo")(test_fqdn))[0][0]));

    //commit db transaction
    ctx.commit_transaction();
}//update_domain



/**
 * test UpdateDomain with wrong fqdn
 */

BOOST_AUTO_TEST_CASE(update_domain_wrong_fqdn)
{
    std::string bad_test_fqdn = std::string("bad")+xmark+".cz";
    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(bad_test_fqdn, registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_domain_fqdn());
        BOOST_CHECK(ex.get_unknown_domain_fqdn().compare(bad_test_fqdn) == 0);
    }
}


/**
 * test UpdateDomain with wrong registrar
 */
BOOST_AUTO_TEST_CASE(update_domain_wrong_registrar)
{
    std::string bad_registrar_handle = registrar_handle+xmark;
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test UpdateDomain with wrong registrant
 */
BOOST_AUTO_TEST_CASE(update_domain_wrong_registrant)
{
    std::string bad_registrant_handle = registrant_contact_handle+xmark;

    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .set_registrant(bad_registrant_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrant_handle());
        BOOST_TEST_MESSAGE(bad_registrant_handle);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_registrant_handle().compare(bad_registrant_handle) == 0);
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test UpdateDomain add non-existing admin
 */
BOOST_AUTO_TEST_CASE(update_domain_add_wrong_admin)
{
    std::string bad_admin_contact_handle = admin_contact2_handle+xmark;

    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .add_admin_contact(bad_admin_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unknown_admin_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unknown_admin_contact_handle().at(0).compare(bad_admin_contact_handle) == 0);
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());

}

/**
 * test UpdateDomain add already added admin
 */
BOOST_AUTO_TEST_CASE(update_domain_add_already_added_admin)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .add_admin_contact(admin_contact2_handle)
        .add_admin_contact(admin_contact2_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_already_set_admin_contact_handle());
        BOOST_CHECK(ex.get_vector_of_already_set_admin_contact_handle().at(0).compare(admin_contact2_handle) == 0);
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());

}

/**
 * test UpdateDomain remove non-existing admin
 */
BOOST_AUTO_TEST_CASE(update_domain_rem_wrong_admin)
{
    std::string bad_admin_contact_handle = admin_contact2_handle+xmark;

    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .rem_admin_contact(bad_admin_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unknown_admin_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unknown_admin_contact_handle().at(0).compare(bad_admin_contact_handle) == 0);
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());

}

/**
 * test UpdateDomain remove existing unassigned admin
 */
BOOST_AUTO_TEST_CASE(update_domain_rem_unassigned_admin)
{
    std::string bad_admin_contact_handle = registrant_contact_handle;

    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .rem_admin_contact(bad_admin_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unassigned_admin_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unassigned_admin_contact_handle().at(0).compare(bad_admin_contact_handle) == 0);
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());

}

/**
 * test InfoDomainHistoryByRoid
 * create and update test domain
 * compare successive states from info domain with states from info domain history
 * check initial and next historyid in info domain history
 * check valid_from and valid_to in info domain history
 */
BOOST_AUTO_TEST_CASE(info_domain_history_test)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    //call update
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }

    ::LibFred::InfoDomainOutput info_data_2;
    std::vector<::LibFred::InfoDomainOutput> history_info_data;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
        history_info_data = ::LibFred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);
    }

    BOOST_CHECK(history_info_data.at(0) == info_data_2);
    BOOST_CHECK(history_info_data.at(1) == info_data_1);

    BOOST_CHECK(history_info_data.at(1).next_historyid.get_value() == history_info_data.at(0).info_domain_data.historyid);

    BOOST_CHECK(history_info_data.at(1).history_valid_from < history_info_data.at(1).history_valid_to.get_value());
    BOOST_CHECK(history_info_data.at(1).history_valid_to.get_value() <= history_info_data.at(0).history_valid_from);
    BOOST_CHECK(history_info_data.at(0).history_valid_to.isnull());

    BOOST_CHECK(history_info_data.at(1).info_domain_data.crhistoryid == history_info_data.at(1).info_domain_data.historyid);

}

/**
 * test UpdateDomain set exdate
 */
BOOST_AUTO_TEST_CASE(update_domain_set_exdate)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    boost::gregorian::date exdate(boost::gregorian::from_string("2010-12-20"));

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .set_domain_expiration(exdate)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_2.info_domain_data.expiration_date == exdate);
}

/**
 * test UpdateDomain set invalid exdate
 */
BOOST_FIXTURE_TEST_CASE(update_domain_set_wrong_exdate, update_domain_fixture)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }

    boost::gregorian::date exdate;

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .set_domain_expiration(exdate)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_expiration_date());
        BOOST_CHECK(ex.get_invalid_expiration_date().is_special());
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test UpdateDomain set ENUM valexdate to ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(update_domain_set_valexdate, update_domain_fixture)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }

    boost::gregorian::date valexdate(boost::gregorian::from_string("2010-12-20"));

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_enum_fqdn, registrar_handle)
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_2.info_domain_data.enum_domain_validation.get_value()
            .validation_expiration == valexdate);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test UpdateDomain set invalid ENUM valexdate to ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(update_domain_set_wrong_valexdate, update_domain_fixture)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }

    boost::gregorian::date valexdate;

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_enum_fqdn, registrar_handle)
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::UpdateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_enum_validation_expiration_date());
        BOOST_CHECK(ex.get_invalid_enum_validation_expiration_date().is_special());
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test UpdateDomain set ENUM valexdate to non-ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(update_domain_set_valexdate_wrong_domain, update_domain_fixture)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }

    boost::gregorian::date valexdate(boost::gregorian::from_string("2010-12-20"));

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InternalError& ex)
    {
        BOOST_TEST_MESSAGE(ex.what());
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test UpdateDomain set ENUM publish flag to non-ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(update_domain_set_publish_wrong_domain, update_domain_fixture)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::UpdateDomain(test_fqdn, registrar_handle)
        .set_enum_publish_flag(true)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InternalError& ex)
    {
        BOOST_TEST_MESSAGE(ex.what());
    }

    ::LibFred::InfoDomainOutput info_data_2;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_2 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}


BOOST_AUTO_TEST_SUITE_END();//TestUpdateDomain
