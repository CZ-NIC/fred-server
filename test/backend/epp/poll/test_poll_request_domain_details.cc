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
#include "src/backend/epp/poll/poll_request_get_update_domain_details.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/util/tz/utc.hh"
#include "src/util/tz/get_psql_handle_of.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)
BOOST_AUTO_TEST_SUITE(PollRequestDomainDetails)

namespace {

void check_equal(
    const Epp::Domain::InfoDomainOutputData& output_data,
    const ::LibFred::InfoDomainData& domain_data)
{
    BOOST_CHECK_EQUAL(output_data.roid, domain_data.roid);
    BOOST_CHECK_EQUAL(output_data.fqdn, domain_data.fqdn);
    BOOST_CHECK_EQUAL(output_data.registrant, domain_data.registrant.handle);
    BOOST_CHECK_EQUAL(output_data.nsset.get_value_or_default(), domain_data.nsset.get_value_or_default().handle);
    BOOST_CHECK_EQUAL(output_data.keyset.get_value_or_default(), domain_data.keyset.get_value_or_default().handle);
    BOOST_CHECK_EQUAL(output_data.sponsoring_registrar_handle, domain_data.sponsoring_registrar_handle);
    BOOST_CHECK_EQUAL(output_data.creating_registrar_handle, domain_data.create_registrar_handle);
    BOOST_CHECK_EQUAL(output_data.crdate, domain_data.creation_time);
    BOOST_CHECK_EQUAL(output_data.last_transfer, domain_data.transfer_time);
    BOOST_CHECK_EQUAL(output_data.exdate, domain_data.expiration_date);
    BOOST_REQUIRE(output_data.authinfopw);
    BOOST_CHECK_EQUAL(*output_data.authinfopw, domain_data.authinfopw);

    std::set<std::string> info_domain_data_admin_contacts;
    for (const auto& object_id_handle_pair : domain_data.admin_contacts)
    {
        info_domain_data_admin_contacts.insert(object_id_handle_pair.handle);
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(
        output_data.admin.begin(),
        output_data.admin.end(),
        info_domain_data_admin_contacts.begin(),
        info_domain_data_admin_contacts.end());

    BOOST_CHECK_EQUAL(
       output_data.ext_enum_domain_validation.get_value_or(Epp::Domain::EnumValidationExtension()).get_valexdate(),
       domain_data.enum_domain_validation.get_value_or_default().validation_expiration);

    BOOST_CHECK_EQUAL(
       output_data.ext_enum_domain_validation.get_value_or_default().get_publish(),
       domain_data.enum_domain_validation.get_value_or_default().publish);

    BOOST_CHECK_EQUAL(output_data.tmpcontact.size(), 0);
}

struct HasDomainUpdate : virtual Test::Backend::Epp::autorollbacking_context
{
    ::LibFred::InfoDomainData old_domain_data;
    ::LibFred::InfoDomainData new_domain_data;

    HasDomainUpdate()
    {
        Test::mark_all_messages_as_seen(ctx);
        static const char new_passwd[] = "doesntmatter_38E166961BEE";

        const Test::domain domain(ctx, Tz::get_psql_handle_of<Tz::UTC>());
        old_domain_data = new_domain_data = domain.info_data;

        new_domain_data.authinfopw = new_passwd;

        unsigned long long new_history_id =
            ::LibFred::UpdateDomain(domain.info_data.fqdn,
                               domain.info_data.sponsoring_registrar_handle
                ).set_authinfo(new_passwd).exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(successful_request_domain_details, HasDomainUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    Epp::Poll::PollRequestUpdateDomainOutputData output;
    BOOST_CHECK_NO_THROW(output =
        Epp::Poll::poll_request_get_update_domain_details(ctx, mesage_detail.message_id, mesage_detail.registrar_id));

    check_equal(output.old_data, old_domain_data);
    check_equal(output.new_data, new_domain_data);

    BOOST_CHECK(*output.old_data.authinfopw != *output.new_data.authinfopw);
    BOOST_CHECK(output.old_data.last_update.isnull());
    BOOST_CHECK(!output.new_data.last_update.isnull());

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_FIXTURE_TEST_CASE(failed_request_domain_details, HasDomainUpdate)
{
    const unsigned long long before_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Test::MessageDetail mesage_detail = Test::get_message_ids(ctx);

    const unsigned long long bogus_message_id = Test::get_nonexistent_message_id(ctx);
    const unsigned long long bogus_registrar_id = Test::get_nonexistent_registrar_id(ctx);

    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_domain_details(ctx, mesage_detail.message_id, bogus_registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_domain_details(ctx, bogus_message_id, mesage_detail.registrar_id),
        Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        Epp::Poll::poll_request_get_update_domain_details(ctx, bogus_message_id, bogus_registrar_id),
        Epp::EppResponseFailure);

    const unsigned long long after_message_count = Test::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
