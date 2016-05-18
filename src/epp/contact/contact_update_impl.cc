#include "src/epp/contact/contact_update_impl.h"

#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"

namespace Epp {

static bool has_data_changed(const Optional<std::string>& change, const Nullable<std::string>& current_value) {
    // no change
    if( !change.isset() ) {
        return false;
    } else {
        return change.get_value() != current_value.get_value_or_default();
    }
}

static bool has_data_changed(const Optional<std::string>& change, const Optional<std::string>& current_value) {
    // no change
    if( !change.isset() ) {
        return false;
    } else {
        return change.get_value() != current_value.get_value_or_default();
    }
}

static void conditionall_set_ContactUpdate_member(
    const Optional<std::string>& _input,
    Fred::UpdateContactByHandle& update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const std::string&)
) {
    if(_input.isset()) {
        (update_object.*setter)(_input.get_value());
    }
}

static void conditionall_set_ContactUpdate_member(
    const Optional<std::string>& _input,
    Fred::UpdateContactByHandle& update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const Nullable<std::string>&)
) {
    if(_input.isset()) {
        (update_object.*setter)( Nullable<std::string>(_input.get_value()) );
    }
}

static void conditionall_set_ContactUpdate_member(
    const Optional<bool>& _input,
    Fred::UpdateContactByHandle& update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(bool)
) {
    if(_input.isset()) {
        (update_object.*setter)(_input.get_value());
    }
}

static bool should_address_be_disclosed(
    Fred::OperationContext& _ctx,
    unsigned long long _contact_id,
    const Fred::InfoContactData& _contact_data_before_update,
    const ContactUpdateInputData& _data
) {
     //discloseaddress conditions #12563
     //discloseaddress not changed
     if( !_data.disclose_address.isset() ) {

         // no change so we need to check current value
         if( !_data.organization.isset() ) {
             if( !_contact_data_before_update.organization.isnull() && !_contact_data_before_update.organization.get_value().empty() ) {
                 return true;
             }

         // changing organization to non-empty value
         } else if(
             _data.organization.isset()
             &&
             !_data.organization.get_value().empty()
         ) {
             return true;
         }

         if(
                has_data_changed(_data.email,               _contact_data_before_update.email)
             || has_data_changed(_data.telephone,           _contact_data_before_update.telephone)
             || has_data_changed(_data.name,                _contact_data_before_update.name)
             || has_data_changed(_data.organization,        _contact_data_before_update.organization)
             || has_data_changed(_data.city,                _contact_data_before_update.place.isnull() ? Optional<std::string>() : _contact_data_before_update.place.get_value().city)
             || has_data_changed(_data.state_or_province,   _contact_data_before_update.place.isnull() ? Optional<std::string>() : _contact_data_before_update.place.get_value().stateorprovince)
             || has_data_changed(_data.postal_code,         _contact_data_before_update.place.isnull() ? Optional<std::string>() : _contact_data_before_update.place.get_value().postalcode)
             || has_data_changed(_data.country_code,        _contact_data_before_update.place.isnull() ? Optional<std::string>() : _contact_data_before_update.place.get_value().country)
             || has_data_changed(_data.street1,             _contact_data_before_update.place.isnull() ? Optional<std::string>() : _contact_data_before_update.place.get_value().street1)
             || has_data_changed(_data.street2,             _contact_data_before_update.place.isnull() ? Optional<std::string>() : _contact_data_before_update.place.get_value().street2)
             || has_data_changed(_data.street3,             _contact_data_before_update.place.isnull() ? Optional<std::string>() : _contact_data_before_update.place.get_value().street3)
         ) {
             return true;
         }

         if(
             !(
                 Fred::ObjectHasState(_contact_id, Fred::ObjectState::IDENTIFIED_CONTACT).exec(_ctx)
                 ||
                 Fred::ObjectHasState(_contact_id, Fred::ObjectState::VALIDATED_CONTACT).exec(_ctx)
             )
         ) {
             return true;
         }
     }

     return false;
}


/**
 * Ensures ident and identtype are either both empty or both non-empty.
 */
struct Ident {
    const std::string ident_;
    const Nullable<IdentType::Enum> identtype_;

    /**
     * @throws SsnTypeWithoutSsn
     * @throws SsnWithoutSsnType
     */
    Ident(
        const std::string& _ident,
        const Nullable<IdentType::Enum>& _identtype
    ) :
        ident_(_ident),
        identtype_(_identtype)
    {
        if( _ident.empty() && !_identtype.isnull() ) {
            throw SsnTypeWithoutSsn();
        }

        if( !_ident.empty() && _identtype.isnull() ) {
            throw SsnWithoutSsnType();
        }
    }

    bool is_null() const {
        return ident_.empty();
    }
};

unsigned long long contact_update_impl(
    Fred::OperationContext& _ctx,
    const ContactUpdateInputData& _data,
    const unsigned long long _registrar_id,
    const unsigned long long _logd_request_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Contact::is_handle_valid(_data.handle) != Fred::ContactHandleState::SyntaxValidity::valid ) {
        throw InvalidHandle();

    } else if( Fred::Contact::is_handle_in_registry(_ctx, _data.handle) != Fred::ContactHandleState::InRegistry::registered ) {
        throw NonexistentHandle();
    }

    struct translate_info_contact_exception {
        static Fred::InfoContactData exec(Fred::OperationContext& _ctx, const std::string _handle) {
            try {
                // TODO admin_contact_verification_modification AdminContactVerificationObjectStates::conditionally_cancel_final_states( ) relies on this exclusive lock
                return Fred::InfoContactByHandle(_handle).set_lock().exec(_ctx).info_contact_data;
            } catch(const Fred::InfoContactByHandle::Exception& e) {
                if( e.is_set_unknown_contact_handle() ) {
                    throw NonexistentHandle();
                }
                throw;
            }
        }
    };

    const Fred::InfoContactData contact_data_before_update = translate_info_contact_exception::exec(_ctx, _data.handle);

    const Fred::InfoRegistrarData sponsoring_registrar_before_update =
        Fred::InfoRegistrarByHandle(contact_data_before_update.sponsoring_registrar_handle)
            .set_lock(/* TODO az to bude mozne, staci lock registrar for share */ )
            .exec(_ctx)
            .info_registrar_data;

    if( sponsoring_registrar_before_update.id != _registrar_id ) {
        throw AutorError();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(contact_data_before_update.id).exec(_ctx);
    Fred::PerformObjectStateRequest(contact_data_before_update.id).exec(_ctx);

    if( Fred::ObjectHasState(contact_data_before_update.id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(_ctx)
        ||
        Fred::ObjectHasState(contact_data_before_update.id, Fred::ObjectState::DELETE_CANDIDATE).exec(_ctx)
    ) {
        throw ObjectStatusProhibitingOperation();
    }

    // when deleting or not-changing, no check of data is needed
    if( _data.country_code.isset() && !_data.country_code.get_value().empty() ) {
        if ( !is_country_code_valid(_ctx, _data.country_code.get_value() ) ) {
            AggregatedParamErrors exception;
            exception.add( Error( Param::contact_cc, 0, Reason::country_notexist ) );
            throw exception;
        }
    }

    // update itself
    {
        Fred::UpdateContactByHandle update(_data.handle, sponsoring_registrar_before_update.handle);

        conditionall_set_ContactUpdate_member(_data.name,          update, &Fred::UpdateContactByHandle::set_name);
        conditionall_set_ContactUpdate_member(_data.organization,  update, &Fred::UpdateContactByHandle::set_organization);
        conditionall_set_ContactUpdate_member(_data.telephone,     update, &Fred::UpdateContactByHandle::set_telephone);
        conditionall_set_ContactUpdate_member(_data.fax,           update, &Fred::UpdateContactByHandle::set_fax);
        conditionall_set_ContactUpdate_member(_data.email,         update, &Fred::UpdateContactByHandle::set_email);
        conditionall_set_ContactUpdate_member(_data.notify_email,  update, &Fred::UpdateContactByHandle::set_notifyemail);
        conditionall_set_ContactUpdate_member(_data.VAT,           update, &Fred::UpdateContactByHandle::set_vat);
        conditionall_set_ContactUpdate_member(_data.authinfo,      update, &Fred::UpdateContactByHandle::set_authinfo);

        if( _data.ident.isset() ) {

            const Epp::Ident ident_change(_data.ident.get_value(), _data.identtype);

            // delete
            if( ident_change.is_null() ) {
                update.set_personal_id( Nullable<Fred::PersonalIdUnion>() );

            // value update
            } else {
                update.set_personal_id(
                    Fred::PersonalIdUnion::get_any_type(
                        to_db_handle( ident_change.identtype_.get_value() ),
                        ident_change.ident_
                    )
                );
            }
        }

        conditionall_set_ContactUpdate_member(_data.disclose_name,         update, &Fred::UpdateContactByHandle::set_disclosename);
        conditionall_set_ContactUpdate_member(_data.disclose_organization, update, &Fred::UpdateContactByHandle::set_discloseorganization);
        conditionall_set_ContactUpdate_member(_data.disclose_telephone,    update, &Fred::UpdateContactByHandle::set_disclosetelephone);
        conditionall_set_ContactUpdate_member(_data.disclose_fax,          update, &Fred::UpdateContactByHandle::set_disclosefax);
        conditionall_set_ContactUpdate_member(_data.disclose_email,        update, &Fred::UpdateContactByHandle::set_discloseemail);
        conditionall_set_ContactUpdate_member(_data.disclose_VAT,          update, &Fred::UpdateContactByHandle::set_disclosevat);
        conditionall_set_ContactUpdate_member(_data.disclose_ident,        update, &Fred::UpdateContactByHandle::set_discloseident);
        conditionall_set_ContactUpdate_member(_data.disclose_notify_email, update, &Fred::UpdateContactByHandle::set_disclosenotifyemail);
        conditionall_set_ContactUpdate_member(_data.disclose_address,      update, &Fred::UpdateContactByHandle::set_discloseaddress);

        if( should_address_be_disclosed(_ctx, contact_data_before_update.id, contact_data_before_update, _data) ) {
            // don't set it otherwise it might already been set to true
            update.set_discloseaddress(true);
        }

        if( _data.street1.isset() || _data.street2.isset() || _data.street3.isset()
            || _data.city.isset()
            || _data.state_or_province.isset()
            || _data.postal_code.isset()
            || _data.country_code.isset()
        ) {
            update.set_place(
                Fred::Contact::PlaceAddress(
                    _data.street1.isset()           ? _data.street1.get_value()             : "",
                    _data.street2.isset()           ? _data.street2.get_value()             : "",
                    _data.street3.isset()           ? _data.street3.get_value()             : "",
                    _data.city.isset()              ? _data.city.get_value()                : "",
                    _data.state_or_province.isset() ? _data.state_or_province.get_value()   : "",
                    _data.postal_code.isset()       ? _data.postal_code.get_value()         : "",
                    _data.country_code.isset()      ? _data.country_code.get_value()        : ""
                )
            );
        }

        try {
            const unsigned long long new_history_id = update.exec(_ctx);

            //check disclose address
            {
                //discloseaddress conditions #7493
                const bool hidden_address_allowed_by_contact_state =
                    Fred::ObjectHasState(contact_data_before_update.id, Fred::ObjectState::IDENTIFIED_CONTACT).exec(_ctx)
                    ||
                    Fred::ObjectHasState(contact_data_before_update.id, Fred::ObjectState::VALIDATED_CONTACT).exec(_ctx);

                const Fred::InfoContactData contact_data_after_update = Fred::InfoContactByHandle(_data.handle).exec(_ctx).info_contact_data;

                if(
                    (contact_data_after_update.discloseaddress == false)
                    &&
                    (   ( ! contact_data_after_update.organization.isnull() && ! contact_data_after_update.organization.get_value().empty() )
                        ||
                        ! hidden_address_allowed_by_contact_state
                    )
                ) {
                    throw ObjectStatusProhibitingOperation();
                }
            }

            return new_history_id;

        } catch(const Fred::UpdateContactByHandle::ExceptionType& e) {

            /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
            if(
                e.is_set_forbidden_company_name_setting()
                || e.is_set_unknown_registrar_handle()
                || e.is_set_unknown_sponsoring_registrar_handle()
                || e.is_set_unknown_ssntype()
            ) {
                throw;
            }

            if( e.is_set_unknown_contact_handle() ) {
                throw NonexistentHandle();
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

}
