#include "src/epp/nsset/nsset_create_impl.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"
#include "src/epp/nsset/nsset_dns_host_data.h"

#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <boost/foreach.hpp>

namespace Epp {

NssetCreateResult nsset_create_impl(
    Fred::OperationContext& _ctx,
    const NssetCreateInputData& _data,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Nsset::get_handle_syntax_validity(_data.handle) != Fred::NssetHandleState::SyntaxValidity::valid ) {
        AggregatedParamErrors invalid_handle_exception;
        invalid_handle_exception.add( Error(Param::nsset_handle, 0, Reason::bad_format_nsset_handle) );
        throw invalid_handle_exception;
    }

    {
        const Fred::NssetHandleState::Registrability::Enum in_registry = Fred::Nsset::get_handle_registrability(_ctx, _data.handle);

        if(in_registry == Fred::NssetHandleState::Registrability::registered) {
            throw ObjectExists();
        }

        AggregatedParamErrors exception;

        if(in_registry == Fred::NssetHandleState::Registrability::in_protection_period) {
            exception.add( Error( Param::nsset_handle, 0, Reason::protected_period ) );
        }

        //TODO error handling

        if ( !exception.is_empty() ) {
            throw exception;
        }
    }

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
