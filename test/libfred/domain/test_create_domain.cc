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
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-create-domain";

struct create_domain_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_handle;
    std::string test_enum_domain;

    create_domain_fixture()
    : xmark(Random::Generator().get_seq(Random::CharSet::digits(), 9))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_handle ( std::string("fred")+xmark+".cz")
    , test_enum_domain ( std::string()+xmark.at(0)+'.'+xmark.at(1)+'.'+xmark.at(2)+'.'
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
    ::LibFred::OperationContextCreator ctx;
    std::string bad_registrar_handle = registrar_handle+xmark;

    BOOST_CHECK_EXCEPTION(
    try
    {
        ::LibFred::CreateDomain(
                test_domain_handle //const std::string& fqdn
                , bad_registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::CreateDomain::Exception& ex)
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
    ::LibFred::OperationContextCreator ctx;
    std::string bad_test_domain_handle = test_domain_handle+".2bad..";
    BOOST_CHECK_EXCEPTION(
    try
    {
        ::LibFred::CreateDomain(
                bad_test_domain_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::CreateDomain::Exception& ex)
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
    ::LibFred::OperationContextCreator ctx;
    std::string bad_test_domain_handle = std::string("-")+test_domain_handle;
    BOOST_CHECK_EXCEPTION(
    try
    {
        ::LibFred::CreateDomain(
                bad_test_domain_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::CreateDomain::Exception& ex)
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
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_expiration_date(exdate)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::CreateDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoDomainOutput info_data_1 = ::LibFred::InfoDomainByFqdn(test_domain_handle).exec(ctx);
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
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_expiration_date(exdate)
        .exec(ctx);
        BOOST_ERROR("set invalid exdate and no exception thrown");
    }
    catch(const ::LibFred::CreateDomain::Exception& ex)
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
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CreateDomain(test_enum_domain, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::CreateDomain::Exception& ex)
    {
        BOOST_ERROR(boost::diagnostic_information(ex));
    }
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoDomainOutput info_data_1 = ::LibFred::InfoDomainByFqdn(test_enum_domain).exec(ctx);
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
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CreateDomain(test_enum_domain, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        BOOST_ERROR("set invalid ENUM valexdate and no exception thrown");
    }
    catch(const ::LibFred::CreateDomain::Exception& ex)
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
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(valexdate)
        .exec(ctx);
        BOOST_ERROR("set ENUM valexdate to non-ENUM domain and no exception thrown");
    }
    catch(const ::LibFred::InternalError& ex)
    {
        BOOST_TEST_MESSAGE(ex.what());
    }
}

/**
 * test CreateDomain set ENUM publish flag to non-ENUM domain
 */
BOOST_AUTO_TEST_CASE(create_domain_set_publish_wrong_domain)
{
    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CreateDomain(test_domain_handle, registrar_handle, registrant_contact_handle)
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_publish_flag(true)
        .exec(ctx);
        BOOST_ERROR("set ENUM publish flag to non-ENUM domain and no exception thrown");
    }
    catch(const ::LibFred::InternalError& ex)
    {
        BOOST_TEST_MESSAGE(ex.what());
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateContact

