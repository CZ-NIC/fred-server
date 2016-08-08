#include "src/epp/disclose_policy.h"
#include "src/epp/contact/contact_update_impl.h"

#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

namespace Epp {

namespace {

void conditionall_set_ContactUpdate_member(
    const Optional< std::string > &_input,
    Fred::UpdateContactByHandle &update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const std::string&))
{
    if (_input.isset()) {
        (update_object.*setter)(_input.get_value());
    }
}

void conditionall_set_ContactUpdate_member(
    const Optional< std::string >& _input,
    Fred::UpdateContactByHandle &update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const Nullable< std::string >&))
{
    if (_input.isset()) {
        (update_object.*setter)(_input.get_value());
    }
}

inline bool has_data_changed(const Optional< std::string > &change, const std::string &current_value)
{
    return change.isset() && (change.get_value() != current_value);
}

bool has_data_changed(const Optional< std::string > &change, const Nullable< std::string > &current_value)
{
    return has_data_changed(change, current_value.get_value_or_default());
}

bool has_data_changed(const Optional< std::string > &change, const Optional< std::string > &current_value)
{
    return has_data_changed(change, current_value.get_value_or_default());
}

bool has_data_changed(const Optional< std::string > &change)
{
    return change.isset() && !change.get_value().empty();
}

bool has_place_changed(const ContactUpdateInputData &_changed_data,
                       const Nullable< Fred::Contact::PlaceAddress > &_current_place)
{
    if (_current_place.isnull()) {
        return has_data_changed(_changed_data.city) ||
               has_data_changed(_changed_data.state_or_province) ||
               has_data_changed(_changed_data.postal_code) ||
               has_data_changed(_changed_data.country_code) ||
               has_data_changed(_changed_data.street1) ||
               has_data_changed(_changed_data.street2) ||
               has_data_changed(_changed_data.street3);
    }
    const Fred::Contact::PlaceAddress current_place = _current_place.get_value();
    return has_data_changed(_changed_data.city,              current_place.city) ||
           has_data_changed(_changed_data.state_or_province, current_place.stateorprovince) ||
           has_data_changed(_changed_data.postal_code,       current_place.postalcode) ||
           has_data_changed(_changed_data.country_code,      current_place.country) ||
           has_data_changed(_changed_data.street1,           current_place.street1) ||
           has_data_changed(_changed_data.street2,           current_place.street2) ||
           has_data_changed(_changed_data.street3,           current_place.street3);
}

bool should_address_be_disclosed(
    Fred::OperationContext& _ctx,
    const unsigned long long _contact_id,
    const Fred::InfoContactData& _current_contact_data,
    const ContactUpdateInputData& _data)
{
    //don't touch organization => current value has to be checked
    if (!_data.organization.isset())
    {
        const bool contact_is_organization = !_current_contact_data.organization.isnull() &&
                                             !_current_contact_data.organization.get_value().empty();
        if (contact_is_organization)
        {
            return true;
        }
    }
    //change organization => new value has to be checked
    else {
        const bool contact_will_be_organization = !_data.organization.get_value().empty();
        if (contact_will_be_organization)
        {
            return true;
        }
    }

    if (has_data_changed(_data.email,        _current_contact_data.email) ||
        has_data_changed(_data.telephone,    _current_contact_data.telephone) ||
        has_data_changed(_data.name,         _current_contact_data.name) ||
        has_data_changed(_data.organization, _current_contact_data.organization) ||
        has_place_changed(_data,             _current_contact_data.place))
    {
        return true;
    }

    const Fred::ObjectStatesInfo contact_states(Fred::GetObjectStates(_contact_id).exec(_ctx));
    return !contact_states.presents(Fred::Object_State::identified_contact) &&
           !contact_states.presents(Fred::Object_State::validated_contact);
}

