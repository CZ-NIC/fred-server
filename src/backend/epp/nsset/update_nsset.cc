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
#include "src/backend/epp/nsset/update_nsset.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "src/backend/epp/impl/util.hh"
#include "src/backend/epp/nsset/dns_host_input.hh"
#include "src/backend/epp/nsset/impl/nsset.hh"
#include "src/backend/epp/nsset/impl/limits.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/domain/domain_name.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/object_has_state.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/zone/zone.hh"
#include "util/map_at.hh"
#include "util/optional_value.hh"
#include "util/util.hh"

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <set>
#include <string>
#include <vector>

using namespace std;

namespace Epp {
namespace Nsset {

unsigned long long update_nsset(
        LibFred::OperationContext& _ctx,
        const UpdateNssetInputData& _update_nsset_data,
        const UpdateNssetConfigData& _nsset_config,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    struct translate_info_nsset_exception
    {
        static LibFred::InfoNssetData exec(
                LibFred::OperationContext& _ctx,
                const std::string _handle)
        {
            try
            {
                return LibFred::InfoNssetByHandle(_handle).set_lock().exec(_ctx).info_nsset_data;
            }
            catch (const LibFred::InfoNssetByHandle::Exception& e)
            {
                if (e.is_set_unknown_handle())
                {
                    throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
                }
                throw;
            }
        }

    };

    const LibFred::InfoRegistrarData session_registrar =
            LibFred::InfoRegistrarById(_session_data.registrar_id)
                    .exec(_ctx)
                    .info_registrar_data;

    const LibFred::InfoNssetData nsset_data_before_update =
            translate_info_nsset_exception::exec(
                    _ctx,
                    _update_nsset_data.handle);

    const bool is_sponsoring_registrar = (nsset_data_before_update.sponsoring_registrar_handle ==
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

    // do it before any object state related checks
    LibFred::LockObjectStateRequestLock(nsset_data_before_update.id).exec(_ctx);
    LibFred::PerformObjectStateRequest(nsset_data_before_update.id).exec(_ctx);

    if (!is_system_registrar)
    {
        const LibFred::ObjectStatesInfo nsset_states_before_update(LibFred::GetObjectStates(
                        nsset_data_before_update.id).exec(_ctx));
        if (nsset_states_before_update.presents(LibFred::Object_State::server_update_prohibited) ||
            nsset_states_before_update.presents(LibFred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    // lists check
    {
        EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

        std::set<std::string> nsset_dns_host_fqdn;
        BOOST_FOREACH(const LibFred::DnsHost & dns_hosts, nsset_data_before_update.dns_hosts)
        {
            nsset_dns_host_fqdn.insert(boost::algorithm::to_lower_copy(dns_hosts.get_fqdn()));
        }

        std::set<std::string> nsset_tech_c_handles;
        for (const auto& tech_c_element : nsset_data_before_update.tech_contacts)
        {
            nsset_tech_c_handles.insert(boost::algorithm::to_upper_copy(tech_c_element.handle));
        }

        // tech contacts to add check
        std::set<std::string> tech_contact_to_add_duplicity;
        for (std::size_t i = 0; i < _update_nsset_data.tech_contacts_add.size(); ++i)
        {
            const std::string upper_tech_contact_handle = boost::algorithm::to_upper_copy(
                    _update_nsset_data.tech_contacts_add.at(i));

            // check technical contact exists
            if (LibFred::Contact::get_handle_registrability(_ctx, _update_nsset_data.tech_contacts_add.at(i))
                != LibFred::ContactHandleState::Registrability::registered)
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_add,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::technical_contact_not_registered));
            }

            // check if given tech contact to be added is already admin of the nsset
            if (nsset_tech_c_handles.find(upper_tech_contact_handle) != nsset_tech_c_handles.end())
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_add,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::technical_contact_already_assigned));
            }

            // check technical contact duplicity
            if (tech_contact_to_add_duplicity.insert(upper_tech_contact_handle).second == false)
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_add,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::duplicated_contact));
            }
        }

        std::set<std::string> tech_contact_to_remove_duplicity;
        std::set<std::string> nsset_dns_host_fqdn_to_remove;
        BOOST_FOREACH(const DnsHostInput &dns_host_data_to_remove, _update_nsset_data.dns_hosts_rem)
        {
            nsset_dns_host_fqdn_to_remove.insert(boost::algorithm::to_lower_copy(dns_host_data_to_remove.fqdn));
        }

        // tech contacts to remove check
        for (std::size_t i = 0; i < _update_nsset_data.tech_contacts_rem.size(); ++i)
        {
            const std::string upper_tech_contact_handle = boost::algorithm::to_upper_copy(
                    _update_nsset_data.tech_contacts_rem.at(i));

            // check if given tech contact to remove is NOT admin of nsset
            if (nsset_tech_c_handles.find(upper_tech_contact_handle) == nsset_tech_c_handles.end())
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_rem,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::can_not_remove_tech));
            }

            // check technical contact duplicity
            if (tech_contact_to_remove_duplicity.insert(upper_tech_contact_handle).second == false)
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_rem,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::duplicated_contact));
            }
        }

        // check dns hosts to add
        {
            std::set<std::string> dns_host_to_add_fqdn_duplicity;
            std::size_t nsset_ipaddr_to_add_position = 0;
            for (std::size_t i = 0; i < _update_nsset_data.dns_hosts_add.size(); ++i)
            {
                const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(
                        _update_nsset_data.dns_hosts_add.at(i).fqdn);

                if (!LibFred::Domain::is_rfc1123_compliant_host_name(_update_nsset_data.dns_hosts_add.at(i).fqdn))
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_add,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::bad_dns_name));
                }

                // check dns host duplicity
                if (dns_host_to_add_fqdn_duplicity.insert(lower_dnshost_fqdn).second == false)
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_add,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::duplicated_dns_name));
                }

                check_disallowed_glue_ipaddrs(
                        _update_nsset_data.dns_hosts_add.at(
                                i),
                        nsset_ipaddr_to_add_position,
                        parameter_value_policy_errors,
                        _ctx);

                // nameserver fqdn alredy assigned to nsset and not in list of fqdn to be removed
                if (
                        // dns host fqdn to be added is alredy assigned to nsset
                        (nsset_dns_host_fqdn.find(lower_dnshost_fqdn) != nsset_dns_host_fqdn.end())
                        &&
                         // dns host fqdn to be added is not in list of fqdn to be removed (dns hosts are removed first)
                        (nsset_dns_host_fqdn_to_remove.find(lower_dnshost_fqdn) == nsset_dns_host_fqdn_to_remove.end())
                ) {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_add,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::dns_name_exist));
                }

                // check nameserver IP addresses
                {
                    std::set<boost::asio::ip::address> dns_host_to_add_ip_duplicity;
                    for (std::size_t j = 0;
                         j < _update_nsset_data.dns_hosts_add.at(i).inet_addr.size();
                         ++j, ++nsset_ipaddr_to_add_position)
                    {
                        boost::optional<boost::asio::ip::address> dnshostipaddr =
                            _update_nsset_data.dns_hosts_add.at(i).inet_addr.at(j);
                        if (is_prohibited_ip_addr(dnshostipaddr, _ctx))
                        {
                            parameter_value_policy_errors.add_extended_error(
                                    EppExtendedError::of_vector_parameter(
                                            Param::nsset_dns_addr,
                                            boost::numeric_cast<unsigned short>(nsset_ipaddr_to_add_position),
                                            Reason::bad_ip_address));
                        }

                        if (dnshostipaddr.is_initialized() &&
                            dns_host_to_add_ip_duplicity.insert(dnshostipaddr.get()).second == false)
                        {
                            parameter_value_policy_errors.add_extended_error(
                                    EppExtendedError::of_vector_parameter(
                                            Param::nsset_dns_addr,
                                            boost::numeric_cast<unsigned short>(nsset_ipaddr_to_add_position),
                                            Reason::duplicated_dns_address));
                        }
                    }
                }
            }
        }

        // check dns hosts to remove
        {
            std::set<std::string> dns_host_to_remove_fqdn_duplicity;
            for (std::size_t i = 0; i < _update_nsset_data.dns_hosts_rem.size(); ++i)
            {
                const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(
                        _update_nsset_data.dns_hosts_rem.at(i).fqdn);

                // dns host fqdn to be removed is NOT assigned to nsset
                if (nsset_dns_host_fqdn.find(lower_dnshost_fqdn) == nsset_dns_host_fqdn.end())
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_rem,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::dns_name_notexist));
                }

                // check dns host duplicity
                if (dns_host_to_remove_fqdn_duplicity.insert(lower_dnshost_fqdn).second == false)
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_rem,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::duplicated_dns_name));
                }
            }
        }

        if (!parameter_value_policy_errors.empty())
        {
            throw EppResponseFailure(parameter_value_policy_errors);
        }
    }

    if (_update_nsset_data.tech_check_level
        && *_update_nsset_data.tech_check_level > max_nsset_tech_check_level)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_range_error));
    }

    // update itself
    {
        std::vector<std::string> dns_hosts_rem;
        dns_hosts_rem.reserve(_update_nsset_data.dns_hosts_rem.size());
        BOOST_FOREACH(const DnsHostInput &host, _update_nsset_data.dns_hosts_rem)
        {
            dns_hosts_rem.push_back(host.fqdn);
        }

        LibFred::UpdateNsset update(_update_nsset_data.handle,
                session_registrar.handle,
                _update_nsset_data.authinfopw,
                make_fred_dns_hosts(_update_nsset_data.dns_hosts_add),
                dns_hosts_rem,
                _update_nsset_data.tech_contacts_add,
                _update_nsset_data.tech_contacts_rem,
                _update_nsset_data.tech_check_level
                        ? Optional<short>(*_update_nsset_data.tech_check_level)
                        : Optional<short>(),
                _session_data.logd_request_id);

        try
        {
            const unsigned long long new_history_id = update.exec(_ctx);

            const LibFred::InfoNssetData nsset_data_after_update =
                translate_info_nsset_exception::exec(_ctx, _update_nsset_data.handle);

            if (nsset_data_after_update.tech_contacts.empty()
                || nsset_data_after_update.tech_contacts.size() > max_nsset_tech_contacts)
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
            }

            if (nsset_data_after_update.dns_hosts.size() < _nsset_config.min_hosts
                || nsset_data_after_update.dns_hosts.size() > _nsset_config.max_hosts)
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
            }

            return new_history_id;
        }
        catch (const LibFred::UpdateNsset::Exception& e)
        {

            // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
            if (e.is_set_unknown_registrar_handle())
            {
                throw;
            }

            if (e.is_set_unknown_nsset_handle())
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
            }

            // in the improbable case that exception is incorrectly set
            throw;
        }
    }
}


} // namespace Epp::Nsset
} // namespace Epp
