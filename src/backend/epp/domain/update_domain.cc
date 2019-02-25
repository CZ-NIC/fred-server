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
#include "src/backend/epp/domain/update_domain.hh"

#include "src/backend/epp/domain/domain_enum_validation.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/param.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/impl/util.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/domain.hh"
#include "libfred/registrable_object/domain/domain_name.hh"
#include "libfred/registrable_object/domain/enum_validation_extension.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/handle_state.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/handle_state.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/registrar_zone_access.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"

#include <boost/cast.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <set>
#include <string>
#include <vector>

namespace Epp {
namespace Domain {

namespace {

std::string to_upper(const std::string& mixed)
{
    return boost::algorithm::to_upper_copy(mixed);
}

template <class T>
class MatchesHandle
{
public:
    explicit MatchesHandle(const std::string& _handle)
        : upper_case_handle_(to_upper(_handle))
    { }
    bool operator()(const T& item) const
    {
        return item.handle == upper_case_handle_;
    }
private:
    const std::string upper_case_handle_;
};

} // namespace Epp::Domain::{anonymous}

unsigned long long update_domain(
        LibFred::OperationContext& _ctx,
        const UpdateDomainInputData& _update_domain_data,
        const UpdateDomainConfigData& _update_domain_config_data,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    // check fqdn has known zone
    LibFred::Zone::Data zone_data;
    try
    {
        zone_data = LibFred::Zone::find_zone_in_fqdn(
                _ctx,
                LibFred::Zone::rem_trailing_dot(_update_domain_data.fqdn));
    }
    catch (const LibFred::Zone::Exception& e)
    {
        if (e.is_set_unknown_zone_in_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    // start of db transaction, utc timestamp without timezone, will be timestamp of domain creation crdate
    const boost::posix_time::ptime current_utc_time =
            boost::posix_time::time_from_string(
                    static_cast<std::string>(_ctx.get_conn().exec(
                            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));

    // warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time =
            boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);

    const boost::gregorian::date current_local_date = current_local_time.date();

    if (!LibFred::is_zone_accessible_by_registrar(_session_data.registrar_id, zone_data.id, current_local_date, _ctx))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    LibFred::InfoDomainData info_domain_data_before_update;
    try
    {
        info_domain_data_before_update =
                LibFred::InfoDomainByFqdn(_update_domain_data.fqdn)
                        .set_lock()
                        .exec(_ctx, "UTC")
                        .info_domain_data;
    }
    catch (const LibFred::InfoDomainByFqdn::Exception& ex)
    {
        if (ex.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    EppResultFailure parameter_value_range_errors(EppResultCode::parameter_value_range_error);
    EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

    Optional<boost::gregorian::date> req_enum_valexdate;
    Optional<bool> enum_publish_flag;

    if (zone_data.is_enum)
    {
        if (_update_domain_data.enum_validation)
        {
            req_enum_valexdate = (*_update_domain_data.enum_validation).get_valexdate();

            if (req_enum_valexdate.get_value().is_special())
            {
                parameter_value_range_errors.add_extended_error(
                        EppExtendedError::of_scalar_parameter(
                                Param::domain_ext_val_date,
                                Reason::valexpdate_not_valid));
            }
            else
            {
                const boost::optional<boost::gregorian::date> curr_enum_valexdate =
                        info_domain_data_before_update.enum_domain_validation.isnull()
                                ? boost::optional<boost::gregorian::date>()
                                : boost::optional<boost::gregorian::date>(
                                          info_domain_data_before_update.enum_domain_validation
                                                  .get_value()
                                                  .validation_expiration);

                if (is_new_enum_domain_validation_expiration_date_invalid(
                            req_enum_valexdate.get_value(),
                            current_local_date,
                            zone_data.enum_validation_period,
                            curr_enum_valexdate,
                            _ctx))
                {
                    parameter_value_range_errors.add_extended_error(
                            EppExtendedError::of_scalar_parameter(
                                    Param::domain_ext_val_date,
                                    Reason::valexpdate_not_valid));
                }
            }

            enum_publish_flag = (*_update_domain_data.enum_validation).get_publish();
        }
    }
    else   // not enum
    {
        if (_update_domain_data.enum_validation)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::domain_ext_val_date,
                            Reason::valexpdate_not_used));
        }
    }

    if (!parameter_value_range_errors.empty())
    {
        throw EppResponseFailure(parameter_value_range_errors);
    }

    const LibFred::InfoRegistrarData session_registrar =
        LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data;

    const bool is_sponsoring_registrar = (info_domain_data_before_update.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_registrar_authorized = (is_sponsoring_registrar || is_system_registrar);

    if (!is_registrar_authorized)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::registrar_autor,
                                                         Reason::unauthorized_registrar)));
    }

    const LibFred::ObjectStatesInfo domain_states(LibFred::GetObjectStates(info_domain_data_before_update.id).exec(_ctx));

    if (!is_system_registrar)
    {
        if (domain_states.presents(LibFred::Object_State::server_update_prohibited) ||
            domain_states.presents(LibFred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    std::set<std::string> admin_contact_add_duplicity;
    for (
        std::vector<std::string>::const_iterator admin_contact_add_iter =
            _update_domain_data.admin_contacts_add.begin();
        admin_contact_add_iter != _update_domain_data.admin_contacts_add.end();
        ++admin_contact_add_iter)
    {
        if (LibFred::Contact::get_handle_registrability(
                    _ctx,
                    *admin_contact_add_iter) != LibFred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_admin_add,
                            admin_contact_add_iter - _update_domain_data.admin_contacts_add.begin(),
                            Reason::admin_notexist));
        }
        else if (
            std::find_if(info_domain_data_before_update.admin_contacts.begin(),
                         info_domain_data_before_update.admin_contacts.end(),
                         MatchesHandle<LibFred::ObjectIdHandlePair>(*admin_contact_add_iter))
            != info_domain_data_before_update.admin_contacts.end())
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_admin_add,
                            admin_contact_add_iter - _update_domain_data.admin_contacts_add.begin(),
                            Reason::admin_exist));
        }
        else if (!admin_contact_add_duplicity.insert(boost::algorithm::to_upper_copy(*admin_contact_add_iter)).second)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_admin_add,
                            admin_contact_add_iter - _update_domain_data.admin_contacts_add.begin(),
                            Reason::duplicated_contact));
        }
    }

    std::set<std::string> admin_contact_rem_duplicity;
    for (
        std::vector<std::string>::const_iterator admin_contact_rem_iter =
            _update_domain_data.admin_contacts_rem.begin();
        admin_contact_rem_iter != _update_domain_data.admin_contacts_rem.end();
        ++admin_contact_rem_iter)
    {
        if (LibFred::Contact::get_handle_registrability(_ctx, *admin_contact_rem_iter)
                != LibFred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_admin_rem,
                            admin_contact_rem_iter - _update_domain_data.admin_contacts_rem.begin(),
                            Reason::admin_notexist));
        }
        else if (
            std::find_if(info_domain_data_before_update.admin_contacts.begin(),
                         info_domain_data_before_update.admin_contacts.end(),
                         MatchesHandle<LibFred::ObjectIdHandlePair>(*admin_contact_rem_iter))
            == info_domain_data_before_update.admin_contacts.end())
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_admin_rem,
                            admin_contact_rem_iter - _update_domain_data.admin_contacts_rem.begin(),
                            Reason::admin_not_assigned));
        }
        else if (!admin_contact_rem_duplicity.insert(boost::algorithm::to_upper_copy(*admin_contact_rem_iter)).second)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_admin_rem,
                            admin_contact_rem_iter - _update_domain_data.admin_contacts_rem.begin(),
                            Reason::duplicated_contact));
        }
    }

    for (std::vector<std::string>::const_iterator tmpcontact_rem = _update_domain_data.tmpcontacts_rem.begin();
         tmpcontact_rem != _update_domain_data.tmpcontacts_rem.end();
         ++tmpcontact_rem)
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_vector_parameter(
                        Param::domain_tmpcontact,
                        tmpcontact_rem - _update_domain_data.tmpcontacts_rem.begin(),
                        Reason::tmpcontacts_obsolete));
    }

    if (_update_domain_data.nsset_chg.isset()
        && !_update_domain_data.nsset_chg.get_value().isnull()
        && (LibFred::Nsset::get_handle_registrability(
                    _ctx,
                    _update_domain_data.nsset_chg.get_value().get_value())
            != LibFred::NssetHandleState::Registrability::registered))
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_nsset,
                        Reason::nsset_notexist));
    }

    if (_update_domain_data.keyset_chg.isset()
        && !_update_domain_data.keyset_chg.get_value().isnull()
        && (LibFred::Keyset::get_handle_registrability(
                    _ctx,
                    _update_domain_data.keyset_chg.get_value().get_value())
            != LibFred::Keyset::HandleState::registered))
    {
        parameter_value_policy_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_keyset,
                        Reason::keyset_notexist));
    }

    if (_update_domain_data.registrant_chg.isset())
    {
        if (!is_system_registrar)
        {
            if (domain_states.presents(LibFred::Object_State::server_registrant_change_prohibited))
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
            }
        }

        if (LibFred::Contact::get_handle_registrability(
                    _ctx,
                    _update_domain_data.registrant_chg.get_value())
            != LibFred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_scalar_parameter(
                            Param::domain_registrant,
                            Reason::registrant_notexist));
        }
    }

    if (!parameter_value_policy_errors.empty())
    {
        throw EppResponseFailure(parameter_value_policy_errors);
    }

    const bool change_nsset_to_different_handle_requested =
            _update_domain_data.nsset_chg.isset() &&
            !_update_domain_data.nsset_chg.get_value().isnull() &&
            (info_domain_data_before_update.nsset.isnull() ||
             (to_upper(_update_domain_data.nsset_chg.get_value().get_value()) !=
              info_domain_data_before_update.nsset.get_value().handle));

    const bool change_keyset_requested = _update_domain_data.keyset_chg.isset();

    const Optional<Nullable<std::string>> keyset_chg =
            (_update_domain_config_data.rifd_epp_update_domain_keyset_clear &&
             change_nsset_to_different_handle_requested &&
             !change_keyset_requested)
                    ? Optional<Nullable<std::string>>(Nullable<std::string>())
                    : _update_domain_data.keyset_chg;

    LibFred::UpdateDomain update_domain =
            LibFred::UpdateDomain(
                    _update_domain_data.fqdn,
                    session_registrar.handle,
                    _update_domain_data.registrant_chg,
                    _update_domain_data.authinfopw_chg,
                    _update_domain_data.nsset_chg,
                    keyset_chg,
                    _update_domain_data.admin_contacts_add,
                    _update_domain_data.admin_contacts_rem,
                    Optional<boost::gregorian::date>(), // expiration_date
                    req_enum_valexdate,
                    enum_publish_flag,
                    _session_data.logd_request_id);

    try
    {
        const unsigned long long domain_new_history_id = update_domain.exec(_ctx);
        return domain_new_history_id;
    }
    catch (const LibFred::UpdateDomain::Exception& e)
    {
        if (e.is_set_unknown_domain_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        if (e.is_set_unknown_registrar_handle())
        {
            // TODO
        }

        if (e.is_set_invalid_expiration_date())
        {
            // TODO
        }

        // add_unassigned_admin_contact_handle()

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Domain
} // namespace Epp
