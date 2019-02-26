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
#include "test/backend/epp/poll/fixture.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/backend/epp/util.hh"
#include "libfred/poll/create_update_object_poll_message.hh"
#include "libfred/poll/create_poll_message.hh"
#include "src/backend/epp/poll/poll_request_get_update_keyset_details.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/util/tz/utc.hh"
#include "src/util/tz/get_psql_handle_of.hh"
#include "util/optional_value.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)
BOOST_AUTO_TEST_SUITE(PollRequestKeysetDetails)

namespace {

void check_equal(
    const Epp::Keyset::InfoKeysetOutputData& output_data,
    const ::LibFred::InfoKeysetData& keyset_data)
{
    BOOST_CHECK_EQUAL(output_data.roid, keyset_data.roid);
    BOOST_CHECK_EQUAL(output_data.handle, keyset_data.handle);
    BOOST_CHECK_EQUAL(output_data.sponsoring_registrar_handle, keyset_data.sponsoring_registrar_handle);
    BOOST_CHECK_EQUAL(output_data.creating_registrar_handle, keyset_data.create_registrar_handle);

    BOOST_REQUIRE(output_data.authinfopw);

    BOOST_CHECK_EQUAL(output_data.crdate, keyset_data.creation_time);
    BOOST_CHECK_EQUAL(output_data.last_transfer, keyset_data.transfer_time);

    std::set<Epp::Keyset::DnsKey> dns_keys = output_data.dns_keys;
    BOOST_CHECK_EQUAL(dns_keys.size(), keyset_data.dns_keys.size());
    for (std::vector<::LibFred::DnsKey>::const_iterator dns_key_itr = keyset_data.dns_keys.begin();
         dns_key_itr != keyset_data.dns_keys.end();
         ++dns_key_itr)
    {
        dns_keys.erase(Epp::Keyset::DnsKey(dns_key_itr->get_flags(),
                                           dns_key_itr->get_protocol(),
                                           dns_key_itr->get_alg(),
                                           dns_key_itr->get_key()));
    }
    BOOST_CHECK_EQUAL(dns_keys.size(), 0);

    std::set<std::string> tech_contacts = output_data.tech_contacts;
    BOOST_CHECK_EQUAL(tech_contacts.size(), keyset_data.tech_contacts.size());
    for (const auto& tech_contact_itr : keyset_data.tech_contacts)
    {
        tech_contacts.erase(tech_contact_itr.handle);
    }

    BOOST_CHECK_EQUAL(tech_contacts.size(), 0);
}

struct HasKeysetUpdate : virtual Test::Backend::Epp::autorollbacking_context
{
    ::LibFred::InfoKeysetData old_keyset_data;
    ::LibFred::InfoKeysetData new_keyset_data;

    HasKeysetUpdate()
    {
        Test::mark_all_messages_as_seen(ctx);
        static const char new_passwd[] = "doesntmatter_38E166961BEE";

        const Test::keyset keyset(ctx, Optional<std::string>(), Optional<std::string>(), Tz::get_psql_handle_of<Tz::UTC>());
        old_keyset_data = new_keyset_data = keyset.info_data;

        new_keyset_data.authinfopw = new_passwd;

        unsigned long long new_history_id =
            ::LibFred::UpdateKeyset(keyset.info_data.handle,
                               keyset.info_data.sponsoring_registrar_handle
                ).set_authinfo(new_passwd).exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(successful_request_keyset_details, HasKeysetUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    Epp::Poll::PollRequestUpdateKeysetOutputData output;
    BOOST_CHECK_NO_THROW(output =
        Epp::Poll::poll_request_get_update_keyset_details(ctx, mesage_detail.message_id, mesage_detail.registrar_id));

    check_equal(output.old_data, old_keyset_data);
    check_equal(output.new_data, new_keyset_data);

    BOOST_CHECK(*output.old_data.authinfopw != *output.new_data.authinfopw);
    BOOST_CHECK(output.old_data.last_update.isnull());
    BOOST_CHECK(!output.new_data.last_update.isnull());

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_FIXTURE_TEST_CASE(failed_request_keyset_details, HasKeysetUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    const unsigned long long bogus_message_id = Test::get_nonexistent_message_id(ctx);
    const unsigned long long bogus_registrar_id = Test::get_nonexistent_registrar_id(ctx);

    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_keyset_details(ctx, mesage_detail.message_id, bogus_registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_keyset_details(ctx, bogus_message_id, mesage_detail.registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_keyset_details(ctx, bogus_message_id, bogus_registrar_id),
        Epp::EppResponseFailure);

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
