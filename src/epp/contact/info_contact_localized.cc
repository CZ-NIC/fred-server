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

/**
 *  @file
 */

#include "src/epp/contact/info_contact_localized.h"
#include "src/epp/contact/info_contact.h"

#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/epp/impl/action.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"
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

    FilterOut(const std::vector<std::string>& _disallowed) : disallowed_(_disallowed)
    { }
};

} // namespace Epp::{anonymous}

InfoContactLocalizedResponse info_contact_localized(
        const std::string& _contact_handle,
        const unsigned long long _registrar_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle)
{
    // since no changes are comitted this transaction is reused for everything
    Fred::OperationContextCreator ctx;

    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoContact)));

        const InfoContactOutputData info_contact_data =
            info_contact(
                    ctx,
                    _contact_handle,
                    _lang,
                    _registrar_id);

        InfoContactLocalizedOutputData output_data(info_contact_data.disclose);
        output_data.handle                       = info_contact_data.handle;
        output_data.roid                         = info_contact_data.roid;
        output_data.sponsoring_registrar_handle  = info_contact_data.sponsoring_registrar_handle;
        output_data.creating_registrar_handle    = info_contact_data.creating_registrar_handle;
        output_data.last_update_registrar_handle = info_contact_data.last_update_registrar_handle;

        // compute output_data.localized_external_states
        {
            const std::vector<std::string> admin_contact_verification_states =
                Admin::AdminContactVerificationObjectStates::get_all();

            std::set<std::string> filtered_states = info_contact_data.states;

            // XXX HACK: Ticket #10053 - temporary hack until changed xml schemas are released upon poor registrars
            // Do not propagate admin contact verification states.
            FilterOut::what(admin_contact_verification_states).from(filtered_states);

            if (filtered_states.empty()) {//XXX HACK: OK state
                static const char *const ok_state_name = "ok";
                filtered_states.insert(ok_state_name);
            }
            output_data.localized_external_states =
                    localize_object_states_deprecated(ctx, filtered_states, _lang);
        }

        output_data.crdate            = info_contact_data.crdate;
        output_data.last_update       = info_contact_data.last_update;
        output_data.last_transfer     = info_contact_data.last_transfer;
        output_data.name              = info_contact_data.name;
        output_data.organization      = info_contact_data.organization;
        output_data.street1           = info_contact_data.street1;
        output_data.street2           = info_contact_data.street2;
        output_data.street3           = info_contact_data.street3;
        output_data.city              = info_contact_data.city;
        output_data.state_or_province = info_contact_data.state_or_province;
        output_data.postal_code       = info_contact_data.postal_code;
        output_data.country_code      = info_contact_data.country_code;
        output_data.telephone         = info_contact_data.telephone;
        output_data.fax               = info_contact_data.fax;
        output_data.email             = info_contact_data.email;
        output_data.notify_email      = info_contact_data.notify_email;
        output_data.VAT               = info_contact_data.VAT;
        if (info_contact_data.personal_id.is_initialized()) {
            output_data.ident         = info_contact_data.personal_id->get();
            if (info_contact_data.personal_id->get_type() == Fred::PersonalIdUnion::get_OP("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::op;
            }
            else if (info_contact_data.personal_id->get_type() == Fred::PersonalIdUnion::get_PASS("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::pass;
            }
            else if (info_contact_data.personal_id->get_type() == Fred::PersonalIdUnion::get_ICO("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::ico;
            }
            else if (info_contact_data.personal_id->get_type() == Fred::PersonalIdUnion::get_MPSV("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::mpsv;
            }
            else if (info_contact_data.personal_id->get_type() == Fred::PersonalIdUnion::get_BIRTHDAY("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::birthday;
            }
            else
            {
                throw std::runtime_error("Invalid ident type.");
            }
        }
        output_data.auth_info_pw      = info_contact_data.auth_info_pw;

        return InfoContactLocalizedResponse(
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang),
                output_data);

    }
    catch (const AuthErrorServerClosingConnection&) {
        throw create_localized_fail_response(
                ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang);
    }
    catch (const NonexistentHandle&) {
        ctx.get_log().info("info_contact_localized failure: NonexistentHandle");
        throw create_localized_fail_response(
                ctx,
                Response::object_not_exist,
                std::set<Error>(),
                _lang);
    }
    catch (const std::exception& e) {
        ctx.get_log().info(std::string("info_contact_localized failure: ") + e.what());
        throw create_localized_fail_response(
                ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch (...) {
        ctx.get_log().info("info_contact_localized failure: unexpected exception");
        throw create_localized_fail_response(
                ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
}

} // namespace Epp::Contact
} // namespace Epp
