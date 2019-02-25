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
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>


BOOST_AUTO_TEST_SUITE(TestInfoDomain)

struct test_domain_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact_handle;
    std::string admin_contact1_handle;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_nsset_handle;
    std::string test_keyset_handle;
    std::string test_fqdn;

    ::LibFred::InfoDomainOutput test_info_domain_output;

    test_domain_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact_handle(std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark)
    , admin_contact1_handle(std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark)
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark)
    , test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    , test_fqdn(std::string("fredinfo")+xmark+".cz")
    {
        namespace ip = boost::asio::ip;
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

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

        ::LibFred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
            (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
            (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
            ).exec(ctx);

        ::LibFred::CreateKeyset(test_keyset_handle, registrar_handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        ::LibFred::CreateDomain(test_fqdn//const std::string& fqdn
                    , registrar_handle//const std::string& registrar
                    , registrant_contact_handle//const std::string& registrant
                    , Optional<std::string>("testauthinfo1")//const Optional<std::string>& authinfo
                    , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                    , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                    , Util::vector_of<std::string>(admin_contact1_handle)//const std::vector<std::string>& admin_contacts
                    , boost::gregorian::from_simple_string("2012-06-30")//const Optional<boost::gregorian::date>& expiration_date
                    , Optional<boost::gregorian::date>()
                    , Optional<bool>()
                    , 0//const Optional<unsigned long long> logd_request_id
                    ).exec(ctx);

        //id query
        Database::Result id_res = ctx.get_conn().exec_params("SELECT"
        " (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND name = LOWER($1::text)) AS test_fqdn_id"
        ",  (SELECT roid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND name = LOWER($1::text)) AS test_fqdn_roid"
        ",  (SELECT crhistoryid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND name = LOWER($1::text)) AS test_fqdn_crhistoryid"
        ",  (SELECT historyid FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND name = LOWER($1::text)) AS test_fqdn_historyid"
        ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) AND name = UPPER($2::text)) AS registrant_contact_handle_id"
        ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND name = UPPER($3::text)) AS test_nsset_handle_id"
        ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND name = UPPER($4::text)) AS test_keyset_handle_id"
        ", (SELECT id FROM registrar WHERE handle = UPPER($5::text)) AS registrar_handle_id"
        ", (SELECT id FROM object_registry WHERE type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) AND name = UPPER($6::text)) AS admin_contact1_handle_id"
        , Database::query_param_list(test_fqdn)(registrant_contact_handle)(test_nsset_handle)(test_keyset_handle)(registrar_handle)(admin_contact1_handle));

        //crdate fix
        ctx.get_conn().exec_params("UPDATE object_registry SET crdate = $1::timestamp WHERE id = $2::bigint",
            Database::query_param_list("2011-06-30T23:59:59")(static_cast<unsigned long long>(id_res[0]["test_fqdn_id"])));

        ctx.commit_transaction();//commit fixture

        //expected output data
        test_info_domain_output.info_domain_data.roid = static_cast<std::string>(id_res[0]["test_fqdn_roid"]);
        test_info_domain_output.info_domain_data.fqdn = test_fqdn;
        test_info_domain_output.info_domain_data.registrant = ::LibFred::ObjectIdHandlePair(static_cast<unsigned long long>(id_res[0]["registrant_contact_handle_id"]), registrant_contact_handle);
        test_info_domain_output.info_domain_data.nsset = ::LibFred::ObjectIdHandlePair(static_cast<unsigned long long>(id_res[0]["test_nsset_handle_id"]),test_nsset_handle);
        test_info_domain_output.info_domain_data.keyset = ::LibFred::ObjectIdHandlePair(static_cast<unsigned long long>(id_res[0]["test_keyset_handle_id"]),test_keyset_handle);
        test_info_domain_output.info_domain_data.sponsoring_registrar_handle = registrar_handle;
        test_info_domain_output.info_domain_data.create_registrar_handle = registrar_handle;
        test_info_domain_output.info_domain_data.update_registrar_handle = Nullable<std::string>();

        test_info_domain_output.info_domain_data.creation_time = boost::posix_time::time_from_string("2011-07-01 01:59:59");
        test_info_domain_output.info_domain_data.transfer_time = Nullable<boost::posix_time::ptime>();
        test_info_domain_output.info_domain_data.expiration_date = boost::gregorian::from_simple_string("2012-06-30");
        test_info_domain_output.info_domain_data.authinfopw = "testauthinfo1";
        test_info_domain_output.info_domain_data.admin_contacts = Util::vector_of<::LibFred::ObjectIdHandlePair>
            (::LibFred::ObjectIdHandlePair(static_cast<unsigned long long>(id_res[0]["admin_contact1_handle_id"]),admin_contact1_handle));
        test_info_domain_output.info_domain_data.enum_domain_validation = Nullable<::LibFred::ENUMValidationExtension>();
        test_info_domain_output.info_domain_data.delete_time = Nullable<boost::posix_time::ptime>();
        test_info_domain_output.info_domain_data.crhistoryid = static_cast<unsigned long long>(id_res[0]["test_fqdn_crhistoryid"]);
        test_info_domain_output.info_domain_data.historyid = static_cast<unsigned long long>(id_res[0]["test_fqdn_historyid"]);
        test_info_domain_output.info_domain_data.id = static_cast<unsigned long long>(id_res[0]["test_fqdn_id"]);
        test_info_domain_output.info_domain_data.zone = ::LibFred::ObjectIdHandlePair(2,"cz");

    }
    ~test_domain_fixture()
    {}
};

/**
 * test InfoDomain
 */
BOOST_FIXTURE_TEST_CASE(info_domain, test_domain_fixture)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoDomainOutput info_data_1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    BOOST_CHECK(test_info_domain_output == info_data_1);
    ::LibFred::InfoDomainOutput info_data_2 = ::LibFred::InfoDomainById(test_info_domain_output.info_domain_data.id).exec(ctx);
    BOOST_CHECK(test_info_domain_output == info_data_2);
    ::LibFred::InfoDomainOutput info_data_3 = ::LibFred::InfoDomainHistoryByRoid(test_info_domain_output.info_domain_data.roid).exec(ctx).at(0);
    BOOST_CHECK(test_info_domain_output == info_data_3);
    ::LibFred::InfoDomainOutput info_data_4 = ::LibFred::InfoDomainHistoryById(test_info_domain_output.info_domain_data.id).exec(ctx).at(0);
    BOOST_CHECK(test_info_domain_output == info_data_4);
    ::LibFred::InfoDomainOutput info_data_5 = ::LibFred::InfoDomainHistoryByHistoryid(test_info_domain_output.info_domain_data.historyid).exec(ctx);
    BOOST_CHECK(test_info_domain_output == info_data_5);
    ::LibFred::InfoDomainOutput info_data_6 = ::LibFred::InfoDomainByRegistrantHandle(registrant_contact_handle).exec(ctx).at(0);
    BOOST_CHECK(test_info_domain_output == info_data_6);
    ::LibFred::InfoDomainOutput info_data_7 = ::LibFred::InfoDomainByAdminContactHandle(admin_contact1_handle).exec(ctx).at(0);
    BOOST_CHECK(test_info_domain_output == info_data_7);
    ::LibFred::InfoDomainOutput info_data_8 = ::LibFred::InfoDomainByNssetHandle(test_nsset_handle).exec(ctx).at(0);
    BOOST_CHECK(test_info_domain_output == info_data_8);
    ::LibFred::InfoDomainOutput info_data_9 = ::LibFred::InfoDomainByKeysetHandle(test_keyset_handle).exec(ctx).at(0);
    BOOST_CHECK(test_info_domain_output == info_data_9);

    BOOST_CHECK(::LibFred::InfoDomainHistoryByRoid(xmark+test_info_domain_output.info_domain_data.roid).exec(ctx).empty());
    BOOST_CHECK(::LibFred::InfoDomainHistoryById(0).exec(ctx).empty());
}


