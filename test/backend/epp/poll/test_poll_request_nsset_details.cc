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

#include "test/backend/epp/poll/fixture.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/backend/epp/util.hh"
#include "src/libfred/poll/create_update_object_poll_message.hh"
#include "src/libfred/poll/create_poll_message.hh"
#include "src/backend/epp/poll/poll_request_get_update_nsset_details.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)
BOOST_AUTO_TEST_SUITE(PollRequestNssetDetails)

namespace {

void check_equal(const Epp::Nsset::InfoNssetOutputData& nsset_data, const ::LibFred::InfoNssetData& info_data)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(nsset_data.handle), info_data.handle);

    BOOST_REQUIRE(nsset_data.authinfopw);
    BOOST_CHECK_EQUAL(*nsset_data.authinfopw, info_data.authinfopw);

    BOOST_CHECK_EQUAL(nsset_data.dns_hosts.size(), info_data.dns_hosts.size());
    if (nsset_data.dns_hosts.size() == info_data.dns_hosts.size())
    {
        for (std::size_t i = 0; i < nsset_data.dns_hosts.size(); ++i)
        {
            BOOST_CHECK_EQUAL(nsset_data.dns_hosts.at(i).fqdn, info_data.dns_hosts.at(i).get_fqdn());

            BOOST_CHECK_EQUAL(nsset_data.dns_hosts.at(i).inet_addr.size(), info_data.dns_hosts.at(i).get_inet_addr().size());
            for (std::size_t j = 0; j < nsset_data.dns_hosts.size(); ++j)
            {
                BOOST_CHECK_EQUAL(nsset_data.dns_hosts.at(i).inet_addr.at(j),info_data.dns_hosts.at(i).get_inet_addr().at(j));
            }
        }
    }

    BOOST_CHECK_EQUAL(nsset_data.tech_contacts.size(), info_data.tech_contacts.size());
    if (nsset_data.tech_contacts.size() == info_data.tech_contacts.size())
    {
        for (std::size_t i = 0; i < nsset_data.tech_contacts.size(); ++i)
        {
            BOOST_CHECK_EQUAL(nsset_data.tech_contacts.at(i), info_data.tech_contacts.at(i).handle);
        }
    }

    BOOST_CHECK_EQUAL(nsset_data.tech_check_level, info_data.tech_check_level);
}

struct HasNssetUpdate : virtual Test::Backend::Epp::autorollbacking_context
{
    ::LibFred::InfoNssetData old_nsset_data;
    ::LibFred::InfoNssetData new_nsset_data;

    HasNssetUpdate()
    {
        Test::mark_all_messages_as_seen(ctx);
        static const char new_passwd[] = "doesntmatter_38E166961BEE";

        const Test::nsset nsset(ctx);
        old_nsset_data = new_nsset_data = nsset.info_data;

        new_nsset_data.authinfopw = new_passwd;

        unsigned long long new_history_id =
            ::LibFred::UpdateNsset(nsset.info_data.handle,
                              nsset.info_data.sponsoring_registrar_handle
                ).set_authinfo(new_passwd).exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(successful_request_nsset_details, HasNssetUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    try {
        Epp::Poll::PollRequestUpdateNssetOutputData output =
            Epp::Poll::poll_request_get_update_nsset_details(ctx, mesage_detail.message_id, mesage_detail.registrar_id);

        check_equal(output.old_data, old_nsset_data);
        check_equal(output.new_data, new_nsset_data);
        BOOST_CHECK(*output.old_data.authinfopw != *output.new_data.authinfopw);
    }
    catch (...) {
        BOOST_FAIL("It is necessary that Epp::Poll::poll_request_get_update_nsset_details not throw an exception here.");
    }

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_FIXTURE_TEST_CASE(failed_request_nsset_details, HasNssetUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    const unsigned long long bogus_message_id = Test::get_nonexistent_message_id(ctx);
    const unsigned long long bogus_registrar_id = Test::get_nonexistent_registrar_id(ctx);

    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_nsset_details(ctx, mesage_detail.message_id, bogus_registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_nsset_details(ctx, bogus_message_id, mesage_detail.registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_nsset_details(ctx, bogus_message_id, bogus_registrar_id),
        Epp::EppResponseFailure);

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
