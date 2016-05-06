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
#include <fredlib/domain.h>
#include <fredlib/contact.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"
#include "tests/fredlib/util.h"

const std::string server_name = "test-create-domain";

struct create_domain_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_handle;
    std::string test_enum_domain;

    create_domain_fixture()
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
        ctx.commit_transaction();
    }
    ~create_domain_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateDomain, create_domain_fixture)

/**
 * test CreateDomain with wrong registrar
 */
BOOST_AUTO_TEST_CASE(create_domain_wrong_registrar)
{
    Fred::OperationContextCreator ctx;
    std::string bad_registrar_handle = registrar_handle+xmark;

    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreateDomain(
                test_domain_handle //const std::string& fqdn
                , bad_registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        throw;
    }
    , std::exception
    , check_std_exception);

}

/**
 * test CreateDomain with wrong fqdn syntax
 */
BOOST_AUTO_TEST_CASE(create_domain_wrong_fqdn_syntax)
{
    Fred::OperationContextCreator ctx;
    std::string bad_test_domain_handle = test_domain_handle+".2bad..";
    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreateDomain(
                bad_test_domain_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_invalid_fqdn_syntax());
        throw;
    }
    , std::exception
    , check_std_exception);
}

/**
 * test CreateDomain with wrong fqdn syntax according to zone cz
 */
BOOST_AUTO_TEST_CASE(create_domain_wrong_cz_syntax)
{
    Fred::OperationContextCreator ctx;
    std::string bad_test_domain_handle = std::string("-")+test_domain_handle;
    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreateDomain(
                bad_test_domain_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_invalid_fqdn_syntax());
        throw;
    }
    , std::exception
    , check_std_exception);
}

/**
 * test CreateDomain set exdate
 */
BOOST_AUTO_TEST_CASE(create_domain_set_exdate)
{
    boost::gregorian::date exdate(boost::gregorian::from_string("2010-12-20"));
    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_expiration_date(exdate)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }
    Fred::OperationContextCreator ctx;
    Fred::InfoDomainOutput info_data_1 = Fred::InfoDomainByHandle(test_domain_handle).exec(ctx);
    BOOST_CHECK(info_data_1.info_domain_data.expiration_date == exdate);
}

/**
 * test CreateDomain set invalid exdate
 */
BOOST_AUTO_TEST_CASE(create_domain_set_wrong_exdate)
{
    boost::gregorian::date exdate;
    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_expiration_date(exdate)
        .exec(ctx);
        BOOST_ERROR("set invalid exdate and no exception thrown");
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_expiration_date());
        BOOST_CHECK(ex.get_invalid_expiration_date().is_special());
    }
}

/**
 * test CreateDomain set ENUM valexdate to ENUM domain
 */
BOOST_AUTO_TEST_CASE(create_domain_set_valexdate)
{
    boost::gregorian::date valexdate(boost::gregorian::from_string("2010-12-20"));
    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomain(test_enum_domain, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }
    Fred::OperationContextCreator ctx;
    Fred::InfoDomainOutput info_data_1 = Fred::InfoDomainByHandle(test_enum_domain).exec(ctx);
    BOOST_CHECK(info_data_1.info_domain_data.enum_domain_validation.get_value()
            .validation_expiration == valexdate);
}

/**
 * test CreateDomain set invalid ENUM valexdate to ENUM domain
 */
BOOST_AUTO_TEST_CASE(create_domain_set_wrong_valexdate)
{
    boost::gregorian::date valexdate;
    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomain(test_enum_domain, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        BOOST_ERROR("set invalid ENUM valexdate and no exception thrown");
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_invalid_enum_validation_expiration_date());
        BOOST_CHECK(ex.get_invalid_enum_validation_expiration_date().is_special());
    }
}

/**
 * test CreateDomain set ENUM valexdate to non-ENUM domain
 */
BOOST_AUTO_TEST_CASE(create_domain_set_valexdate_wrong_domain)
{
    boost::gregorian::date valexdate(boost::gregorian::from_string("2010-12-20"));
    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        BOOST_ERROR("set ENUM valexdate to non-ENUM domain and no exception thrown");
    }
    catch(const Fred::InternalError& ex)
    {
        BOOST_MESSAGE(ex.what());
    }
}

/**
 * test CreateDomain set ENUM publish flag to non-ENUM domain
 */
BOOST_AUTO_TEST_CASE(create_domain_set_publish_wrong_domain)
{
    try
    {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_publish_flag(true)
        .exec(ctx);
        BOOST_ERROR("set ENUM publish flag to non-ENUM domain and no exception thrown");
    }
    catch(const Fred::InternalError& ex)
    {
        BOOST_MESSAGE(ex.what());
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateContact

