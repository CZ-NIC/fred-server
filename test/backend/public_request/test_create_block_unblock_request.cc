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

#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/create_block_unblock_request.hh"
#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/lock_request_type.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/object_state/perform_object_state_request.hh"
#include "src/libfred/object/object_type.hh"
#include "src/libfred/registrable_object/contact/info_contact_data.hh"

#include "test/setup/fixtures_utils.hh"
#include "test/setup/fixtures.hh"
#include "test/backend/public_request/fixture_common.hh"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)
BOOST_AUTO_TEST_SUITE(LockRequest)

class lock_request_fixture : public Test::instantiate_db_template
{
public:
    lock_request_fixture()
        : contact(Test::contact::make(ctx))
    {
        ctx.commit_transaction();
    }
private:
    ::LibFred::OperationContextCreator ctx;
public:
    ::LibFred::InfoContactData contact;
};

void boost_check_fail_blocks(const std::string& handle)
{
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::email,
                Fred::Backend::PublicRequest::LockRequestType::block_transfer),
            Fred::Backend::PublicRequest::ObjectAlreadyBlocked);
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::letter,
                Fred::Backend::PublicRequest::LockRequestType::block_transfer),
            Fred::Backend::PublicRequest::ObjectAlreadyBlocked);
}

void make_fake_state_request(
    ::LibFred::OperationContext& ctx,
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
    ::LibFred::PerformObjectStateRequest(id).exec(ctx);
}

// email blocks
BOOST_FIXTURE_TEST_CASE(block_transfer_then_unblock_transfer_email, lock_request_fixture) // checks for wrong blocks
{
    const unsigned long long block_transfer = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::email,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer);
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_transfer, 6, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();

    boost_check_fail_blocks(contact.handle);

    // successful unblock
    const unsigned long long unblock_transfer = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::email,
            Fred::Backend::PublicRequest::LockRequestType::unblock_transfer);
    BOOST_CHECK_EQUAL(
            get_db_public_request(::LibFred::OperationContextCreator(), unblock_transfer, 10, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_unblock_transfer_post, lock_request_fixture)
{
    const unsigned long long block_transfer = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::letter,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer);
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_transfer, 7, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();

    const unsigned long long unblock_transfer = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::letter,
            Fred::Backend::PublicRequest::LockRequestType::unblock_transfer);
    BOOST_CHECK_EQUAL(
            get_db_public_request(::LibFred::OperationContextCreator(), unblock_transfer, 11, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_block_update_email, lock_request_fixture)
{
    const unsigned long long block_transfer = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::email,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer);
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_transfer, 6, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();

    const unsigned long long block_update = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::email,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update);
    BOOST_CHECK_EQUAL(
            get_db_public_request(::LibFred::OperationContextCreator(), block_update, 4, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_block_update_post, lock_request_fixture)
{
    const unsigned long long block_transfer = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::letter,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer);
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE_EQUAL(get_db_public_request(ctx, block_transfer, 7, 0).size(), 1);

    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();

    const unsigned long long block_update = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::letter,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update);
    BOOST_CHECK_EQUAL(
            get_db_public_request(::LibFred::OperationContextCreator(), block_update, 5, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_unblock_update_email, lock_request_fixture)
{
    const unsigned long long block_update = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::email,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update);
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_update, 4, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();

    const unsigned long long unblock_update = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::email,
            Fred::Backend::PublicRequest::LockRequestType::unblock_transfer_and_update);
    BOOST_CHECK_EQUAL(
            get_db_public_request(::LibFred::OperationContextCreator(), unblock_update, 8, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_unblock_update_post, lock_request_fixture) // checks for wrong blocks
{
    const unsigned long long block_update = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::letter,
            Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update);
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE_EQUAL(
            get_db_public_request(ctx, block_update, 5, 0).size(),
            1);

    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();

    boost_check_fail_blocks(contact.handle);
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::email,
                Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update),
            Fred::Backend::PublicRequest::ObjectAlreadyBlocked);
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::letter,
                Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update),
            Fred::Backend::PublicRequest::ObjectAlreadyBlocked);

    const unsigned long long unblock_update = Fred::Backend::PublicRequest::create_block_unblock_request(
            Fred::Backend::PublicRequest::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Fred::Backend::PublicRequest::ConfirmedBy::letter,
            Fred::Backend::PublicRequest::LockRequestType::unblock_transfer_and_update);
    BOOST_CHECK_EQUAL(
            get_db_public_request(::LibFred::OperationContextCreator(), unblock_update, 9, 0).size(),
            1);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_block_transfer, lock_request_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::email,
                Fred::Backend::PublicRequest::LockRequestType::block_transfer),
            Fred::Backend::PublicRequest::ObjectAlreadyBlocked);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_block_transfer, lock_request_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::letter,
                Fred::Backend::PublicRequest::LockRequestType::block_transfer),
            Fred::Backend::PublicRequest::ObjectAlreadyBlocked);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_block_update, lock_request_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::letter,
                Fred::Backend::PublicRequest::LockRequestType::block_transfer_and_update),
            Fred::Backend::PublicRequest::ObjectAlreadyBlocked);
}

BOOST_FIXTURE_TEST_CASE(block_transfer_then_unblock_update, lock_request_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::letter,
                Fred::Backend::PublicRequest::LockRequestType::unblock_transfer_and_update),
            Fred::Backend::PublicRequest::HasDifferentBlock);
}

BOOST_FIXTURE_TEST_CASE(block_update_then_unblock_transfer, lock_request_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    make_fake_state_request(ctx, contact.id, true);
    ctx.commit_transaction();
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::email,
                Fred::Backend::PublicRequest::LockRequestType::unblock_transfer),
            Fred::Backend::PublicRequest::HasDifferentBlock);
}

BOOST_FIXTURE_TEST_CASE(unblock_not_blocked_object, lock_request_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::email,
                Fred::Backend::PublicRequest::LockRequestType::unblock_transfer),
            Fred::Backend::PublicRequest::ObjectNotBlocked);
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_block_unblock_request(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                Fred::Backend::PublicRequest::ConfirmedBy::letter,
                Fred::Backend::PublicRequest::LockRequestType::unblock_transfer_and_update),
            Fred::Backend::PublicRequest::ObjectNotBlocked);
}

BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest/LockRequest
BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest
