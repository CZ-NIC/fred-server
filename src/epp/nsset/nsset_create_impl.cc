#include "src/epp/nsset/nsset_create_impl.h"
#include "src/epp/nsset/nsset_dns_host_input.h"
#include "src/epp/nsset/nsset_impl.h"
#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/nsset_constants.h"

#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/registrar/info_registrar.h"

#include "util/optional_value.h"
#include "util/map_at.h"
#include "util/util.h"

#include <vector>
#include <map>

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>

namespace Epp {

NssetCreateResult nsset_create_impl(
    Fred::OperationContext& _ctx,
    const NssetCreateInputData& _data,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
) {

    //check registrar logged in
    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    //check number of technical contacts
    if(_data.tech_contacts.empty()) {
        throw RequiredParameterMissing();
    }

    if(_data.tech_contacts.size() > max_nsset_tech_contacts) {
        throw ParameterValuePolicyError();
    }

    //check number of nameservers
    if(_data.dns_hosts.empty()) {
        throw RequiredParameterMissing();
    }

    if(_data.dns_hosts.size() < min_nsset_dns_hosts) {
        throw ParameterValuePolicyError();
    }

    if(_data.dns_hosts.size() > max_nsset_dns_hosts) {
        throw ParameterValuePolicyError();
    }

    //check new nsset handle
    if( Fred::Nsset::get_handle_syntax_validity(_data.handle) != Fred::NssetHandleState::SyntaxValidity::valid ) {
        throw AggregatedParamErrors().add( Error::of_scalar_parameter(Param::nsset_handle, Reason::bad_format_nsset_handle));
    }

    {
        const Fred::NssetHandleState::Registrability::Enum in_registry = Fred::Nsset::get_handle_registrability(_ctx, _data.handle);

        if(in_registry == Fred::NssetHandleState::Registrability::registered) {
            throw ObjectExists();
        }

        if(in_registry == Fred::NssetHandleState::Registrability::in_protection_period) {
            throw ParameterValuePolicyError().add( Error::of_scalar_parameter( Param::nsset_handle, Reason::protected_period ) );
        }
    }

    //check technical contacts
    {
        std::set<std::string> tech_contact_duplicity;
        ParameterValuePolicyError ex;
        for(std::size_t i = 0; i < _data.tech_contacts.size(); ++i)
        {   //check technical contact exists
            if(Fred::Contact::get_handle_registrability(_ctx, _data.tech_contacts.at(i))
                != Fred::ContactHandleState::Registrability::registered)
            {
                ex.add(Error::of_vector_parameter(Param::nsset_tech,
                    boost::numeric_cast<unsigned short>(i),
                    Reason::technical_contact_not_registered));
            }
            else
            {//check technical contact duplicity
                if(tech_contact_duplicity.insert(boost::algorithm::to_upper_copy(
                        _data.tech_contacts.at(i))).second == false)
                {
                    ex.add(Error::of_vector_parameter(Param::nsset_tech,
                        boost::numeric_cast<unsigned short>(i),
                        Reason::duplicated_contact));
                }
            }
        }
        if(!ex.is_empty()) throw ex;
    }


    //check dns hosts
    {
        std::set<std::string> dns_host_fqdn_duplicity;
        ParameterValuePolicyError ex;
        std::size_t nsset_ipaddr_position = 0;
        for(std::size_t i = 0; i < _data.dns_hosts.size(); ++i)
        {
            const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(_data.dns_hosts.at(i).fqdn);

            if(!Fred::Domain::general_domain_name_syntax_check(_data.dns_hosts.at(i).fqdn))
            {
                ex.add(Error::of_vector_parameter(Param::nsset_dns_name,
                    boost::numeric_cast<unsigned short>(i),
                    Reason::bad_dns_name));
            }
            else
            {//check nameserver fqdn duplicity
                if(dns_host_fqdn_duplicity.insert(lower_dnshost_fqdn).second == false)
                {
                    ex.add(Error::of_vector_parameter(Param::nsset_dns_name,
                        boost::numeric_cast<unsigned short>(i),
                        Reason::dns_name_exist));
                }
            }

            check_disallowed_glue_ipaddrs(_data.dns_hosts.at(i), nsset_ipaddr_position, ex, _ctx);

            //check nameserver IP addresses
            {
                std::set<boost::asio::ip::address> dns_host_ip_duplicity;
                for(std::size_t j = 0; j < _data.dns_hosts.at(i).inet_addr.size(); ++j, ++nsset_ipaddr_position)
                {
                    const boost::optional<boost::asio::ip::address> dnshostipaddr = _data.dns_hosts.at(i).inet_addr.at(j);
                    if(is_prohibited_ip_addr(dnshostipaddr, _ctx))
                    {
                        ex.add(Error::of_vector_parameter(Param::nsset_dns_addr,
                            boost::numeric_cast<unsigned short>(nsset_ipaddr_position),
                            Reason::bad_ip_address));
                    }
                    else
                    {
                        //IP address duplicity check
                        if(dnshostipaddr.is_initialized() &&
                            dns_host_ip_duplicity.insert(dnshostipaddr.get()).second == false)
                        {
                            ex.add(Error::of_vector_parameter(Param::nsset_dns_addr,
                                boost::numeric_cast<unsigned short>(nsset_ipaddr_position),
                                Reason::duplicated_dns_address));
                        }
                    }
                }
            }
        }
        if(!ex.is_empty()) throw ex;
    }

    if(_data.get_nsset_tech_check_level() > max_nsset_tech_check_level
        || _data.get_nsset_tech_check_level() < min_nsset_tech_check_level) {
        throw ParameterValueRangeError();
    }

    try {
        const Fred::CreateNsset::Result create_data = Fred::CreateNsset(
            _data.handle,
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle,
            _data.authinfo,
            _data.get_nsset_tech_check_level(),
            make_fred_dns_hosts(_data.dns_hosts),
            _data.tech_contacts,
            _logd_request_id
        ).exec(
            _ctx,
            "UTC"
        );

        return NssetCreateResult(
            create_data.create_object_result.object_id,
            create_data.create_object_result.history_id,
            create_data.creation_time
        );

    } catch (const Fred::CreateNsset::Exception& e) {

        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
        if(e.is_set_unknown_registrar_handle()) {
            throw;
        }

        if( e.is_set_invalid_nsset_handle() /* wrong exception member name */ ) {
            throw ObjectExists();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
