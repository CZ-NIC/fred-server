#include "src/epp/nsset/nsset_create_impl.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/nsset_dns_host_data.h"
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


    bool is_unspecified_ip_addr(const boost::asio::ip::address& ipaddr)
    {
        if(ipaddr.is_v6()
            && ipaddr.to_v6().is_unspecified())
        {
             return true;
        }
        else if(ipaddr.is_v4()
            && (ipaddr.to_v4().to_ulong() == 0ul))
        {
             return true;
        }
        return false;
    }

    bool is_loopback_ip_addr(const boost::asio::ip::address& ipaddr)
    {
        if(ipaddr.is_v6()
            && ipaddr.to_v6().is_loopback())
        {
            return true;
        }
        else if(ipaddr.is_v4()
            && ((ipaddr.to_v4().to_ulong() & 0xFF000000) == 0x7F000000))
        {
            return true;
        }
        return false;
    }

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

    if(_data.tech_contacts.size() > max_nsset_tech_contacts)
    {
        ParameterValuePolicyError ex;
        for(std::size_t i = max_nsset_tech_contacts; i < _data.tech_contacts.size(); ++i)
        {
            ex.add(Error(Param::nsset_tech,
                boost::numeric_cast<unsigned short>(i+1),//position in list
                Reason::techadmin_limit));
        }
        throw ex;
    }

    //check number of nameservers
    if(_data.dns_hosts.empty()) {
        throw RequiredParameterMissing();
    }

    if(_data.dns_hosts.size() < min_nsset_dns_hosts) {
        throw ParameterValuePolicyError();
    }

    if(_data.dns_hosts.size() > max_nsset_dns_hosts)
    {
        ParameterValuePolicyError ex;
        for(std::size_t i = max_nsset_dns_hosts; i < _data.dns_hosts.size(); ++i)
        {
            ex.add(Error(Param::nsset_tech,
                boost::numeric_cast<unsigned short>(i+1),//position in list
                Reason::nsset_limit));
        }
        throw ex;
    }

    //check new nsset handle
    if( Fred::Nsset::get_handle_syntax_validity(_data.handle) != Fred::NssetHandleState::SyntaxValidity::valid ) {
        throw AggregatedParamErrors().add( Error(Param::nsset_handle, 0, Reason::bad_format_nsset_handle));
    }

    {
        const Fred::NssetHandleState::Registrability::Enum in_registry = Fred::Nsset::get_handle_registrability(_ctx, _data.handle);

        if(in_registry == Fred::NssetHandleState::Registrability::registered) {
            throw ObjectExists();
        }

        if(in_registry == Fred::NssetHandleState::Registrability::in_protection_period) {
            throw ParameterValuePolicyError().add( Error( Param::nsset_handle, 0, Reason::protected_period ) );
        }
    }

    //check technical contacts
    {
        std::map<std::string, std::size_t> tech_contact_duplicity_map;
        ParameterValuePolicyError ex;
        for(std::size_t i = 0; i < _data.tech_contacts.size(); ++i)
        {   //check technical contact exists
            if(Fred::Contact::get_handle_registrability(_ctx, _data.tech_contacts.at(i))
                != Fred::NssetHandleState::Registrability::registered)
            {
                ex.add(Error(Param::nsset_tech,
                    boost::numeric_cast<unsigned short>(i+1),//position in list
                    Reason::tech_notexist));
            }
            else
            {//check technical contact duplicity
                const std::string upper_tech_contact_handle = boost::algorithm::to_upper_copy(
                    _data.tech_contacts.at(i));
                Optional<std::size_t> duplicity = optional_map_at<Optional>(
                    tech_contact_duplicity_map, upper_tech_contact_handle);

                if(duplicity.isset())
                {
                    ex.add(Error(Param::nsset_tech,
                        boost::numeric_cast<unsigned short>(i+1),//position in list
                        Reason::duplicity_contact));
                }
                else
                {
                    tech_contact_duplicity_map[upper_tech_contact_handle] = i;
                }
            }
        }
        if(!ex.is_empty()) throw ex;
    }


    //check dns hosts wip TODO: specify required checks
    {
        std::map<std::string, std::size_t> dns_host_fqdn_duplicity_map;
        ParameterValuePolicyError ex;
        std::size_t nsset_ipaddr_position = 1;
        for(std::size_t i = 0; i < _data.dns_hosts.size(); ++i)
        {
            if(!Fred::Domain::general_domain_name_syntax_check(_data.dns_hosts.at(i).fqdn))
            {
                ex.add(Error(Param::nsset_dns_name,
                    boost::numeric_cast<unsigned short>(i+1),//position in list
                    Reason::bad_dns_name));
            }
            else
            {//check nameserver fqdn duplicity

                const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(
                        _data.dns_hosts.at(i).fqdn);
                Optional<std::size_t> duplicity = optional_map_at<Optional>(
                        dns_host_fqdn_duplicity_map, lower_dnshost_fqdn);

                if(duplicity.isset())
                {
                    ex.add(Error(Param::nsset_dns_name,
                        boost::numeric_cast<unsigned short>(i+1),//position in list
                        Reason::dns_name_exist));
                }
                else
                {
                    dns_host_fqdn_duplicity_map[lower_dnshost_fqdn] = i;
                }
            }

            //check nameserver IP addresses
            {
                std::map<boost::asio::ip::address, std::size_t> dns_host_ip_duplicity_map;
                for(std::size_t j = 0; j < _data.dns_hosts.at(i).inet_addr.size(); ++j, ++nsset_ipaddr_position)
                {
                    boost::asio::ip::address dnshostipaddr = _data.dns_hosts.at(i).inet_addr.at(j);
                    if(is_unspecified_ip_addr(dnshostipaddr) //.is_unspecified()
                    || is_loopback_ip_addr(dnshostipaddr)//.is_loopback()
                    )
                    {
                        ex.add(Error(Param::nsset_dns_addr,
                            boost::numeric_cast<unsigned short>(nsset_ipaddr_position),//position in list
                            Reason::bad_ip_address));
                    }
                    else
                    {
                        //IP address duplicity check
                        Optional<std::size_t> duplicity = optional_map_at<Optional>(
                                dns_host_ip_duplicity_map, dnshostipaddr);

                        if(duplicity.isset())
                        {
                            ex.add(Error(Param::nsset_dns_addr,
                                boost::numeric_cast<unsigned short>(nsset_ipaddr_position),//position in list
                                Reason::duplicity_dns_address));
                        }
                        else
                        {
                            dns_host_ip_duplicity_map[dnshostipaddr] = nsset_ipaddr_position;
                        }
                    }
                }
            }
        }
        if(!ex.is_empty()) throw ex;
    }
    //TODO error handling

    try {

        std::vector<Fred::DnsHost> dns_hosts;
        dns_hosts.reserve(_data.dns_hosts.size());
        BOOST_FOREACH(const Epp::DNShostData& host, _data.dns_hosts)
        {
            dns_hosts.push_back(Fred::DnsHost(host.fqdn, host.inet_addr));
        }

        const Fred::CreateNsset::Result create_data = Fred::CreateNsset(
            _data.handle,
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle,
            _data.authinfo,
            _data.tech_check_level,
            dns_hosts,
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

        /*TODO agg errors
        if( e.is_set_unknown_country() ) {
            AggregatedParamErrors exception;
            exception.add( Error( Param::contact_cc, 0, Reason::country_notexist ) );
            throw exception;
        }
        */

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
