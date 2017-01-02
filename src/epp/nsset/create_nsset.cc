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

#include "src/epp/nsset/create_nsset.h"

#include "src/epp/error.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/impl/dns_host_input.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/epp/nsset/impl/nsset_constants.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/map_at.h"
#include "util/optional_value.h"
#include "util/util.h"

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>

#include <set>
#include <string>

using namespace std;

namespace Epp {
namespace Nsset {

CreateNssetResult create_nsset(
        Fred::OperationContext& _ctx,
        const Optional<unsigned long long>& _logd_request_id)
{

    // check registrar logged in
    if (_registrar_id == 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authentication_error_server_closing_connection));
    }

    // check number of technical contacts
    if (_data.tech_contacts.empty()) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::required_parameter_missing));
    }

    if (_data.tech_contacts.size() > max_nsset_tech_contacts) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
    }

    // check number of nameservers
    if (_data.dns_hosts.empty()) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::required_parameter_missing));
    }

    if (_data.dns_hosts.size() < _data.config_nsset_min_hosts) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
    }

    if (_data.dns_hosts.size() > _data.config_nsset_max_hosts) {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error));
    }

    // check new nsset handle
    if (Fred::Nsset::get_handle_syntax_validity(_data.handle) != Fred::NssetHandleState::SyntaxValidity::valid)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_syntax_error)
                                        .add_extended_error(EppExtendedError::of_scalar_parameter(
                                                Param::nsset_handle,
                                                Reason::bad_format_nsset_handle)));

    }

    {
        const Fred::NssetHandleState::Registrability::Enum in_registry = Fred::Nsset::get_handle_registrability(_ctx, _data.handle);

        if (in_registry == Fred::NssetHandleState::Registrability::registered) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
        }

        if (in_registry == Fred::NssetHandleState::Registrability::in_protection_period) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error)
                                        .add_extended_error(EppExtendedError::of_scalar_parameter(
                                                Param::nsset_handle,
                                                Reason::protected_period)));
        }
    }

    // check technical contacts
    {
        std::set<std::string> tech_contact_duplicity;
        EppResultFailure parameter_value_policy_error(EppResultCode::parameter_value_policy_error);
        for(std::size_t i = 0; i < _data.tech_contacts.size(); ++i)
        {   // check technical contact exists
            if (Fred::Contact::get_handle_registrability(_ctx, _data.tech_contacts.at(i))
                != Fred::ContactHandleState::Registrability::registered)
            {
                parameter_value_policy_error.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_tech,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::technical_contact_not_registered));
            }
            else
            {// check technical contact duplicity
                if (tech_contact_duplicity.insert(boost::algorithm::to_upper_copy(
                        _data.tech_contacts.at(i))).second == false)
                {
                    parameter_value_policy_error.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_tech,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::duplicated_contact));
                }
            }
        }
        if (!parameter_value_policy_error.empty()) {
            throw EppResponseFailure(parameter_value_policy_error);
        }
    }


    // check dns hosts
    {
        std::set<std::string> dns_host_fqdn_duplicity;
        EppResultFailure parameter_value_policy_error(EppResultCode::parameter_value_policy_error);
        std::size_t nsset_ipaddr_position = 0;
        for(std::size_t i = 0; i < _data.dns_hosts.size(); ++i)
        {
            const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(_data.dns_hosts.at(i).fqdn);

            if (!Fred::Domain::is_rfc1123_compliant_host_name(_data.dns_hosts.at(i).fqdn))
            {
                parameter_value_policy_error.add_extended_error(
                        EppExtendedError::of_vector_parameter(
                                Param::nsset_dns_name,
                                boost::numeric_cast<unsigned short>(i),
                                Reason::bad_dns_name));
            }
            else
            {// check nameserver fqdn duplicity
                if (dns_host_fqdn_duplicity.insert(lower_dnshost_fqdn).second == false)
                {
                    parameter_value_policy_error.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_name,
                                    boost::numeric_cast<unsigned short>(i),
                                    Reason::duplicated_dns_name));
                }
            }

            check_disallowed_glue_ipaddrs(_data.dns_hosts.at(i), nsset_ipaddr_position, parameter_value_policy_error, _ctx);

            // check nameserver IP addresses
            {
                std::set<boost::asio::ip::address> dns_host_ip_duplicity;
                for(std::size_t j = 0; j < _data.dns_hosts.at(i).inet_addr.size(); ++j, ++nsset_ipaddr_position)
                {
                    const boost::optional<boost::asio::ip::address> dnshostipaddr = _data.dns_hosts.at(i).inet_addr.at(j);
                    if (is_prohibited_ip_addr(dnshostipaddr, _ctx))
                    {
                        parameter_value_policy_error.add_extended_error(
                                EppExtendedError::of_vector_parameter(
                                        Param::nsset_dns_addr,
                                        boost::numeric_cast<unsigned short>(nsset_ipaddr_position),
                                        Reason::bad_ip_address));
                    }
                    else
                    {
                        // IP address duplicity check
                        if (dnshostipaddr.is_initialized() &&
                            dns_host_ip_duplicity.insert(dnshostipaddr.get()).second == false)
                        {
                            parameter_value_policy_error.add_extended_error(
                                    EppExtendedError::of_vector_parameter(
                                            Param::nsset_dns_addr,
                                            boost::numeric_cast<unsigned short>(nsset_ipaddr_position),
                                            Reason::duplicated_dns_address));
                        }
                    }
                }
            }
        }
        if (!parameter_value_policy_error.empty()) {
            throw EppResponseFailure(parameter_value_policy_error);
        }
    }

    if (_data.get_nsset_tech_check_level() > max_nsset_tech_check_level ||
        _data.get_nsset_tech_check_level() < min_nsset_tech_check_level)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_range_error));
    }

    try {
        const Fred::CreateNsset::Result create_data = Fred::CreateNsset(
            _data.handle,
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle,
            _data.authinfo ? Optional<std::string>(*_data.authinfo) : Optional<std::string>(),
            _data.get_nsset_tech_check_level(),
            make_fred_dns_hosts(_data.dns_hosts),
            _data.tech_contacts,
            _logd_request_id
        ).exec(
            _ctx,
            "UTC"
        );

        return CreateNssetResult(
            create_data.create_object_result.object_id,
            create_data.create_object_result.history_id,
            create_data.creation_time
        );

    }
    catch (const Fred::CreateNsset::Exception& e) {

        // general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle()) {
            throw;
        }

        if (e.is_set_invalid_nsset_handle() /* wrong exception member name */) {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_exists));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namespace Epp::Nsset
} // namespace Epp
