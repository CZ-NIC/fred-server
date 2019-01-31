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
#include "util/log/context.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <stdexcept>

namespace Epp {
namespace Poll {

namespace {

Contact::InfoContactLocalizedOutputData::Address to_localized_address(
        const boost::optional<Contact::ContactData::Address>& src)
{
    Contact::InfoContactLocalizedOutputData::Address dst;
    if (static_cast<bool>(src))
    {
        dst.street = src->street;
        dst.city = src->city;
        dst.state_or_province = src->state_or_province;
        dst.postal_code = src->postal_code;
        dst.country_code = src->country_code;
    }
    return dst;
}

Contact::InfoContactLocalizedOutputData localize(
        LibFred::OperationContext& ctx,
        const Contact::InfoContactOutputData& src,
        const SessionData& session_data)
{
    return Contact::InfoContactLocalizedOutputData(
            src.handle,
            src.roid,
            src.sponsoring_registrar_handle,
            src.creating_registrar_handle,
            src.last_update_registrar_handle,
            localize_object_states<Contact::StatusValue>(ctx, src.states, session_data.lang),
            src.crdate,
            src.last_update,
            src.last_transfer,
            src.name,
            src.organization,
            src.address.make_with_the_same_privacy(to_localized_address(*src.address)),
            src.mailing_address,
            src.telephone,
            src.fax,
            src.email,
            src.notify_email,
            src.VAT,
            src.personal_id,
            src.authinfopw);
}

}//namespace Epp::Poll::{anonymous}

PollRequestUpdateContactLocalizedResponse poll_request_get_update_contact_details_localized(
    unsigned long long _message_id,
    const SessionData& _session_data)
{
    try
    {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::PollResponse)));

        LibFred::OperationContextCreator ctx;

        const PollRequestUpdateContactOutputData output_data =
            poll_request_get_update_contact_details(ctx, _message_id, _session_data.registrar_id);

        const auto localized_output_data = PollRequestUpdateContactLocalizedOutputData(
                localize(ctx, output_data.old_data, _session_data),
                localize(ctx, output_data.new_data, _session_data));

        return PollRequestUpdateContactLocalizedResponse(
            EppResponseSuccessLocalized(
                ctx,
                EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                _session_data.lang),
            localized_output_data);
    }
    catch (const EppResponseFailure& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_contact_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            e,
            _session_data.lang);
    }
    catch (const std::exception& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("poll_request_contact_update_details_localized: ") + e.what());
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in poll_request_contact_update_details_localized function");
        throw EppResponseFailureLocalized(
            ctx,
            EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
            _session_data.lang);
    }
}

}//namespace Epp::Poll
}//namespace Epp
