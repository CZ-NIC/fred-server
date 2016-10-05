/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "src/public_request/public_request.h"
#include "src/fredlib/object/get_present_object_id.h"

#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/domain/info_domain_data.h"
#include "src/fredlib/keyset/info_keyset_data.h"

#include "tests/setup/fixtures_utils.h"
#include "tests/setup/fixtures.h"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)

BOOST_AUTO_TEST_SUITE(TestNonRegistryEmail)

struct non_registry_email_fixture : Test::Fixture::instantiate_db_template //TODO only EMAIL_WITH_QUALIFIED_CERTIFICATE
{
private:
    Fred::OperationContextCreator ctx;

public:
    Fred::InfoContactData contact;
    Fred::InfoNssetData nsset;
    Fred::InfoDomainData domain;
    Fred::InfoKeysetData keyset;
    const std::string reason;
    const std::string email;
    unsigned long long contact_id;
    unsigned long long nsset_id;
    unsigned long long domain_id;
    unsigned long long keyset_id;

    non_registry_email_fixture()
    : ctx(),
      contact(Test::contact::make(ctx)),
      nsset(Test::nsset::make(ctx)),
      domain(Test::domain::make(ctx)),
      keyset(Test::keyset::make(ctx)),
      reason("some reason"),
      email("some@email.com")
    {
        ctx.commit_transaction();
        Registry::PublicRequestImpl::PublicRequest pr;
        contact_id = pr.create_authinfo_request_non_registry_email(
                Fred::Object_Type::contact,
                contact.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                email);
        nsset_id = pr.create_authinfo_request_non_registry_email(
                Fred::Object_Type::nsset,
                nsset.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                email);
        domain_id = pr.create_authinfo_request_non_registry_email(
                Fred::Object_Type::domain,
                domain.fqdn,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                email);
        keyset_id = pr.create_authinfo_request_non_registry_email(
                Fred::Object_Type::keyset,
                keyset.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                email);
    }
};

BOOST_FIXTURE_TEST_CASE(authinfo_request_to_non_registry_email, non_registry_email_fixture)
{
    Fred::OperationContextCreator ctx;
    Database::Result request = ctx.get_conn().exec_params(
            "SELECT * "
            "FROM public_request "
            "WHERE id=$1::bigint "
              "AND request_type=$2::smallint "
              "AND status=$3::smallint "
              "AND reason=$4::text "
              "AND email_to_answer=$5::text "
              "AND registrar_id IS NULL ",
            Database::query_param_list(contact_id)(2)(0)(reason)(email));
    BOOST_CHECK(request.size() == 1);
}

BOOST_FIXTURE_TEST_CASE(no_object, Test::Fixture::instantiate_db_template)
{
    BOOST_CHECK_THROW(
            Registry::PublicRequestImpl::PublicRequest().create_authinfo_request_non_registry_email(
                Fred::Object_Type::contact,
                "test handle",
                "some reason",
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                "some@email.com"),
            Fred::UnknownObject);
}

BOOST_FIXTURE_TEST_CASE(invalid_email, non_registry_email_fixture)
{
    try
    {
        Registry::PublicRequestImpl::PublicRequest().create_authinfo_request_non_registry_email(
                Fred::Object_Type::contact,
                contact.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                "wrongemail");
        BOOST_ERROR("wrong email exception awaited");
    }
    catch (const Fred::CreatePublicRequest::Exception& e)
    {
        BOOST_CHECK(e.is_set_wrong_email());
    }
}

BOOST_AUTO_TEST_SUITE_END() // TestNonRegistryEmail

BOOST_AUTO_TEST_SUITE_END() // TestPublicRequest
