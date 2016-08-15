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

void set_ContactUpdate_member(
    const boost::optional< Nullable< std::string > > &_input,
    Fred::UpdateContactByHandle &update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const std::string&))
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(_input)) {
        (update_object.*setter)(ContactChange::get_value(_input));
    }
    else if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(_input)) {
        const std::string empty_string;
        (update_object.*setter)(empty_string);
    }
}

void set_ContactUpdate_member(
    const boost::optional< Nullable< std::string > > &_input,
    Fred::UpdateContactByHandle &update_object,
    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const Nullable< std::string >&))
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(_input)) {
        (update_object.*setter)(ContactChange::get_value(_input));
    }
    else if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(_input)) {
        const Nullable< std::string > value_meaning_to_delete;
        (update_object.*setter)(value_meaning_to_delete);
    }
}

bool has_data_changed(const boost::optional< Nullable< std::string > > &change,
                      const std::string &current_value)
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
    {
        return ContactChange::get_value(change) != current_value;
    }
    if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(change))
    {
        const bool current_value_means_deleted_value = current_value.empty();
        return !current_value_means_deleted_value;
    }
    return false;
}

bool has_data_changed(const boost::optional< Nullable< std::string > > &change,
                      const Nullable< std::string > &current_value)
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
    {
        const std::string no_value;
        return ContactChange::get_value(change) != current_value.get_value_or(no_value);
    }
    if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(change))
    {
        const bool current_value_means_deleted_value = current_value.isnull() ||
                                                       current_value.get_value().empty();
        return !current_value_means_deleted_value;
    }
    return false;
}

bool has_data_changed(const boost::optional< Nullable< std::string > > &change,
                      const Optional< std::string > &current_value)
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
    {
        const std::string no_value;
        return ContactChange::get_value(change) != current_value.get_value_or(no_value);
    }
    if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(change))
    {
        const bool current_value_means_deleted_value = !current_value.isset() ||
                                                       current_value.get_value().empty();
        return !current_value_means_deleted_value;
    }
    return false;
}

bool has_data_changed(const boost::optional< std::string > &change,
                      const std::string &current_value)
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
    {
        return ContactChange::get_value(change) != current_value;
    }
    return false;
}

bool has_streets_changed(const std::vector< boost::optional< Nullable< std::string > > > &change,
                      const Fred::Contact::PlaceAddress &current_value)
{
    switch (change.size()) {
        case 0:
            return false;
        case 1:
            return has_data_changed(change[0], current_value.street1);
        case 2:
            return has_data_changed(change[0], current_value.street1) ||
                   has_data_changed(change[1], current_value.street2);
        case 3:
            return has_data_changed(change[0], current_value.street1) ||
                   has_data_changed(change[1], current_value.street2) ||
                   has_data_changed(change[2], current_value.street3);
    }
    throw std::runtime_error("Too many streets.");
}

bool has_data(const boost::optional< Nullable< std::string > > &change)
{
    return ContactChange::does_value_mean< ContactChange::Value::to_set >(change);
}

bool has_data(const boost::optional< std::string > &change)
{
    return ContactChange::does_value_mean< ContactChange::Value::to_set >(change);
}

bool has_streets(const std::vector< boost::optional< Nullable< std::string > > > &change)
{
    bool result = false;
    switch (change.size())
    {
        case 3:  result |= has_data(change[2]);
        case 2:  result |= has_data(change[1]);
        case 1:  result |= has_data(change[0]);
        case 0:  return result;
        default: throw std::runtime_error("Too many streets.");
    }
}

bool has_place_changed(const ContactChange &_changed_data,
                       const Nullable< Fred::Contact::PlaceAddress > &_current_place)
{
    if (_current_place.isnull()) {
        return has_data(_changed_data.city)              ||
               has_data(_changed_data.state_or_province) ||
               has_data(_changed_data.postal_code)       ||
               has_data(_changed_data.country_code)      ||
               has_streets(_changed_data.streets);
    }
    const Fred::Contact::PlaceAddress current_place = _current_place.get_value();
    return has_data_changed(_changed_data.city,              current_place.city)            ||
           has_data_changed(_changed_data.state_or_province, current_place.stateorprovince) ||
           has_data_changed(_changed_data.postal_code,       current_place.postalcode)      ||
           has_data_changed(_changed_data.country_code,      current_place.country)         ||
           has_streets_changed(_changed_data.streets,        current_place);
}

void set_data(const boost::optional< Nullable< std::string > > &change, std::string &data)
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change)) {
        data = ContactChange::get_value(change);
    }
}

void set_data(const boost::optional< std::string > &change, std::string &data)
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change)) {
        data = ContactChange::get_value(change);
    }
}

void set_data(const boost::optional< Nullable< std::string > > &change, Optional< std::string > &data)
{
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change)) {
        data = ContactChange::get_value(change);
    }
}

bool should_address_be_disclosed(
    Fred::OperationContext &_ctx,
    const Fred::InfoContactData &_current_contact_data,
    const ContactChange &_data)
{
    //don't touch organization => current value has to be checked
    if (ContactChange::does_value_mean< ContactChange::Value::not_to_touch >(_data.organization))
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
        const bool contact_will_be_organization =
            ContactChange::does_value_mean< ContactChange::Value::to_set >(_data.organization);
        if (contact_will_be_organization)
        {
            return true;
        }
    }

    const bool contact_will_lose_identification =
        has_data_changed(_data.email,        _current_contact_data.email)        ||
        has_data_changed(_data.telephone,    _current_contact_data.telephone)    ||
        has_data_changed(_data.name,         _current_contact_data.name)         ||
        has_data_changed(_data.organization, _current_contact_data.organization) ||
        has_place_changed(_data,             _current_contact_data.place);
    if (contact_will_lose_identification)
    {
        return true;
    }

    const Fred::ObjectStatesInfo contact_states(Fred::GetObjectStates(_current_contact_data.id).exec(_ctx));
    const bool contact_is_identified = contact_states.presents(Fred::Object_State::identified_contact) ||
                                       contact_states.presents(Fred::Object_State::validated_contact);
    return !contact_is_identified;
}

template < ContactDisclose::Item::Enum ITEM >
void set_ContactUpdate_discloseflag(Fred::UpdateContactByHandle &update_op, bool value);

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::name >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosename(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::organization >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseorganization(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::address >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseaddress(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::telephone >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosetelephone(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::fax >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosefax(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::email >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseemail(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::vat >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosevat(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::ident >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_discloseident(value);
}

template < >
void set_ContactUpdate_discloseflag< ContactDisclose::Item::notify_email >(Fred::UpdateContactByHandle &update_op, bool value)
{
    update_op.set_disclosenotifyemail(value);
}

template < ContactDisclose::Item::Enum ITEM >
void set_ContactUpdate_discloseflag(const ContactDisclose &_disclose, Fred::UpdateContactByHandle &update_op)
{
    const bool to_disclose = _disclose.should_be_disclosed< ITEM >(is_the_default_policy_to_disclose());
    set_ContactUpdate_discloseflag< ITEM >(update_op, to_disclose);
}

/**
 * Ensures ident and identtype are either both empty or both non-empty.
 */
struct Ident
{
    /**
     * @throws SsnTypeWithoutSsn
     * @throws SsnWithoutSsnType
     */
    Ident(const boost::optional< Nullable< std::string > > &_ident_value,
          const Nullable< ContactChange::IdentType::Enum > &_ident_type)
    :   ident_value_(_ident_value),
        ident_type_(_ident_type)
    {
        const bool ident_value_presents = ContactChange::does_value_mean< ContactChange::Value::to_set >(ident_value_);
        const bool ident_type_presents = !ident_type_.isnull();
        if (ident_value_presents != ident_type_presents) {
            if (ident_type_presents) {
                throw SsnTypeWithoutSsn();
            }
            throw SsnWithoutSsnType();
        }
    }

    bool presents()const
    {
        return ContactChange::does_value_mean< ContactChange::Value::to_set >(ident_value_);
    }

    Fred::PersonalIdUnion get()const
    {
        const std::string value = ContactChange::get_value(ident_value_);
        switch (ident_type_.get_value())
        {
            case ContactChange::IdentType::op:       return Fred::PersonalIdUnion::get_OP(value);
            case ContactChange::IdentType::pass:     return Fred::PersonalIdUnion::get_PASS(value);
            case ContactChange::IdentType::ico:      return Fred::PersonalIdUnion::get_ICO(value);
            case ContactChange::IdentType::mpsv:     return Fred::PersonalIdUnion::get_MPSV(value);
            case ContactChange::IdentType::birthday: return Fred::PersonalIdUnion::get_BIRTHDAY(value);
        }
        throw std::runtime_error("Invalid Epp::ContactChange::IdentType::Enum value.");
    }

    const boost::optional< Nullable< std::string > > ident_value_;
    const Nullable< ContactChange::IdentType::Enum > ident_type_;
};

}//namespace Epp::{anonymous}

unsigned long long contact_update_impl(
    Fred::OperationContext &_ctx,
    const std::string &_contact_handle,
    const ContactChange &_data,
    const unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    const bool contact_is_registered = Fred::Contact::get_handle_registrability(_ctx, _contact_handle) ==
                                       Fred::ContactHandleState::Registrability::registered;
    if (!contact_is_registered) {
        throw NonexistentHandle();
    }

    class InfoContactByHandle
    {
    public:
        InfoContactByHandle(const std::string &_handle):handle_(_handle) { }
        Fred::InfoContactData exec(Fred::OperationContext &_ctx)const
        {
            try {
                // TODO admin_contact_verification_modification AdminContactVerificationObjectStates::conditionally_cancel_final_states( ) relies on this exclusive lock
                return Fred::InfoContactByHandle(handle_).set_lock().exec(_ctx).info_contact_data;
            }
            catch (const Fred::InfoContactByHandle::Exception &e) {
                e.is_set_unknown_contact_handle() ? throw NonexistentHandle() : throw;
            }
        }
    private:
        const std::string handle_;
    };

    const Fred::InfoRegistrarData callers_registrar =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;
    const Fred::InfoContactData contact_data_before_update = InfoContactByHandle(_contact_handle).exec(_ctx);

    const bool is_sponsoring_registrar = (contact_data_before_update.sponsoring_registrar_handle ==
                                          callers_registrar.handle);
    const bool is_system_registrar = callers_registrar.system.get_value_or(false);
    const bool operation_is_permitted = (is_system_registrar || is_sponsoring_registrar);

    if (!operation_is_permitted) {
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
    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(_data.country_code)) {
        if (!is_country_code_valid(_ctx, ContactChange::get_value(_data.country_code))) {
            AggregatedParamErrors exception;
            exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
            throw exception;
        }
    }
    else {
        
    }

    // update itself
    {
        Fred::UpdateContactByHandle update(_contact_handle, callers_registrar.handle);

        set_ContactUpdate_member(_data.name,         update, &Fred::UpdateContactByHandle::set_name);
        set_ContactUpdate_member(_data.organization, update, &Fred::UpdateContactByHandle::set_organization);
        set_ContactUpdate_member(_data.telephone,    update, &Fred::UpdateContactByHandle::set_telephone);
        set_ContactUpdate_member(_data.fax,          update, &Fred::UpdateContactByHandle::set_fax);
        set_ContactUpdate_member(_data.email,        update, &Fred::UpdateContactByHandle::set_email);
        set_ContactUpdate_member(_data.notify_email, update, &Fred::UpdateContactByHandle::set_notifyemail);
        set_ContactUpdate_member(_data.vat,          update, &Fred::UpdateContactByHandle::set_vat);
        set_ContactUpdate_member(_data.auth_info_pw, update, &Fred::UpdateContactByHandle::set_authinfo);

        if (ContactChange::does_value_mean< ContactChange::Value::to_set >(_data.ident))
        {
            const Ident ident(_data.ident, _data.ident_type);
            const bool ident_has_to_be_updated = ident.presents();
            update.set_personal_id(ident_has_to_be_updated ? ident.get()                          //to update
                                                           : Nullable< Fred::PersonalIdUnion >());//to delete
        }

        if (_data.disclose.is_initialized()) {
            _data.disclose->check_validity();
            set_ContactUpdate_discloseflag< ContactDisclose::Item::name         >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::organization >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::address      >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::telephone    >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::fax          >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::email        >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::vat          >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::ident        >(*_data.disclose, update);
            set_ContactUpdate_discloseflag< ContactDisclose::Item::notify_email >(*_data.disclose, update);
        }
        else if (should_address_be_disclosed(_ctx, contact_data_before_update, _data)) {
            // don't set it otherwise it might already been set to true
            update.set_discloseaddress(true);
        }

        if (has_place_changed(_data, contact_data_before_update.place))
        {
            Fred::Contact::PlaceAddress new_place;
            switch (_data.streets.size())
            {
                case 3: set_data(_data.streets[2], new_place.street3);
                case 2: set_data(_data.streets[1], new_place.street2);
                case 1: set_data(_data.streets[0], new_place.street1);
                case 0: break;
                default: throw std::runtime_error("Too many streets.");
            }
            set_data(_data.city,              new_place.city);
            set_data(_data.state_or_province, new_place.stateorprovince);
            set_data(_data.postal_code,       new_place.postalcode);
            set_data(_data.country_code,      new_place.country);
            update.set_place(new_place);
        }

        if (_logd_request_id.isset()) {
            update.set_logd_request_id(_logd_request_id.get_value());
        }

        try {
            const unsigned long long new_history_id = update.exec(_ctx);

            const Fred::InfoContactData contact_data_after_update = Fred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data;
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
