/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "test/backend/epp/poll/fixture.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/backend/epp/util.hh"
#include "src/libfred/poll/create_update_object_poll_message.hh"
#include "src/libfred/poll/create_poll_message.hh"
#include "src/libfred/registrable_object/contact/info_contact_data.hh"
#include "src/backend/epp/poll/poll_request_get_update_contact_details.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/util/tz/utc.hh"
#include "src/util/tz/get_psql_handle_of.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)
BOOST_AUTO_TEST_SUITE(PollRequestContactDetails)

namespace {

void check_equal(
    const Epp::Contact::InfoContactOutputData& output_data,
    const ::LibFred::InfoContactData& contact_data)
{
    BOOST_CHECK_EQUAL(output_data.roid, contact_data.roid);
    BOOST_CHECK_EQUAL(output_data.handle, contact_data.handle);
    BOOST_CHECK_EQUAL(output_data.sponsoring_registrar_handle, contact_data.sponsoring_registrar_handle);
    BOOST_CHECK_EQUAL(output_data.creating_registrar_handle, contact_data.create_registrar_handle);
    BOOST_CHECK_EQUAL(output_data.crdate, contact_data.creation_time);
    BOOST_CHECK_EQUAL(output_data.last_transfer, contact_data.transfer_time);
    BOOST_REQUIRE(output_data.authinfopw);
    BOOST_CHECK_EQUAL(*output_data.authinfopw, contact_data.authinfopw);
    BOOST_CHECK_EQUAL(output_data.telephone, contact_data.telephone);
    BOOST_CHECK_EQUAL(output_data.fax, contact_data.fax);
    BOOST_CHECK_EQUAL(output_data.email, contact_data.email);
    BOOST_CHECK_EQUAL(output_data.notify_email, contact_data.notifyemail);
    BOOST_CHECK_EQUAL(output_data.VAT, contact_data.vat);
    BOOST_CHECK_EQUAL(output_data.personal_id, !contact_data.ssntype.isnull());
    BOOST_CHECK_EQUAL(output_data.personal_id, !contact_data.ssn.isnull());
    if (output_data.personal_id) {
        BOOST_CHECK_EQUAL(*output_data.personal_id, ::LibFred::PersonalIdUnion::get_any_type(contact_data.ssntype.get_value(), contact_data.ssn.get_value()));
    }
    BOOST_CHECK_EQUAL(output_data.mailing_address, contact_data.addresses.find(::LibFred::ContactAddressType::MAILING) != contact_data.addresses.end());
    if (output_data.mailing_address) {
        BOOST_CHECK_EQUAL(*output_data.mailing_address, contact_data.addresses.at(::LibFred::ContactAddressType::MAILING));
    }
}

struct HasContactUpdate : virtual Test::Backend::Epp::autorollbacking_context
{
    ::LibFred::InfoContactData old_contact_data;
    ::LibFred::InfoContactData new_contact_data;

    HasContactUpdate()
    {
        Test::mark_all_messages_as_seen(ctx);
        static const char new_passwd[] = "doesntmatter_38E166961BEE";

        const Test::contact contact(ctx, Tz::get_psql_handle_of<Tz::UTC>());
        old_contact_data = new_contact_data = contact.info_data;

        new_contact_data.authinfopw = new_passwd;

        const unsigned long long new_history_id =
                ::LibFred::UpdateContactByHandle(contact.info_data.handle,
                        contact.info_data.sponsoring_registrar_handle)
                        .set_authinfo(new_passwd)
                        .exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(successful_request_contact_details, HasContactUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    Epp::Poll::PollRequestUpdateContactOutputData output;
    BOOST_CHECK_NO_THROW(output =
        Epp::Poll::poll_request_get_update_contact_details(ctx, mesage_detail.message_id, mesage_detail.registrar_id));

    check_equal(output.old_data, old_contact_data);
    check_equal(output.new_data, new_contact_data);

    BOOST_CHECK(*output.old_data.authinfopw != *output.new_data.authinfopw);
    BOOST_CHECK(output.old_data.last_update.isnull());
    BOOST_CHECK(!output.new_data.last_update.isnull());

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_FIXTURE_TEST_CASE(failed_request_contact_details, HasContactUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    const unsigned long long bogus_message_id = Test::get_nonexistent_message_id(ctx);
    const unsigned long long bogus_registrar_id = Test::get_nonexistent_registrar_id(ctx);

    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_contact_details(ctx, mesage_detail.message_id, bogus_registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_contact_details(ctx, bogus_message_id, mesage_detail.registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_contact_details(ctx, bogus_message_id, bogus_registrar_id),
        Epp::EppResponseFailure);

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