BOOST_FIXTURE_TEST_CASE(test_info_domain_output_timestamp, test_domain_fixture)
{
    const std::string timezone = "Europe/Prague";
    ::LibFred::OperationContextCreator ctx;
    const ::LibFred::InfoDomainOutput domain_output_by_handle              = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, timezone);
    const ::LibFred::InfoDomainOutput domain_output_by_id                  = ::LibFred::InfoDomainById(domain_output_by_handle.info_domain_data.id).exec(ctx, timezone);
    const ::LibFred::InfoDomainOutput domain_output_history_by_historyid   = ::LibFred::InfoDomainHistoryByHistoryid(domain_output_by_handle.info_domain_data.historyid).exec(ctx, timezone);
    const ::LibFred::InfoDomainOutput domain_output_history_by_id          = ::LibFred::InfoDomainHistoryById(domain_output_by_handle.info_domain_data.id).exec(ctx, timezone).at(0);
    const ::LibFred::InfoDomainOutput domain_output_history_by_roid        = ::LibFred::InfoDomainHistoryByRoid(domain_output_by_handle.info_domain_data.roid).exec(ctx, timezone).at(0);

    /* all are equal amongst themselves ... */
    BOOST_CHECK(
            domain_output_by_handle.utc_timestamp              == domain_output_by_id.utc_timestamp
        &&  domain_output_by_id.utc_timestamp                  == domain_output_history_by_historyid.utc_timestamp
        &&  domain_output_history_by_historyid.utc_timestamp   == domain_output_history_by_id.utc_timestamp
        &&  domain_output_history_by_id.utc_timestamp          == domain_output_history_by_roid.utc_timestamp
    );

    /* ... and one of them is equal to correct constant value */
    BOOST_CHECK_EQUAL(
        domain_output_by_handle.utc_timestamp,
        boost::posix_time::time_from_string( static_cast<std::string>( ctx.get_conn().exec("SELECT now()::timestamp")[0][0] ) )
    );
}

/**
 * test InfoDomainByFqdn with wrong fqdn
 */
BOOST_FIXTURE_TEST_CASE(info_domain_wrong_handle, test_domain_fixture)
{
    std::string wrong_fqdn = xmark+test_fqdn;

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoDomainOutput info_data_1 = ::LibFred::InfoDomainByFqdn(wrong_fqdn).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoDomainByFqdn::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_fqdn());
        BOOST_TEST_MESSAGE(wrong_fqdn);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_fqdn() == wrong_fqdn);
    }
}

/**
 * test InfoDomainById with wrong id
 */
BOOST_FIXTURE_TEST_CASE(info_domain_wrong_id, test_domain_fixture)
{
    unsigned long long wrong_id = 0;

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoDomainOutput info_data_1 = ::LibFred::InfoDomainById(wrong_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoDomainById::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_id());
        BOOST_TEST_MESSAGE(wrong_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_object_id() == wrong_id);
    }
}

/**
 * test InfoDomainHistoryByHistoryId with wrong id
 */
BOOST_FIXTURE_TEST_CASE(info_domain_history_wrong_historyid, test_domain_fixture)
{
    unsigned long long wrong_historyid = 0;

    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::InfoDomainOutput info_data_1 = ::LibFred::InfoDomainHistoryByHistoryid(wrong_historyid).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::InfoDomainHistoryByHistoryid::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_object_historyid());
        BOOST_TEST_MESSAGE(wrong_historyid);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
        BOOST_CHECK(ex.get_unknown_object_historyid() == wrong_historyid);
    }
}

/**
 * test InfoDomainByRegistrantHandle with unknown contact handle
 */
BOOST_FIXTURE_TEST_CASE(info_domain_unknown_registrant_handle, test_domain_fixture)
{
    std::string bad_handle = registrant_contact_handle + xmark;

    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::InfoDomainOutput> info_data = ::LibFred::InfoDomainByRegistrantHandle(bad_handle).exec(ctx);
    BOOST_CHECK(info_data.empty());
}

/**
 * test InfoDomainByAdminContactHandle with unknown contact handle
 */
BOOST_FIXTURE_TEST_CASE(info_domain_unknown_admin_handle, test_domain_fixture)
{
    std::string bad_handle = admin_contact1_handle + xmark;

    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::InfoDomainOutput> info_data = ::LibFred::InfoDomainByAdminContactHandle(bad_handle).exec(ctx);
    BOOST_CHECK(info_data.empty());
}

/**
 * test InfoDomainByNssetHandle with unknown nsset handle
 */
BOOST_FIXTURE_TEST_CASE(info_domain_unknown_nsset_handle, test_domain_fixture)
{
    std::string bad_handle = test_nsset_handle + xmark;
    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::InfoDomainOutput> info_data = ::LibFred::InfoDomainByNssetHandle(bad_handle).exec(ctx);
    BOOST_CHECK(info_data.empty());
}

/**
 * test InfoDomainByKeysetHandle with unknown nsset handle
 */
BOOST_FIXTURE_TEST_CASE(info_domain_unknown_keyset_handle, test_domain_fixture)
{
    std::string bad_handle = test_keyset_handle + xmark;
    ::LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::InfoDomainOutput> info_data = ::LibFred::InfoDomainByKeysetHandle(bad_handle).exec(ctx);
    BOOST_CHECK(info_data.empty());
}


