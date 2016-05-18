#include "src/epp/contact/contact_create_impl.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {

ContactCreateResult contact_create_impl(
    Fred::OperationContext& _ctx,
    const ContactCreateInputData& _data,
    const unsigned long long _registrar_id,
    const unsigned long long _logd_request_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    {
        const Fred::ContactHandleState::InRegistry::Enum in_registry = Fred::Contact::is_handle_in_registry(_ctx, _data.handle);

        if(in_registry == Fred::ContactHandleState::InRegistry::registered) {
            throw ObjectExists();
        }

        AggregatedParamErrors exception;

        if( Fred::Contact::is_handle_valid(_data.handle) != Fred::ContactHandleState::SyntaxValidity::valid ) {
            exception.add( Error( Param::contact_handle, 0, Reason::bad_format_contact_handle ) );
        }

        if(in_registry == Fred::ContactHandleState::InRegistry::in_protection_period) {
            exception.add( Error( Param::contact_handle, 0, Reason::protected_period ) );
        }

        if ( !is_country_code_valid(_ctx, _data.country_code) ) {
            exception.add( Error( Param::contact_cc, 0, Reason::country_notexist ) );
        }

        if ( !exception.is_empty() ) {
            throw exception;
        }
    }

    try {
        const Fred::CreateContact::Result create_data = Fred::CreateContact(
            _data.handle,
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle,
            _data.authinfo,
            _data.name,
            _data.organization,
            Fred::Contact::PlaceAddress(
                _data.street1,
                _data.street2,
                _data.street3,
                _data.city,
                _data.state_or_province,
                _data.postal_code,
                _data.country_code
            ),
            _data.telephone,
            _data.fax,
            _data.email,
            _data.notify_email,
            _data.VAT,
            _data.identtype.isnull() ? Optional<std::string>() : to_db_handle(_data.identtype.get_value()),
            _data.ident,
            // will be implemented in #13744
            Optional< Fred::ContactAddressList >(),
            _data.disclose_name,
            _data.disclose_organization,
            _data.disclose_address,
            _data.disclose_telephone,
            _data.disclose_fax,
            _data.disclose_email,
            _data.disclose_VAT,
            _data.disclose_ident,
            _data.disclose_notify_email,
            _logd_request_id
        ).exec(
            _ctx,
            "UTC"
        );

        return ContactCreateResult(
            create_data.create_object_result.object_id,
            create_data.create_object_result.history_id,
            create_data.creation_time
        );

    } catch (const Fred::CreateContact::Exception& e) {

        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
        if(
            e.is_set_forbidden_company_name_setting() ||
            e.is_set_unknown_registrar_handle() ||
            e.is_set_unknown_ssntype()
        ) {
            throw;
        }

        if( e.is_set_invalid_contact_handle() /* wrong exception name */ ) {
            throw ObjectExists();
        }

        if( e.is_set_unknown_country() ) {
            AggregatedParamErrors exception;
            exception.add( Error( Param::contact_cc, 0, Reason::country_notexist ) );
            throw exception;
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
