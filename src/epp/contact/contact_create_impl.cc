#include "src/epp/contact/contact_create_impl.h"
#include "src/epp/disclose_policy.h"

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

namespace {

template < ContactDisclose::Enum ITEM >
bool compute_disclose_flag(const ContactCreateInputData &_data)
{
    return _data.compute_disclose_flag< ITEM >(is_the_default_policy_to_disclose());
}

}//namespace Epp::{anonymous}

ContactCreateResult contact_create_impl(
    Fred::OperationContext &_ctx,
    const std::string &_contact_handle,
    const ContactCreateInputData &_data,
    const unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id)
{
    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    const bool handle_is_valid = Fred::Contact::get_handle_syntax_validity(_contact_handle) ==
                                 Fred::ContactHandleState::SyntaxValidity::valid;
    if (!handle_is_valid) {
        throw InvalidHandle();
    }

    {
        const Fred::ContactHandleState::Registrability::Enum contact_registrability =
            Fred::Contact::get_handle_registrability(_ctx, _contact_handle);

        const bool contact_is_registered = contact_registrability ==
                                           Fred::ContactHandleState::Registrability::registered;
        if (contact_is_registered) {
            throw ObjectExists();
        }

        AggregatedParamErrors exception;

        const bool contact_is_in_protection_period = contact_registrability ==
                                                     Fred::ContactHandleState::Registrability::in_protection_period;
        if (contact_is_in_protection_period) {
            exception.add(Error::of_scalar_parameter(Param::contact_handle, Reason::protected_period));
        }

        if (!is_country_code_valid(_ctx, _data.country_code)) {
            exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
        }

        if (!exception.is_empty()) {
            throw exception;
        }
    }

    try {
        const Fred::CreateContact create_contact_op(
            _contact_handle,
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
                _data.country_code),
            _data.telephone,
            _data.fax,
            _data.email,
            _data.notify_email,
            _data.VAT,
            _data.identtype.isnull() ? Optional<std::string>() : to_db_handle(_data.identtype.get_value()),
            _data.ident,
            // will be implemented in #13744
            Optional< Fred::ContactAddressList >(),
            _data.compute_disclose_flag< ContactDisclose::name         >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::organization >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::address      >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::telephone    >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::fax          >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::email        >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::vat          >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::ident        >(is_the_default_policy_to_disclose()),
            _data.compute_disclose_flag< ContactDisclose::notify_email >(is_the_default_policy_to_disclose()),
            _logd_request_id);
        const Fred::CreateContact::Result create_data = create_contact_op.exec(_ctx, "UTC");

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
            exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
            throw exception;
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