/**
 * test call InfoDomainDiff
*/
BOOST_FIXTURE_TEST_CASE(info_domain_diff, test_domain_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoDomainOutput domain_info1 = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);
    ::LibFred::InfoDomainOutput domain_info2 = ::LibFred::InfoDomainByFqdn(test_fqdn).set_lock().exec(ctx);

    ::LibFred::InfoDomainDiff test_diff, test_empty_diff;

    //differing data
    test_diff.crhistoryid = std::make_pair(1ull,2ull);
    test_diff.historyid = std::make_pair(1ull,2ull);
    test_diff.id = std::make_pair(1ull,2ull);
    test_diff.delete_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.fqdn = std::make_pair(std::string("testfqdn1"),std::string("testfqdn2"));
    test_diff.roid = std::make_pair(std::string("testroid1"),std::string("testroid2"));
    test_diff.sponsoring_registrar_handle = std::make_pair(std::string("testspreg1"),std::string("testspreg2"));
    test_diff.create_registrar_handle = std::make_pair(std::string("testcrreg1"),std::string("testcrreg2"));
    test_diff.update_registrar_handle = std::make_pair(Nullable<std::string>("testcrreg1"),Nullable<std::string>());
    test_diff.creation_time = std::make_pair(boost::posix_time::ptime(),boost::posix_time::second_clock::local_time());
    test_diff.update_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.transfer_time = std::make_pair(Nullable<boost::posix_time::ptime>()
                ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.authinfopw = std::make_pair(std::string("testpass1"),std::string("testpass2"));

    BOOST_TEST_MESSAGE(test_diff.to_string());
    BOOST_TEST_MESSAGE(test_empty_diff.to_string());

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_TEST_MESSAGE(::LibFred::diff_domain_data(domain_info1.info_domain_data,domain_info2.info_domain_data).to_string());
}

struct test_info_domain_order_fixture : public test_domain_fixture
{
    test_info_domain_order_fixture()
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::UpdateDomain(test_fqdn//fqdn
            , registrar_handle//registrar
            , registrant_contact_handle //registrant - owner
            , std::string("testauthinfo1") //authinfo
            , Optional<Nullable<std::string> >()//dont change nsset
            , Optional<Nullable<std::string> >()//dont change keyset
            , Util::vector_of<std::string> (admin_contact2_handle)(registrant_contact_handle) //add admin contacts
            , Util::vector_of<std::string> (admin_contact1_handle) //remove admin contacts
            , Optional<boost::gregorian::date>()//exdate
            , Optional<boost::gregorian::date>()//valexdate
            , Optional<bool>()
            , Optional<unsigned long long>() //request_id not set
            ).exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
};

/**
 * test InfoDomainHistoryByRoid output data sorted by historyid in descending order (current data first, older next)
*/

BOOST_FIXTURE_TEST_CASE(info_domain_history_order, test_info_domain_order_fixture)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoDomainOutput domain_history_info = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx);

    std::vector<::LibFred::InfoDomainOutput> domain_history_info_by_roid = ::LibFred::InfoDomainHistoryByRoid(domain_history_info.info_domain_data.roid).exec(ctx);
    BOOST_CHECK(domain_history_info_by_roid.size() == 2);
    BOOST_CHECK(domain_history_info_by_roid.at(0).info_domain_data.historyid > domain_history_info_by_roid.at(1).info_domain_data.historyid);

    BOOST_CHECK(domain_history_info_by_roid.at(0).info_domain_data.admin_contacts.at(0).handle == admin_contact2_handle);
    BOOST_CHECK(domain_history_info_by_roid.at(1).info_domain_data.admin_contacts.at(0).handle == admin_contact1_handle);

    std::vector<::LibFred::InfoDomainOutput> domain_history_info_by_id = ::LibFred::InfoDomainHistoryById(domain_history_info.info_domain_data.id).exec(ctx);
    BOOST_CHECK(domain_history_info_by_id.size() == 2);
    BOOST_CHECK(domain_history_info_by_id.at(0).info_domain_data.historyid > domain_history_info_by_roid.at(1).info_domain_data.historyid);

    BOOST_CHECK(domain_history_info_by_id.at(0).info_domain_data.admin_contacts.at(0).handle == admin_contact2_handle);
    BOOST_CHECK(domain_history_info_by_id.at(1).info_domain_data.admin_contacts.at(0).handle == admin_contact1_handle);
}

BOOST_AUTO_TEST_SUITE_END();
