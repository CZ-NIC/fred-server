#include "src/epp/nsset/nsset_create_impl.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/nsset_dns_host_data.h"
#include "src/epp/nsset/nsset_constants.h"

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

    if(_data.tech_contacts.size() > MAX_NSSET_TECH_CONTACTS)
    {
        ParametrValuePolicyError ex;
        for(std::size_t i = MAX_NSSET_TECH_CONTACTS; i < _data.tech_contacts.size(); ++i)
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

    if(_data.dns_hosts.size() < MIN_NSSET_DNS_HOSTS) {
        throw ParametrValuePolicyError();
    }

    if(_data.dns_hosts.size() > MAX_NSSET_DNS_HOSTS)
    {
        ParametrValuePolicyError ex;
        for(std::size_t i = MAX_NSSET_DNS_HOSTS; i < _data.dns_hosts.size(); ++i)
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
            throw ParametrValuePolicyError().add( Error( Param::nsset_handle, 0, Reason::protected_period ) );
        }
    }

    //check technical contacts
    {
        std::map<std::string, std::size_t> tech_contact_duplicity_map;
        ParametrValuePolicyError ex;
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
