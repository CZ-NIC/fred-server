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

#include "src/backend/epp/contact/info_contact_localized.hh"
#include "src/backend/epp/contact/info_contact.hh"

#include "src/backend/epp/impl/action.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/localization.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/impl/util.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/opcontext.hh"
#include "util/log/context.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <algorithm>

namespace Epp {
namespace Contact {

namespace {

InfoContactLocalizedOutputData::Address to_localized_address(
        const boost::optional<ContactData::Address>& src)
{
    InfoContactLocalizedOutputData::Address dst;
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

}//namespace Epp::Contact::{anonymous}

InfoContactLocalizedResponse info_contact_localized(
        const std::string& _contact_handle,
        const InfoContactConfigData& _info_contact_config_data,
        const SessionData& _session_data)
{
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoContact)));

    try
    {
        LibFred::OperationContextCreator ctx;
        const InfoContactOutputData src_data =
                info_contact(
                        ctx,
                        _contact_handle,
                        _info_contact_config_data,
                        _session_data);

        InfoContactLocalizedOutputData dst_data;
        dst_data.handle = src_data.handle;
        dst_data.roid = src_data.roid;
        dst_data.sponsoring_registrar_handle =
            src_data.sponsoring_registrar_handle;
        dst_data.creating_registrar_handle =
            src_data.creating_registrar_handle;
        dst_data.last_update_registrar_handle =
            src_data.last_update_registrar_handle;
        dst_data.localized_external_states =
            localize_object_states<StatusValue>(ctx, src_data.states, _session_data.lang);
        dst_data.crdate = src_data.crdate;
        dst_data.last_update = src_data.last_update;
        dst_data.last_transfer = src_data.last_transfer;
        dst_data.name = src_data.name;
        dst_data.organization = src_data.organization;
        dst_data.address = src_data.address.make_with_the_same_privacy(to_localized_address(*src_data.address));

        dst_data.mailing_address = src_data.mailing_address;
        dst_data.telephone = src_data.telephone;
        dst_data.fax = src_data.fax;
        dst_data.email = src_data.email;
        dst_data.notify_email = src_data.notify_email;
        dst_data.VAT = src_data.VAT;
        dst_data.VAT = src_data.VAT;
        dst_data.ident = src_data.personal_id;
        dst_data.authinfopw = src_data.authinfopw;

        const InfoContactLocalizedResponse response(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                dst_data);
        return response;
    }
    catch (const EppResponseFailure& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_contact_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in info_contact_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}

}//namespace Epp::Contact
}//namespace Epp
