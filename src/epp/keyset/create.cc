#include "src/epp/keyset/create.h"
#include "src/epp/keyset/limits.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"

#include "src/fredlib/registrar/info_registrar.h"

#include "src/fredlib/keyset/create_keyset.h"

namespace Epp {

ContactCreateResult keyset_create(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const std::string &_auth_info_pw,
    const KeysetInfoData::TechContacts &_tech_contacts,
    const KeysetInfoData::DsRecords &_ds_records,
    const KeysetInfoData::DnsKeys &_dns_keys,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        throw AuthErrorServerClosingConnection();
    }

    {
        AggregatedParamErrors aggregated_param_errors;

        switch (Fred::KeySet::get_handle_syntax_validity(_keyset_handle))
        {
            case Fred::KeySet::HandleState::valid:
                switch (Fred::Keyset::get_handle_registrability(_ctx, _keyset_handle))
                {
                    case Fred::KeySet::HandleState::registered:
                        aggregated_param_errors.add(scalar_parameter_failure(Param::keyset_handle,
                                                                             Reason::existing));
                        break;
                    case Fred::KeySet::HandleState::in_protection_period:
                        aggregated_param_errors.add(scalar_parameter_failure(Param::keyset_handle,
                                                                             Reason::protected_period));
                        break;
                }
                break;
            case Fred::KeySet::HandleState::invalid:
                aggregated_param_errors.add(scalar_parameter_failure(Param::keyset_handle,
                                                                     Reason::bad_format_keyset_handle));
                break;
        }

        if (_tech_contacts.size() < KeySet::min_number_of_tech_contacts) {
                aggregated_param_errors.add(scalar_parameter_failure(Param::keyset_tech,
                                                                     Reason::tech_notexist));
        }
        if (KeySet::max_number_of_tech_contacts < _tech_contacts.size()) {
                aggregated_param_errors.add(scalar_parameter_failure(Param::keyset_tech,
                                                                     Reason::techadmin_limit));
        }

        if (!aggregated_param_errors.is_empty()) {
            throw aggregated_param_errors;
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
