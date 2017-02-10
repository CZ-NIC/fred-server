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
#include "tests/interfaces/public_request/fixture_common.h"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)
BOOST_AUTO_TEST_SUITE(NonRegistryEmail)

class non_registry_email_fixture : public Test::Fixture::instantiate_db_template
{
public:
    non_registry_email_fixture()
        : contact(Test::contact::make(ctx)),
          nsset(Test::nsset::make(ctx)),
          domain(Test::domain::make(ctx)),
          keyset(Test::keyset::make(ctx)),
          reason("some reason"),
          email("some@email.com")
    {
        ctx.commit_transaction();
        Registry::PublicRequestImpl pr;
        email_contact_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::contact,
                contact.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate,
                email);
        post_contact_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::contact,
                contact.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::letter_with_authenticated_signature,
                email);
        email_nsset_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::nsset,
                nsset.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate,
                email);
        post_nsset_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::nsset,
                nsset.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::letter_with_authenticated_signature,
                email);
        email_domain_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::domain,
                domain.fqdn,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate,
                email);
        post_domain_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::domain,
                domain.fqdn,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::letter_with_authenticated_signature,
                email);
        email_keyset_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::keyset,
                keyset.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate,
                email);
        post_keyset_id = pr.create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::keyset,
                keyset.handle,
                reason,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::letter_with_authenticated_signature,
                email);
    }
private:
    Fred::OperationContextCreator ctx;
public:
    const Fred::InfoContactData contact;
    const Fred::InfoNssetData nsset;
    const Fred::InfoDomainData domain;
    const Fred::InfoKeysetData keyset;
    const std::string reason;
    const std::string email;
    unsigned long long email_contact_id;
    unsigned long long post_contact_id;
    unsigned long long email_nsset_id;
    unsigned long long post_nsset_id;
    unsigned long long email_domain_id;
    unsigned long long post_domain_id;
    unsigned long long email_keyset_id;
    unsigned long long post_keyset_id;
};

BOOST_FIXTURE_TEST_CASE(authinfo_request_to_non_registry_email, non_registry_email_fixture)
{
    Fred::OperationContextCreator ctx;
    Database::Result request;
    request = get_db_public_request(ctx, email_contact_id, 2, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
    request = get_db_public_request(ctx, post_contact_id, 3, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
    request = get_db_public_request(ctx, email_nsset_id, 2, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
    request = get_db_public_request(ctx, post_nsset_id, 3, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
    request = get_db_public_request(ctx, email_domain_id, 2, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
    request = get_db_public_request(ctx, post_domain_id, 3, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
    request = get_db_public_request(ctx, email_keyset_id, 2, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
    request = get_db_public_request(ctx, post_keyset_id, 3, 0, reason, email);
    BOOST_CHECK(request.size() == 1);
}

BOOST_FIXTURE_TEST_CASE(no_object, Test::Fixture::instantiate_db_template)
{
    BOOST_CHECK_THROW(
            Registry::PublicRequestImpl().create_authinfo_request_non_registry_email(
                Registry::PublicRequestImpl::ObjectType::contact,
                "test handle",
                "some reason",
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate,
                "some@email.com"),
            Registry::PublicRequestImpl::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(invalid_email, non_registry_email_fixture)
{
    BOOST_CHECK_THROW(
            Registry::PublicRequestImpl().create_authinfo_request_non_registry_email(
                    Registry::PublicRequestImpl::ObjectType::contact,
                    contact.handle,
                    reason,
                    Optional<unsigned long long>(),
                    Registry::PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate,
                    "wrongemail"),
            Registry::PublicRequestImpl::InvalidContactEmail);
}

BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest/NonRegistryEmail
BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest
