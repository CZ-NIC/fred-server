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

#include "src/epp/nsset/update_nsset.h"

#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/impl/dns_host_input.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/epp/nsset/impl/nsset_constants.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/zone/zone.h"
#include "util/map_at.h"
#include "util/optional_value.h"
#include "util/util.h"

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>
#include <set>
#include <string>

using namespace std;

namespace Epp {
namespace Nsset {

unsigned long long update_nsset(
        Fred::OperationContext& _ctx,
        const UpdateNssetInputData& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id)
{

    if (_registrar_id == 0 ) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    if (Fred::Nsset::get_handle_registrability(_ctx, _data.handle) != Fred::NssetHandleState::Registrability::registered) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }

    struct translate_info_nsset_exception {
        static Fred::InfoNssetData exec(Fred::OperationContext& _ctx, const std::string _handle) {
            try {
                return Fred::InfoNssetByHandle(_handle).set_lock().exec(_ctx).info_nsset_data;
            } catch(const Fred::InfoNssetByHandle::Exception& e) {
                if (e.is_set_unknown_handle()) {
                    throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
                }
                throw;
            }
        }
    };

    const Fred::InfoNssetData nsset_data_before_update = translate_info_nsset_exception::exec(_ctx, _data.handle);

    const Fred::InfoRegistrarData logged_in_registrar =
            Fred::InfoRegistrarById(_registrar_id)
                    .set_lock(/* TODO lock registrar for share */)
                    .exec(_ctx)
                    .info_registrar_data;

    const bool is_sponsoring_registrar = (nsset_data_before_update.sponsoring_registrar_handle ==
                                          logged_in_registrar.handle);
    const bool is_system_registrar = logged_in_registrar.system.get_value_or(false);
    const bool operation_is_permitted = (is_sponsoring_registrar || is_system_registrar);

    if (!operation_is_permitted) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::registrar_autor,
                                                         Reason::unauthorized_registrar)));
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(nsset_data_before_update.id).exec(_ctx);
    Fred::PerformObjectStateRequest(nsset_data_before_update.id).exec(_ctx);

    if (!is_system_registrar)
    {
        const Fred::ObjectStatesInfo nsset_states_before_update(Fred::GetObjectStates(nsset_data_before_update.id).exec(_ctx));
        if (nsset_states_before_update.presents(Fred::Object_State::server_update_prohibited) ||
            nsset_states_before_update.presents(Fred::Object_State::delete_candidate))
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
        }
    }

    //lists check
    {
        EppResultFailure parameter_value_policy_errors(EppResultCode::parameter_value_policy_error);

        std::set<std::string> nsset_dns_host_fqdn;
        BOOST_FOREACH(const Fred::DnsHost& dns_hosts, nsset_data_before_update.dns_hosts)
        {
            nsset_dns_host_fqdn.insert(boost::algorithm::to_lower_copy(dns_hosts.get_fqdn()));
        }

        std::set<std::string> nsset_tech_c_handles;
        BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_c_element, nsset_data_before_update.tech_contacts)
        {
            nsset_tech_c_handles.insert(boost::algorithm::to_upper_copy(tech_c_element.handle));
        }

        //tech contacts to add check
        std::set<std::string> tech_contact_to_add_duplicity;
        for(std::size_t i = 0; i < _data.tech_contacts_add.size(); ++i)
        {
            const std::string upper_tech_contact_handle = boost::algorithm::to_upper_copy(
                _data.tech_contacts_add.at(i));

            //check technical contact exists
            if (Fred::Contact::get_handle_registrability(_ctx, _data.tech_contacts_add.at(i))
                != Fred::ContactHandleState::Registrability::registered)
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_add,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::technical_contact_not_registered));
            }

            //check if given tech contact to be added is already admin of the nsset
            if (nsset_tech_c_handles.find(upper_tech_contact_handle) != nsset_tech_c_handles.end())
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_add,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::technical_contact_already_assigned));
            }

            //check technical contact duplicity
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
        BOOST_FOREACH(const DnsHostInput& dns_host_data_to_remove, _data.dns_hosts_rem)
        {
            nsset_dns_host_fqdn_to_remove.insert(boost::algorithm::to_lower_copy(dns_host_data_to_remove.fqdn));
        }

        //tech contacts to remove check
        for(std::size_t i = 0; i < _data.tech_contacts_rem.size(); ++i)
        {
            const std::string upper_tech_contact_handle = boost::algorithm::to_upper_copy(
                _data.tech_contacts_rem.at(i));

            //check if given tech contact to remove is NOT admin of nsset
            if (nsset_tech_c_handles.find(upper_tech_contact_handle) == nsset_tech_c_handles.end())
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_rem,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::can_not_remove_tech));
            }

            //check technical contact duplicity
            if (tech_contact_to_remove_duplicity.insert(upper_tech_contact_handle).second == false)
            {
                parameter_value_policy_errors.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech_rem,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::duplicated_contact));
            }
        }

        //check dns hosts to add
        {
            std::set<std::string> dns_host_to_add_fqdn_duplicity;
            std::size_t nsset_ipaddr_to_add_position = 0;
            for(std::size_t i = 0; i < _data.dns_hosts_add.size(); ++i)
            {
                const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(
                        _data.dns_hosts_add.at(i).fqdn);

                if (!Fred::Domain::is_rfc1123_compliant_host_name(_data.dns_hosts_add.at(i).fqdn))
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_add,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::bad_dns_name));
                }

                //check dns host duplicity
                if (dns_host_to_add_fqdn_duplicity.insert(lower_dnshost_fqdn).second == false)
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_add,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::duplicated_dns_name));
                }

                check_disallowed_glue_ipaddrs(_data.dns_hosts_add.at(i), nsset_ipaddr_to_add_position, parameter_value_policy_errors, _ctx);

                //nameserver fqdn alredy assigned to nsset and not in list of fqdn to be removed
                if ((nsset_dns_host_fqdn.find(lower_dnshost_fqdn) != nsset_dns_host_fqdn.end())//dns host fqdn to be added is alredy assigned to nsset
                    && (nsset_dns_host_fqdn_to_remove.find(lower_dnshost_fqdn) == nsset_dns_host_fqdn_to_remove.end())//dns host fqdn to be added is not in list of fqdn to be removed (dns hosts are removed first)
                )
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_add,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::dns_name_exist));
                }

                //check nameserver IP addresses
                {
                    std::set<boost::asio::ip::address> dns_host_to_add_ip_duplicity;
                    for(std::size_t j = 0; j < _data.dns_hosts_add.at(i).inet_addr.size(); ++j, ++nsset_ipaddr_to_add_position)
                    {
                        boost::optional<boost::asio::ip::address> dnshostipaddr = _data.dns_hosts_add.at(i).inet_addr.at(j);
                        if (is_prohibited_ip_addr(dnshostipaddr, _ctx))
                        {
                            parameter_value_policy_errors.add_extended_error(
                                    EppExtendedError::of_vector_parameter(
                                            Param::nsset_dns_addr,
                                            boost::numeric_cast<unsigned short>(nsset_ipaddr_to_add_position),
                                            Reason::bad_ip_address));
                        }

                        if (dnshostipaddr.is_initialized() && dns_host_to_add_ip_duplicity.insert(dnshostipaddr.get()).second == false)
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

        //check dns hosts to remove
        {
            std::set<std::string> dns_host_to_remove_fqdn_duplicity;
            for(std::size_t i = 0; i < _data.dns_hosts_rem.size(); ++i)
            {
                const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(
                        _data.dns_hosts_rem.at(i).fqdn);

                //dns host fqdn to be removed is NOT assigned to nsset
                if (nsset_dns_host_fqdn.find(lower_dnshost_fqdn) == nsset_dns_host_fqdn.end())
                {
                    parameter_value_policy_errors.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name_rem,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::dns_name_notexist));
                }

                //check dns host duplicity
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

        if (!parameter_value_policy_errors.empty()) {
            throw EppResponseFailure(parameter_value_policy_errors);
        }
    }

    if (_data.tech_check_level
        && *_data.tech_check_level > max_nsset_tech_check_level)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_range_error));
    }

    // update itself
    {
        std::vector<std::string> dns_hosts_rem;
        dns_hosts_rem.reserve(_data.dns_hosts_rem.size());
        BOOST_FOREACH(const DnsHostInput& host, _data.dns_hosts_rem)
        {
            dns_hosts_rem.push_back(host.fqdn);
        }

        Fred::UpdateNsset update(_data.handle,
            logged_in_registrar.handle,
            _data.authinfo,
            make_fred_dns_hosts(_data.dns_hosts_add),
            dns_hosts_rem,
            _data.tech_contacts_add,
            _data.tech_contacts_rem,
            _data.tech_check_level ? Optional<short>(*_data.tech_check_level) : Optional<short>(),
            _logd_request_id
        );

        try {
            const unsigned long long new_history_id = update.exec(_ctx);

            const Fred::InfoNssetData nsset_data_after_update = translate_info_nsset_exception::exec(_ctx, _data.handle);

            if (nsset_data_after_update.tech_contacts.empty()
            || nsset_data_after_update.tech_contacts.size() > max_nsset_tech_contacts)
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
            }

            if (nsset_data_after_update.dns_hosts.size() < _data.config_nsset_min_hosts
            || nsset_data_after_update.dns_hosts.size() > _data.config_nsset_max_hosts)
            {
                throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
            }

            return new_history_id;

        } catch(const Fred::UpdateNsset::Exception& e) {

            // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
            if (e.is_set_unknown_registrar_handle()) {
                throw;
            }

            if (e.is_set_unknown_nsset_handle()) {
                throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
            }

            // in the improbable case that exception is incorrectly set
            throw;
        }
    }
}

} // namespace Epp::Nsset
} // namespace Epp