template < ContactDisclose::Enum ITEM >
void set_ContactUpdate_disclose_flag(Fred::UpdateContactByHandle &update_op, bool value);

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::name >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosename(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::organization >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseorganization(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::address >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseaddress(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::telephone >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosetelephone(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::fax >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosefax(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::email >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseemail(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::vat >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosevat(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::ident >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseident(value);
}

template < >
void set_ContactUpdate_disclose_flag< ContactDisclose::notify_email >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosenotifyemail(value);
}

template < ContactDisclose::Enum ITEM >
void set_ContactUpdate_disclose_flag(
    const ContactUpdateInputData &_data,
    bool _default_policy_is_to_disclose,
    Fred::UpdateContactByHandle &update_op)
{
    const bool item_has_to_be_hidden    = _data.to_hide.find(ITEM) != _data.to_hide.end();
    const bool item_has_to_be_disclosed = _data.to_disclose.find(ITEM) != _data.to_disclose.end();
    if (item_has_to_be_hidden && !item_has_to_be_disclosed) {
        set_ContactUpdate_disclose_flag< ITEM >(update_op, false);
        return;
    }
    if (!item_has_to_be_hidden && item_has_to_be_disclosed) {
        set_ContactUpdate_disclose_flag< ITEM >(update_op, true);
        return;
    }
    if (!item_has_to_be_hidden && !item_has_to_be_disclosed) {
        set_ContactUpdate_disclose_flag< ITEM >(update_op, _default_policy_is_to_disclose);
        return;
    }
    throw std::runtime_error("Ambiguous disclose flag");
}

}//namespace Epp::{anonymous}

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
    const Optional<unsigned long long>& _logd_request_id
) {

    if( _registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Contact::get_handle_registrability(_ctx, _data.handle) != Fred::ContactHandleState::Registrability::registered ) {
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

    const Fred::InfoRegistrarData callers_registrar =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;
    const Fred::InfoContactData contact_data_before_update = translate_info_contact_exception::exec(_ctx, _data.handle);

    const bool is_sponsoring_registrar = (contact_data_before_update.sponsoring_registrar_handle ==
                                          callers_registrar.handle);
    const bool is_system_registrar = callers_registrar.system.get_value_or(false);
    const bool is_operation_permitted = (is_system_registrar || is_sponsoring_registrar);

    if (!is_operation_permitted) {
        throw AuthorizationError();
    }

    // do it before any object state related checks
    Fred::LockObjectStateRequestLock(contact_data_before_update.id).exec(_ctx);
    Fred::PerformObjectStateRequest(contact_data_before_update.id).exec(_ctx);

    if (!is_system_registrar) {
        const Fred::ObjectStatesInfo contact_states(Fred::GetObjectStates(contact_data_before_update.id).exec(_ctx));
        if (contact_states.presents(Fred::Object_State::server_update_prohibited) ||
            contact_states.presents(Fred::Object_State::delete_candidate))
        {
            throw ObjectStatusProhibitsOperation();
        }
    }

    // when deleting or not-changing, no check of data is needed
    if( _data.country_code.isset() && !_data.country_code.get_value().empty() ) {
        if ( !is_country_code_valid(_ctx, _data.country_code.get_value() ) ) {
            AggregatedParamErrors exception;
            exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
            throw exception;
        }
    }

    // update itself
    {
        Fred::UpdateContactByHandle update(_data.handle, callers_registrar.handle);

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

        if (_data.should_discloseflags_be_changed()) {
            const bool policy = is_the_default_policy_to_disclose();
            set_ContactUpdate_disclose_flag< ContactDisclose::name         >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::organization >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::address      >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::telephone    >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::fax          >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::email        >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::vat          >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::ident        >(_data, policy, update);
            set_ContactUpdate_disclose_flag< ContactDisclose::notify_email >(_data, policy, update);
        }
        else if (should_address_be_disclosed(_ctx, contact_data_before_update.id, contact_data_before_update, _data)) {
            // don't set it otherwise it might already been set to true
            update.set_discloseaddress(true);
        }

        if (_data.street1.isset() || _data.street2.isset() || _data.street3.isset() ||
            _data.city.isset() ||
            _data.state_or_province.isset() ||
            _data.postal_code.isset() ||
            _data.country_code.isset())
        {
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

        if (_logd_request_id.isset()) {
            update.set_logd_request_id(_logd_request_id.get_value());
        }

        try {
            const unsigned long long new_history_id = update.exec(_ctx);

            const Fred::InfoContactData contact_data_after_update = Fred::InfoContactByHandle(_data.handle).exec(_ctx).info_contact_data;
            //check disclose address
            if (!contact_data_after_update.discloseaddress)
            {
                //discloseaddress conditions #7493
                const Fred::ObjectStatesInfo contact_states(Fred::GetObjectStates(contact_data_before_update.id).exec(_ctx));
                const bool hidden_address_allowed_by_contact_state =
                    contact_states.presents(Fred::Object_State::identified_contact) ||
                    contact_states.presents(Fred::Object_State::validated_contact);
                const bool contact_is_organization = !contact_data_after_update.organization.isnull() &&
                                                     !contact_data_after_update.organization.get_value().empty();
                const bool hidden_address_allowed = !contact_is_organization &&
                                                    hidden_address_allowed_by_contact_state;

                if (!hidden_address_allowed)
                {
                    throw ObjectStatusProhibitsOperation();
                }
            }

            return new_history_id;

        }
        catch(const Fred::UpdateContactByHandle::ExceptionType &e) {

            /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
            if (e.is_set_forbidden_company_name_setting() ||
                e.is_set_unknown_registrar_handle() ||
                e.is_set_unknown_ssntype())
            {
                throw;
            }

            if (e.is_set_unknown_contact_handle())
            {
                throw NonexistentHandle();
            }

            if (e.is_set_unknown_country())
            {
                AggregatedParamErrors exception;
                exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
                throw exception;
            }

            /* in the improbable case that exception is incorrectly set */
            throw;
        }
    }
}

}
