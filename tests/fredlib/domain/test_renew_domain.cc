/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  RenewDomain tests
 */

#include <boost/test/unit_test.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>

#include "src/fredlib/opcontext.h"
#include <fredlib/domain.h>
#include <fredlib/contact.h>
#include <fredlib/nsset.h>
#include <fredlib/keyset.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-renew-domain";

struct renew_domain_fixture : virtual public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_handle;
    std::string test_enum_domain;

    renew_domain_fixture()
    : xmark(RandomDataGenerator().xnumstring(9))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_handle ( std::string("fred")+xmark+".cz")
    , test_enum_domain ( std::string()+xmark.at(0)+'.'+xmark.at(1)+'.'+xmark.at(2)+'.'
                                        +xmark.at(3)+'.'+xmark.at(4)+'.'+xmark.at(5)+'.'
                                        +xmark.at(6)+'.'+xmark.at(7)+'.'+xmark.at(8)+".0.2.4.e164.arpa")
    {
        Fred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(registrant_contact_handle,registrar_handle)
                .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

        Fred::CreateDomain(
                test_domain_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);

        Fred::CreateDomain(
                test_enum_domain//const std::string& fqdn
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
    BOOST_CHECK_THROW (BOOST_THROW_EXCEPTION(Fred::RenewDomain::Exception().set_unknown_domain_fqdn("badfqdn.cz"));
    , Fred::OperationException);

    //bad path exception exception
    BOOST_CHECK_THROW ( BOOST_THROW_EXCEPTION(Fred::InternalError("test error"));
    , std::exception);
}

/**
 * test RenewDomain with wrong fqdn
 */

BOOST_AUTO_TEST_CASE(renew_domain_wrong_fqdn)
{
    std::string bad_test_domain_handle = "bad" + xmark + ".cz";
    try
    {
        Fred::OperationContextCreator ctx;
        Fred::RenewDomain(bad_test_domain_handle, registrar_handle,
                Fred::InfoDomainByHandle(test_domain_handle).exec(ctx).info_domain_data.expiration_date
            ).exec(ctx);
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::RenewDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_domain_fqdn());
        BOOST_CHECK(ex.get_unknown_domain_fqdn().compare(bad_test_domain_handle) == 0);
    }
}

/**
 * test RenewDomain with wrong registrar
 */
BOOST_AUTO_TEST_CASE(renew_domain_wrong_registrar)
{
    std::string bad_registrar_handle = registrar_handle+xmark;
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
    }

    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_domain_handle, bad_registrar_handle,
                Fred::InfoDomainByHandle(test_domain_handle).exec(ctx).info_domain_data.expiration_date
            ).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::RenewDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoDomainOutput info_data_2;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
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
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
    }
    //call renew
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_domain_handle, registrar_handle,
                Fred::InfoDomainByHandle(test_domain_handle).exec(ctx).info_domain_data.expiration_date)
        .exec(ctx);
        ctx.commit_transaction();
    }

    Fred::InfoDomainOutput info_data_2;
    std::vector<Fred::InfoDomainOutput> history_info_data;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
        history_info_data = Fred::InfoDomainHistoryByRoid(info_data_1.info_domain_data.roid).exec(ctx);
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
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
    }

    boost::gregorian::date exdate(boost::gregorian::from_string("2010-12-20"));

    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_domain_handle, registrar_handle, exdate).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::RenewDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    Fred::InfoDomainOutput info_data_2;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_2.info_domain_data.expiration_date == exdate);
}

/**
 * test RenewDomain set invalid exdate
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_wrong_exdate, renew_domain_fixture)
{
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
    }

    boost::gregorian::date exdate;

    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_domain_handle, registrar_handle, exdate).exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::RenewDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_expiration_date());
        BOOST_CHECK(ex.get_invalid_expiration_date().is_special());
    }

    Fred::InfoDomainOutput info_data_2;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test RenewDomain set ENUM valexdate to ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_valexdate, renew_domain_fixture)
{
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    }

    boost::gregorian::date valexdate(boost::gregorian::from_string("2010-12-20"));

    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_enum_domain, registrar_handle,
            info_data_1.info_domain_data.expiration_date)
                .set_enum_validation_expiration(valexdate)
                .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::RenewDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }

    Fred::InfoDomainOutput info_data_2;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
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
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    }

    boost::gregorian::date valexdate;

    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_enum_domain, registrar_handle,
            info_data_1.info_domain_data.expiration_date)
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::RenewDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_enum_validation_expiration_date());
        BOOST_CHECK(ex.get_invalid_enum_validation_expiration_date().is_special());
    }

    Fred::InfoDomainOutput info_data_2;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test RenewDomain set ENUM valexdate to non-ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_valexdate_wrong_domain, renew_domain_fixture)
{
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    }

    boost::gregorian::date valexdate(boost::gregorian::from_string("2010-12-20"));

    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_domain_handle, registrar_handle,
                info_data_1.info_domain_data.expiration_date)
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::InternalError& ex)
    {
        BOOST_TEST_MESSAGE(ex.what());
    }

    Fred::InfoDomainOutput info_data_2;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}

/**
 * test RenewDomain set ENUM publish flag to non-ENUM domain
 */
BOOST_FIXTURE_TEST_CASE(renew_domain_set_publish_wrong_domain, renew_domain_fixture)
{
    Fred::InfoDomainOutput info_data_1;
    {
        Fred::OperationContextCreator ctx;
        info_data_1 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    }

    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::RenewDomain(test_domain_handle, registrar_handle,
            info_data_1.info_domain_data.expiration_date)
        .set_enum_publish_flag(true)
        .exec(ctx);
        ctx.commit_transaction();
        BOOST_ERROR("no exception thrown");
    }
    catch(const Fred::InternalError& ex)
    {
        BOOST_TEST_MESSAGE(ex.what());
    }

    Fred::InfoDomainOutput info_data_2;
    {
        Fred::OperationContextCreator ctx;
        info_data_2 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    }
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_domain_data.delete_time.isnull());
}


BOOST_AUTO_TEST_SUITE_END();
