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

#include "src/epp/contact/info_contact_localized.h"
#include "src/epp/contact/info_contact.h"

#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/epp/action.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_response_failure_localized.h"
#include "src/epp/epp_response_success.h"
#include "src/epp/epp_response_success_localized.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/epp_result_success.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "src/epp/util.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <algorithm>
#include <set>
#include <vector>

namespace Epp {
namespace Contact {

namespace {

class FilterOut
{
public:
    static FilterOut what(const std::vector<std::string>& _disallowed)
    {
        return FilterOut(_disallowed);
    }

    std::set<std::string>& from(std::set<std::string>& _values) const
    {
        for (std::vector<std::string>::const_iterator disallowed_value_ptr = disallowed_.begin();
             disallowed_value_ptr != disallowed_.end();
            ++disallowed_value_ptr)
        {
            std::set<std::string>::iterator value_to_remove = _values.find(*disallowed_value_ptr);
            const bool remove_it = value_to_remove != _values.end();
            if (remove_it) {
                _values.erase(value_to_remove);
            }
        }
        return _values;
    }

private:
    const std::vector<std::string> disallowed_;

    explicit FilterOut(const std::vector<std::string>& _disallowed) : disallowed_(_disallowed)
    { }
};

} // namespace Epp::Contact::{anonymous}

InfoContactLocalizedResponse info_contact_localized(
        const std::string& _contact_handle,
        const SessionData& _session_data)
{
    // since no changes are comitted this transaction is reused for everything
    Fred::OperationContextCreator ctx;

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _session_data.registrar_id));
        Logging::Context logging_ctx3(_session_data.server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoContact)));

        const InfoContactOutputData info_contact_output_data =
                info_contact(
                        ctx,
                        _contact_handle,
                        _session_data.registrar_id);

        InfoContactLocalizedOutputData info_contact_localized_output_data(info_contact_output_data.disclose);
        info_contact_localized_output_data.handle                       = info_contact_output_data.handle;
        info_contact_localized_output_data.roid                         = info_contact_output_data.roid;
        info_contact_localized_output_data.sponsoring_registrar_handle  = info_contact_output_data.sponsoring_registrar_handle;
        info_contact_localized_output_data.creating_registrar_handle    = info_contact_output_data.creating_registrar_handle;
        info_contact_localized_output_data.last_update_registrar_handle = info_contact_output_data.last_update_registrar_handle;

        // compute info_contact_localized_output_data.localized_external_states
        {
            const std::vector<std::string> admin_contact_verification_states =
                Admin::AdminContactVerificationObjectStates::get_all();

            std::set<std::string> filtered_states = info_contact_output_data.states;

            // XXX HACK: Ticket #10053 - temporary hack until changed xml schemas are released upon poor registrars
            // Do not propagate admin contact verification states.
            FilterOut::what(admin_contact_verification_states).from(filtered_states);

            if (filtered_states.empty()) {//XXX HACK: OK state
                static const char *const ok_state_name = "ok";
                filtered_states.insert(ok_state_name);
            }
            info_contact_localized_output_data.localized_external_states =
                    localize_object_states_deprecated(ctx, filtered_states, _session_data.lang);
        }

        info_contact_localized_output_data.crdate            = info_contact_output_data.crdate;
        info_contact_localized_output_data.last_update       = info_contact_output_data.last_update;
        info_contact_localized_output_data.last_transfer     = info_contact_output_data.last_transfer;
        info_contact_localized_output_data.name              = info_contact_output_data.name;
        info_contact_localized_output_data.organization      = info_contact_output_data.organization;
        info_contact_localized_output_data.street1           = info_contact_output_data.street1;
        info_contact_localized_output_data.street2           = info_contact_output_data.street2;
        info_contact_localized_output_data.street3           = info_contact_output_data.street3;
        info_contact_localized_output_data.city              = info_contact_output_data.city;
        info_contact_localized_output_data.state_or_province = info_contact_output_data.state_or_province;
        info_contact_localized_output_data.postal_code       = info_contact_output_data.postal_code;
        info_contact_localized_output_data.country_code      = info_contact_output_data.country_code;
        info_contact_localized_output_data.telephone         = info_contact_output_data.telephone;
        info_contact_localized_output_data.fax               = info_contact_output_data.fax;
        info_contact_localized_output_data.email             = info_contact_output_data.email;
        info_contact_localized_output_data.notify_email      = info_contact_output_data.notify_email;
        info_contact_localized_output_data.VAT               = info_contact_output_data.VAT;
        if (info_contact_output_data.personal_id.is_initialized()) {
            info_contact_localized_output_data.ident         = info_contact_output_data.personal_id->get();
            if (info_contact_output_data.personal_id->get_type() == Fred::PersonalIdUnion::get_OP("").get_type())
            {
                info_contact_localized_output_data.identtype = InfoContactLocalizedOutputData::IdentType::op;
            }
            else if (info_contact_output_data.personal_id->get_type() == Fred::PersonalIdUnion::get_PASS("").get_type())
            {
                info_contact_localized_output_data.identtype = InfoContactLocalizedOutputData::IdentType::pass;
            }
            else if (info_contact_output_data.personal_id->get_type() == Fred::PersonalIdUnion::get_ICO("").get_type())
            {
                info_contact_localized_output_data.identtype = InfoContactLocalizedOutputData::IdentType::ico;
            }
            else if (info_contact_output_data.personal_id->get_type() == Fred::PersonalIdUnion::get_MPSV("").get_type())
            {
                info_contact_localized_output_data.identtype = InfoContactLocalizedOutputData::IdentType::mpsv;
            }
            else if (info_contact_output_data.personal_id->get_type() == Fred::PersonalIdUnion::get_BIRTHDAY("").get_type())
            {
                info_contact_localized_output_data.identtype = InfoContactLocalizedOutputData::IdentType::birthday;
            }
            else
            {
                throw std::runtime_error("Invalid ident type.");
            }
        }
        info_contact_localized_output_data.authinfopw      = info_contact_output_data.authinfopw;

        return InfoContactLocalizedResponse(
                EppResponseSuccessLocalized(
                        ctx,
                        EppResponseSuccess(EppResultSuccess(EppResultCode::command_completed_successfully)),
                        _session_data.lang),
                info_contact_localized_output_data);
    }
    catch (const EppResponseFailure& e) {
        ctx.get_log().info(std::string("info_contact_localized: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                e,
                _session_data.lang);
    }
    catch (const std::exception& e) {
        ctx.get_log().info(std::string("info_contact_localized failure: ") + e.what());
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
    catch (...) {
        ctx.get_log().info("unexpected exception in info_contact_localized function");
        throw EppResponseFailureLocalized(
                ctx,
                EppResponseFailure(EppResultFailure(EppResultCode::command_failed)),
                _session_data.lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
