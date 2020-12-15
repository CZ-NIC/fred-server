/*
 * Copyright (C) 2018-2020  CZ.NIC, z. s. p. o.
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

#include "test/backend/epp/contact/util.hh"
#include "test/backend/epp/poll/util.hh"
#include "test/backend/epp/util.hh"
#include "test/setup/fixtures_utils.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/poll/poll_request_get_update_contact_details.hh"
#include "src/util/tz/get_psql_handle_of.hh"
#include "src/util/tz/utc.hh"

#include "libfred/poll/create_poll_message.hh"
#include "libfred/poll/create_update_object_poll_message.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Poll)
BOOST_AUTO_TEST_SUITE(PollRequest)
BOOST_AUTO_TEST_SUITE(PollRequestContactDetails)

struct HasContactUpdateBySponsoringRegistrar : virtual Test::Backend::Epp::autorollbacking_context
{
    HasContactUpdateBySponsoringRegistrar()
    {
        Util::mark_all_messages_as_seen(ctx);

        const Test::contact contact(ctx, Optional<std::string>{}, Optional<std::string>{}, Tz::get_psql_handle_of<Tz::UTC>());
        static const char new_passwd[] = "doesntmatter_38E166961BEE";

        const unsigned long long new_history_id =
                ::LibFred::UpdateContactByHandle(contact.info_data.handle,
                        contact.info_data.sponsoring_registrar_handle)
                        .set_authinfo(new_passwd)
                        .exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

// update done by contact's sponsoring registar, but contact owns domain registered by different registrar
struct HasContactUpdateBySponsoringRegistrarButContactOwnsDomain : virtual Test::Backend::Epp::autorollbacking_context
{
    ::LibFred::InfoContactData old_contact_data;
    ::LibFred::InfoContactData new_contact_data;

    HasContactUpdateBySponsoringRegistrarButContactOwnsDomain()
    {
        Util::mark_all_messages_as_seen(ctx);

        const Test::contact contact(ctx, Optional<std::string>{}, Optional<std::string>{}, Tz::get_psql_handle_of<Tz::UTC>());
        const Test::contact different_contact(ctx, Optional<std::string>{}, Optional<std::string>{}, Tz::get_psql_handle_of<Tz::UTC>());
        static const char new_passwd[] = "doesntmatter_38E166961BEE";
        old_contact_data = new_contact_data = contact.info_data;
        new_contact_data.authinfopw = new_passwd;
        const Test::registrar different_registrar(ctx);
        LibFred::InfoDomainData domain =
                Test::exec(
                        Test::CreateX_factory<LibFred::CreateDomain>()
                                .make(different_registrar.info_data.handle, contact.info_data.handle),
                        ctx);
        const unsigned long long new_history_id =
                ::LibFred::UpdateContactByHandle(contact.info_data.handle,
                        contact.info_data.sponsoring_registrar_handle)
                        .set_authinfo(new_passwd)
                        .exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

// update done by contact's sponsoring registar, but contact administrates domain registered by different registrar
struct HasContactUpdateBySponsoringRegistrarButContactAdministratesDomain : virtual Test::Backend::Epp::autorollbacking_context
{
    ::LibFred::InfoContactData old_contact_data;
    ::LibFred::InfoContactData new_contact_data;

    HasContactUpdateBySponsoringRegistrarButContactAdministratesDomain()
    {
        Util::mark_all_messages_as_seen(ctx);

        const Test::contact contact(ctx, Optional<std::string>{}, Optional<std::string>{}, Tz::get_psql_handle_of<Tz::UTC>());
        const Test::contact different_contact(ctx, Optional<std::string>{}, Optional<std::string>{}, Tz::get_psql_handle_of<Tz::UTC>());
        static const char new_passwd[] = "doesntmatter_38E166961BEE";
        old_contact_data = new_contact_data = contact.info_data;
        new_contact_data.authinfopw = new_passwd;
        const Test::registrar different_registrar(ctx);
        LibFred::InfoDomainData domain =
                ::LibFred::InfoDomainById(
                        ::LibFred::CreateDomain("domain-with-admin-c.cz", different_registrar.info_data.handle, different_contact.info_data.handle)
                                .set_admin_contacts(std::vector<std::string>{contact.info_data.handle})
                                .exec(ctx, "UTC")
                                .create_object_result.object_id)
                        .exec(ctx, "UTC")
                        .info_domain_data;

        const unsigned long long new_history_id =
                ::LibFred::UpdateContactByHandle(contact.info_data.handle,
                        contact.info_data.sponsoring_registrar_handle)
                        .set_authinfo(new_passwd)
                        .exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

struct HasContactUpdate : virtual Test::Backend::Epp::autorollbacking_context
{
    ::LibFred::InfoContactData old_contact_data;
    ::LibFred::InfoContactData new_contact_data;

    HasContactUpdate()
    {
        Util::mark_all_messages_as_seen(ctx);

        const Test::contact contact(ctx, Optional<std::string>{}, Optional<std::string>{}, Tz::get_psql_handle_of<Tz::UTC>());
        static const char new_passwd[] = "doesntmatter_38E166961BEE";
        old_contact_data = new_contact_data = contact.info_data;
        new_contact_data.authinfopw = new_passwd;
        const Test::registrar different_registrar(ctx);

        const unsigned long long new_history_id =
                ::LibFred::UpdateContactByHandle(contact.info_data.handle,
                        different_registrar.info_data.handle)
                        .set_authinfo(new_passwd)
                        .exec(ctx);

        ::LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_history_id);
    }
};

BOOST_FIXTURE_TEST_CASE(failed_request_contact_details2, HasContactUpdateBySponsoringRegistrar)
{
    const unsigned long long before_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 0);
}

BOOST_FIXTURE_TEST_CASE(successful_request_contact_details2, HasContactUpdateBySponsoringRegistrarButContactOwnsDomain)
{
    const unsigned long long before_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Util::MessageDetail mesage_detail = Util::get_message_ids(ctx);

    ::Epp::Poll::PollRequestUpdateContactOutputData output;
    BOOST_CHECK_NO_THROW(output =
        ::Epp::Poll::poll_request_get_update_contact_details(ctx, mesage_detail.message_id, mesage_detail.registrar_id));

    Contact::check_equal_but_no_authinfopw(output.old_data, old_contact_data);
    Contact::check_equal_but_no_authinfopw(output.new_data, new_contact_data);

    BOOST_CHECK(output.old_data.last_update == boost::none);
    BOOST_CHECK(output.new_data.last_update != boost::none);

    const unsigned long long after_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_FIXTURE_TEST_CASE(successful_request_contact_details3, HasContactUpdateBySponsoringRegistrarButContactAdministratesDomain)
{
    const unsigned long long before_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Util::MessageDetail mesage_detail = Util::get_message_ids(ctx);

    ::Epp::Poll::PollRequestUpdateContactOutputData output;
    BOOST_CHECK_NO_THROW(output =
        ::Epp::Poll::poll_request_get_update_contact_details(ctx, mesage_detail.message_id, mesage_detail.registrar_id));

    Contact::check_equal_but_no_authinfopw(output.old_data, old_contact_data);
    Contact::check_equal_but_no_authinfopw(output.new_data, new_contact_data);

    BOOST_CHECK(output.old_data.last_update == boost::none);
    BOOST_CHECK(output.new_data.last_update != boost::none);

    const unsigned long long after_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_FIXTURE_TEST_CASE(successful_request_contact_details, HasContactUpdate)
{
    const unsigned long long before_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Util::MessageDetail mesage_detail = Util::get_message_ids(ctx);

    ::Epp::Poll::PollRequestUpdateContactOutputData output;
    BOOST_CHECK_NO_THROW(output =
        ::Epp::Poll::poll_request_get_update_contact_details(ctx, mesage_detail.message_id, mesage_detail.registrar_id));

    Contact::check_equal(output.old_data, old_contact_data);
    Contact::check_equal(output.new_data, new_contact_data);

    BOOST_CHECK(*output.old_data.authinfopw != *output.new_data.authinfopw);
    BOOST_CHECK(output.old_data.last_update == boost::none);
    BOOST_CHECK(output.new_data.last_update != boost::none);

    const unsigned long long after_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_FIXTURE_TEST_CASE(failed_request_contact_details, HasContactUpdate)
{
    const unsigned long long before_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_REQUIRE_EQUAL(before_message_count, 1);

    const Util::MessageDetail mesage_detail = Util::get_message_ids(ctx);

    const unsigned long long bogus_message_id = Test::get_nonexistent_message_id(ctx);
    const unsigned long long bogus_registrar_id = Test::get_nonexistent_registrar_id(ctx);

    BOOST_CHECK_THROW(
        ::Epp::Poll::poll_request_get_update_contact_details(ctx, mesage_detail.message_id, bogus_registrar_id),
        ::Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        ::Epp::Poll::poll_request_get_update_contact_details(ctx, bogus_message_id, mesage_detail.registrar_id),
        ::Epp::EppResponseFailure);
    BOOST_CHECK_THROW(
        ::Epp::Poll::poll_request_get_update_contact_details(ctx, bogus_message_id, bogus_registrar_id),
        ::Epp::EppResponseFailure);

    const unsigned long long after_message_count = Util::get_number_of_unseen_poll_messages(ctx);
    BOOST_CHECK_EQUAL(after_message_count, 1);
}

BOOST_AUTO_TEST_SUITE_END(); // Backend/Epp/Poll/PollRequest/PollRequestContactDetails
BOOST_AUTO_TEST_SUITE_END(); // Backend/Epp/Poll/PollRequest
BOOST_AUTO_TEST_SUITE_END(); // Backend/Epp/Poll
BOOST_AUTO_TEST_SUITE_END(); // Backend/Epp
BOOST_AUTO_TEST_SUITE_END(); // Backend

} // namespace Test
