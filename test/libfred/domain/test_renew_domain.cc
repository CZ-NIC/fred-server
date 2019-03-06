/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
 *  RenewDomain tests
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
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-renew-domain";

struct renew_domain_fixture : virtual public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_fqdn;
    std::string test_enum_fqdn;

    renew_domain_fixture()
    : xmark(RandomDataGenerator().xnumstring(9))
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
    ~renew_domain_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestRenewDomain, renew_domain_fixture)

/**
 * test RenewDomain::Exception
 * test create and throw exception with special data
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_exception, Test::instantiate_db_template)
{
    //good path exception
    BOOST_CHECK_THROW (BOOST_THROW_EXCEPTION(::LibFred::RenewDomain::Exception().set_unknown_domain_fqdn("badfqdn.cz"));
    , ::LibFred::OperationException);

    //bad path exception exception
    BOOST_CHECK_THROW ( BOOST_THROW_EXCEPTION(::LibFred::InternalError("test error"));
    , std::exception);
}

/**
 * test RenewDomain with wrong fqdn
 */

BOOST_AUTO_TEST_CASE(renew_domain_wrong_fqdn)
{
    std::string bad_test_fqdn = "bad" + xmark + ".cz";
    try
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::RenewDomain(bad_test_fqdn, registrar_handle,
                ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx).info_domain_data.expiration_date
            ).exec(ctx);
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::RenewDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_domain_fqdn());
        BOOST_CHECK(ex.get_unknown_domain_fqdn().compare(bad_test_fqdn) == 0);
    }
}

/**
 * test RenewDomain with wrong registrar
 */
BOOST_AUTO_TEST_CASE(renew_domain_wrong_registrar)
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
        ::LibFred::RenewDomain(test_fqdn, bad_registrar_handle,
                ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx).info_domain_data.expiration_date
            ).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::RenewDomain::Exception& ex)
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
 * test InfoDomainHistoryByRoid
 * create and renew test domain
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
    //call renew
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::RenewDomain(test_fqdn, registrar_handle,
                ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx).info_domain_data.expiration_date)
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
 * test RenewDomain, good path
 */
BOOST_AUTO_TEST_CASE(renew_domain)
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
        ::LibFred::RenewDomain(test_fqdn, registrar_handle, exdate).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::RenewDomain::Exception& ex)
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
 * test RenewDomain set invalid exdate
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_wrong_exdate, renew_domain_fixture)
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
        ::LibFred::RenewDomain(test_fqdn, registrar_handle, exdate).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::RenewDomain::Exception& ex)
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
 * test RenewDomain set ENUM valexdate to ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_valexdate, renew_domain_fixture)
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
        ::LibFred::RenewDomain(test_enum_fqdn, registrar_handle,
            info_data_1.info_domain_data.expiration_date)
                .set_enum_validation_expiration(valexdate)
                .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::RenewDomain::Exception& ex)
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
 * test RenewDomain set invalid ENUM valexdate to ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_wrong_valexdate, renew_domain_fixture)
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
        ::LibFred::RenewDomain(test_enum_fqdn, registrar_handle,
            info_data_1.info_domain_data.expiration_date)
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const ::LibFred::RenewDomain::Exception& ex)
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
 * test RenewDomain set ENUM valexdate to non-ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_valexdate_wrong_domain, renew_domain_fixture)
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
        ::LibFred::RenewDomain(test_fqdn, registrar_handle,
                info_data_1.info_domain_data.expiration_date)
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
 * test RenewDomain set ENUM publish flag to non-ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_publish_wrong_domain, renew_domain_fixture)
{
    ::LibFred::InfoDomainOutput info_data_1;
    {
        ::LibFred::OperationContextCreator ctx;
        info_data_1 = ::LibFred::InfoDomainByFqdn(test_enum_fqdn).exec(ctx);
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::RenewDomain(test_fqdn, registrar_handle,
            info_data_1.info_domain_data.expiration_date)
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


BOOST_AUTO_TEST_SUITE_END();
