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

#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/contact/info_contact_data.h"

#include "tests/setup/fixtures_utils.h"
#include "tests/setup/fixtures.h"
#include "tests/interfaces/public_request/fixture_common.h"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)

BOOST_AUTO_TEST_SUITE(TestLockRequest)

struct lock_request_fixture : Test::Fixture::instantiate_db_template
{
private:
    Fred::OperationContextCreator ctx;

public:
    Fred::InfoContactData contact;
    Registry::PublicRequestImpl::PublicRequest pr;

    lock_request_fixture()
    : contact(Test::contact::make(ctx))
    {
        ctx.commit_transaction();
    }
};

void boost_check_fail_blocks(const std::string& handle)
{
    Registry::PublicRequestImpl::PublicRequest pr;
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                Registry::PublicRequestImpl::BLOCK_TRANSFER),
            Registry::PublicRequestImpl::ObjectAlreadyBlocked);
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
                Registry::PublicRequestImpl::BLOCK_TRANSFER),
            Registry::PublicRequestImpl::ObjectAlreadyBlocked);
}

void make_fake_state_request(
    Fred::OperationContext& ctx,
    const unsigned long long id,
    const bool block_update = false)
{
    if (block_update)
    {
        ctx.get_conn().exec_params(
                "INSERT INTO object_state_request (object_id, state_id) "
                "VALUES ($1::bigint, $2::bigint), ($1::bigint, $3::bigint) ",
                Database::query_param_list(id)(3)(4)); // 3/4 to block transfer/update for obj
    }
    else
    {
        ctx.get_conn().exec_params(
                "INSERT INTO object_state_request (object_id, state_id) "
                "VALUES ($1::bigint, $2::bigint) ",
                Database::query_param_list(id)(3)); // 3 to block transfer for obj
    }
    Fred::PerformObjectStateRequest(id).exec(ctx);
}

// email blocks
BOOST_FIXTURE_TEST_CASE(block_unblock_transfer_email, lock_request_fixture) // check for fail blocks
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_transfer = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER);
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_transfer, 6, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();

    boost_check_fail_blocks(contact.handle);

    // successful unblock
    unsigned long long unblock_transfer = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
            Registry::PublicRequestImpl::UNBLOCK_TRANSFER);
    BOOST_CHECK_EQUAL(
            get_db_public_request(Fred::OperationContextCreator(), unblock_transfer, 10, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_unblock_transfer_and_update_email, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_update = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER_AND_UPDATE);
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_change, 4, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();

    unsigned long long unblock_change = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
            Registry::PublicRequestImpl::UNBLOCK_TRANSFER_AND_UPDATE);
    BOOST_CHECK_EQUAL(
            get_db_public_request(Fred::OperationContextCreator(), unblock_update, 8, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_block_update_email, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_transfer = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER);
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_transfer, 7, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();

    unsigned long long block_update = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER_AND_UPDATE);
    BOOST_CHECK_EQUAL(
            get_db_public_request(Fred::OperationContextCreator(), block_update, 5, 0).size(),
            1);
}

// post blocks
BOOST_FIXTURE_TEST_CASE(block_unblock_transfer_post, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_transfer = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER);
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_transfer, 7, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();

    unsigned long long unblock_transfer = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
            Registry::PublicRequestImpl::UNBLOCK_TRANSFER);
    BOOST_CHECK_EQUAL(
            get_db_public_request(Fred::OperationContextCreator(), unblock_transfer, 11, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_unblock_update_post, lock_request_fixture) // checks for wrong blocks
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_update = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER_AND_UPDATE);
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_update, 5, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();

    boost_check_fail_blocks(contact.handle);
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                Registry::PublicRequestImpl::BLOCK_TRANSFER_AND_UPDATE),
            Registry::PublicRequestImpl::ObjectAlreadyBlocked);
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
                Registry::PublicRequestImpl::BLOCK_TRANSFER_AND_UPDATE),
            Registry::PublicRequestImpl::ObjectAlreadyBlocked);

    unsigned long long unblock_update = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
            Registry::PublicRequestImpl::UNBLOCK_TRANSFER_AND_UPDATE);
    BOOST_CHECK_EQUAL(
            get_db_public_request(Fred::OperationContextCreator(), unblock_update, 9, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_block_transfer, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                Registry::PublicRequestImpl::BLOCK_TRANSFER),
            Registry::PublicRequestImpl::ObjectAlreadyBlocked);
}

BOOST_FIXTURE_TEST_CASE(blocked_transfer_then_unblock_update, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
                Registry::PublicRequestImpl::UNBLOCK_TRANSFER_AND_UPDATE),
            Registry::PublicRequestImpl::ObjectNotBlocked);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_block_transfer, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
                Registry::PublicRequestImpl::BLOCK_TRANSFER),
            Registry::PublicRequestImpl::ObjectAlreadyBlocked);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_unblock_transfer, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                Registry::PublicRequestImpl::UNBLOCK_TRANSFER),
            Registry::PublicRequestImpl::ObjectNotBlocked);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_block_update, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
                Registry::PublicRequestImpl::BLOCK_TRANSFER_AND_UPDATE),
            Registry::PublicRequestImpl::ObjectAlreadyBlocked);
}

BOOST_FIXTURE_TEST_CASE(unblock_not_blocked_object, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                Registry::PublicRequestImpl::UNBLOCK_TRANSFER),
            Registry::PublicRequestImpl::ObjectNotBlocked);
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
                Registry::PublicRequestImpl::UNBLOCK_TRANSFER_AND_UPDATE),
            Registry::PublicRequestImpl::ObjectNotBlocked);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_unblock_update, lock_request_fixture)
{
    Fred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            pr.create_block_unblock_request(
                Fred::Object_Type::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
                Registry::PublicRequestImpl::UNBLOCK_TRANSFER_AND_UPDATE),
            Registry::PublicRequestImpl::ObjectNotBlocked);
}

BOOST_AUTO_TEST_SUITE_END() // TestLockRequest

BOOST_AUTO_TEST_SUITE_END() // TestPublicRequest
