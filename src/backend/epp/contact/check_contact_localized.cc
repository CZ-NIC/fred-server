/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/contact/check_contact.hh"
#include "src/backend/epp/contact/check_contact_localized.hh"

#include "src/backend/epp/contact/contact_handle_registration_obstruction.hh"
#include "src/backend/epp/contact/contact_handle_registration_obstruction_localized.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/action.hh"
#include "src/backend/epp/impl/util.hh"
#include "src/backend/epp/localization.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"
#include "util/db/nullable.hh"
#include "util/log/context.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/optional.hpp>

#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace Epp {
namespace Contact {

typedef std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > HandleToObstruction;
typedef std::map<std::string,
        boost::optional<ContactHandleRegistrationObstructionLocalized> > HandleToLocalizedObstruction;

CheckContactLocalizedResponse check_contact_localized(
        const std::set<std::string>& _contact_handles,
        const CheckContactConfigData& _check_contact_config_data,
        const SessionData& _session_data)
{
    Logging::Context logging_ctx("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CheckContact)));

    try
    {
        LibFred::OperationContextCreator ctx;

        const HandleToObstruction check_contact_results =
                check_contact(
                        ctx,
                        _contact_handles,
                        _check_contact_config_data,
                        _session_data);

        return CheckContactLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                localize_check_results<ContactHandleRegistrationObstruction,
                        ContactHandleRegistrationObstructionLocalized, boost::optional>(
                        ctx,
                        check_contact_results,
                        _session_data.lang));

    }
    catch (const EppResponseFailure& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("check_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("check_contact_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in check_contact_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}


} // namespace Epp::Contact
} // namespace Epp
