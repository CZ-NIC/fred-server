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

#include "src/backend/epp/domain/info_domain_localized.hh"

#include "src/backend/epp/domain/info_domain.hh"
#include "src/backend/epp/impl/action.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/localization.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/object_states_localized.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/object/object_state.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/log/context.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <set>
#include <stdexcept>

namespace Epp {
namespace Domain {

InfoDomainLocalizedResponse info_domain_localized(
        const std::string& _fqdn,
        const InfoDomainConfigData& _info_domain_config_data,
        const SessionData& _session_data)
{
    Logging::Context logging_ctx("rifd");
    Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
    Logging::Context logging_ctx3(_session_data.server_transaction_handle);
    Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoDomain)));

    try
    {
        LibFred::OperationContextCreator ctx;

        const InfoDomainOutputData info_domain_output_data =
                info_domain(
                        ctx,
                        _fqdn,
                        _info_domain_config_data,
                        _session_data);

        const InfoDomainLocalizedOutputData info_domain_localized_output_data =
                InfoDomainLocalizedOutputData(
                        info_domain_output_data.roid,
                        info_domain_output_data.fqdn,
                        info_domain_output_data.registrant,
                        info_domain_output_data.nsset,
                        info_domain_output_data.keyset,
                        localize_object_states<StatusValue>(ctx, info_domain_output_data.states, _session_data.lang),
                        info_domain_output_data.sponsoring_registrar_handle,
                        info_domain_output_data.creating_registrar_handle,
                        info_domain_output_data.last_update_registrar_handle,
                        info_domain_output_data.crdate,
                        info_domain_output_data.last_update,
                        info_domain_output_data.last_transfer,
                        info_domain_output_data.exdate,
                        info_domain_output_data.authinfopw,
                        info_domain_output_data.admin,
                        info_domain_output_data.ext_enum_domain_validation,
                        info_domain_output_data.tmpcontact);

        return InfoDomainLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                info_domain_localized_output_data);

    }
    catch (const EppResponseFailure& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_domain_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("info_domain_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in info_domain_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }

}


} // namespace Epp::Domain
} // namespace Epp
