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

#include "src/backend/epp/poll/poll_request_get_update_contact_details_localized.hh"

#include "src/backend/epp/contact/contact_ident.hh"
#include "src/backend/epp/contact/info_contact_localized_output_data.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/impl/action.hh"
#include "src/backend/epp/localization.hh"
#include "src/backend/epp/poll/poll_request_get_update_contact_details.hh"
#include "src/util/log/context.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <stdexcept>

namespace Epp {
namespace Poll {

namespace {

boost::optional<Contact::ContactIdent> personal_id_to_contact_type(boost::optional<LibFred::PersonalIdUnion> _personal_id)
{
    boost::optional<Contact::ContactIdent> contact_ident;
    if (_personal_id.is_initialized())
    {
        if (_personal_id->get_type() == LibFred::PersonalIdUnion::get_OP("").get_type())
        {
            contact_ident = Contact::ContactIdentValueOf<Contact::ContactIdentType::Op>(_personal_id->get());
        }
        else if (_personal_id->get_type() == LibFred::PersonalIdUnion::get_PASS("").get_type())
        {
            contact_ident = Contact::ContactIdentValueOf<Contact::ContactIdentType::Pass>(_personal_id->get());
        }
        else if (_personal_id->get_type() == LibFred::PersonalIdUnion::get_ICO("").get_type())
        {
            contact_ident = Contact::ContactIdentValueOf<Contact::ContactIdentType::Ico>(_personal_id->get());
        }
        else if (_personal_id->get_type() == LibFred::PersonalIdUnion::get_MPSV("").get_type())
        {
            contact_ident = Contact::ContactIdentValueOf<Contact::ContactIdentType::Mpsv>(_personal_id->get());
        }
        else if (_personal_id->get_type() == LibFred::PersonalIdUnion::get_BIRTHDAY("").get_type())
        {
            contact_ident = Contact::ContactIdentValueOf<Contact::ContactIdentType::Birthday>(_personal_id->get());
        }
        else
        {
            throw std::runtime_error("Invalid ident type.");
        }
    }
    return contact_ident;
}

} // namespace Poll::Epp::{anonymous}

PollRequestUpdateContactLocalizedResponse poll_request_get_update_contact_details_localized(
    unsigned long long _message_id,
    const SessionData& _session_data)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::PollResponse)));

        LibFred::OperationContextCreator ctx;

        const PollRequestUpdateContactOutputData output_data =
            poll_request_get_update_contact_details(ctx, _message_id, _session_data.registrar_id);

        const PollRequestUpdateContactLocalizedOutputData localized_output_data =
            PollRequestUpdateContactLocalizedOutputData(
                Epp::Contact::InfoContactLocalizedOutputData(
                    output_data.old_data.handle,
                    output_data.old_data.roid,
                    output_data.old_data.sponsoring_registrar_handle,
                    output_data.old_data.creating_registrar_handle,
                    output_data.old_data.last_update_registrar_handle,
                    localize_object_states<Epp::Contact::StatusValue>(ctx, output_data.old_data.states, _session_data.lang),
                    output_data.old_data.crdate,
                    output_data.old_data.last_update,
                    output_data.old_data.last_transfer,
                    output_data.old_data.name,
                    output_data.old_data.organization,
                    output_data.old_data.street1,
                    output_data.old_data.street2,
                    output_data.old_data.street3,
                    output_data.old_data.city,
                    output_data.old_data.state_or_province,
                    output_data.old_data.postal_code,
                    output_data.old_data.country_code,
                    output_data.old_data.mailing_address,
                    output_data.old_data.telephone,
                    output_data.old_data.fax,
                    output_data.old_data.email,
                    output_data.old_data.notify_email,
                    output_data.old_data.VAT,
                    personal_id_to_contact_type(output_data.old_data.personal_id),
                    output_data.old_data.authinfopw,
                    output_data.old_data.disclose),
                Epp::Contact::InfoContactLocalizedOutputData(
                    output_data.new_data.handle,
                    output_data.new_data.roid,
                    output_data.new_data.sponsoring_registrar_handle,
                    output_data.new_data.creating_registrar_handle,
                    output_data.new_data.last_update_registrar_handle,
                    localize_object_states<Epp::Contact::StatusValue>(ctx, output_data.new_data.states, _session_data.lang),
                    output_data.new_data.crdate,
                    output_data.new_data.last_update,
                    output_data.new_data.last_transfer,
                    output_data.new_data.name,
                    output_data.new_data.organization,
                    output_data.new_data.street1,
                    output_data.new_data.street2,
                    output_data.new_data.street3,
                    output_data.new_data.city,
                    output_data.new_data.state_or_province,
                    output_data.new_data.postal_code,
                    output_data.new_data.country_code,
                    output_data.new_data.mailing_address,
                    output_data.new_data.telephone,
                    output_data.new_data.fax,
                    output_data.new_data.email,
                    output_data.new_data.notify_email,
                    output_data.new_data.VAT,
                    personal_id_to_contact_type(output_data.new_data.personal_id),
                    output_data.new_data.authinfopw,
                    output_data.new_data.disclose));

        return PollRequestUpdateContactLocalizedResponse(
            EppResponseSuccessLocalized(
                ctx,
                EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                _session_data.lang),
            localized_output_data);
    }
    catch (const EppResponseFailure& e) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_contact_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            e,
            _session_data.lang);
    }
    catch (const std::exception& e) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_contact_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
    catch (...) {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("unexpected exception in poll_request_contact_update_details_localized function"));
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
}

} // namespace Epp::Poll
} // namespace Epp
