#include "src/epp/disclose_policy.h"
#include "src/epp/domain/domain_update_impl.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/param.h"
#include "src/epp/reason.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/keyset/handle_state.h"
#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/nsset/handle_state.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <boost/mpl/assert.hpp>

namespace Epp {

namespace Domain {

//namespace {
//
//void set_ContactUpdate_member(
//    const boost::optional< Nullable< std::string > > &_input,
//    Fred::UpdateContactByHandle &update_object,
//    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const std::string&))
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(_input)) {
//        (update_object.*setter)(ContactChange::get_value(_input));
//    }
//    else if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(_input)) {
//        const std::string empty_string;
//        (update_object.*setter)(empty_string);
//    }
//}
//
//void set_ContactUpdate_member(
//    const boost::optional< Nullable< std::string > > &_input,
//    Fred::UpdateContactByHandle &update_object,
//    Fred::UpdateContactByHandle& (Fred::UpdateContactByHandle::*setter)(const Nullable< std::string >&))
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(_input)) {
//        (update_object.*setter)(ContactChange::get_value(_input));
//    }
//    else if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(_input)) {
//        const Nullable< std::string > value_meaning_to_delete;
//        (update_object.*setter)(value_meaning_to_delete);
//    }
//}
//
//bool has_data_changed(const boost::optional< Nullable< std::string > > &change,
//                      const std::string &current_value)
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
//    {
//        return ContactChange::get_value(change) != current_value;
//    }
//    if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(change))
//    {
//        const bool current_value_means_deleted_value = current_value.empty();
//        return !current_value_means_deleted_value;
//    }
//    return false;
//}
//
//bool has_data_changed(const boost::optional< Nullable< std::string > > &change,
//                      const Nullable< std::string > &current_value)
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
//    {
//        const std::string no_value;
//        return ContactChange::get_value(change) != current_value.get_value_or(no_value);
//    }
//    if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(change))
//    {
//        const bool current_value_means_deleted_value = current_value.isnull() ||
//                                                       current_value.get_value().empty();
//        return !current_value_means_deleted_value;
//    }
//    return false;
//}
//
//bool has_data_changed(const boost::optional< Nullable< std::string > > &change,
//                      const Optional< std::string > &current_value)
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
//    {
//        const std::string no_value;
//        return ContactChange::get_value(change) != current_value.get_value_or(no_value);
//    }
//    if (ContactChange::does_value_mean< ContactChange::Value::to_delete >(change))
//    {
//        const bool current_value_means_deleted_value = !current_value.isset() ||
//                                                       current_value.get_value().empty();
//        return !current_value_means_deleted_value;
//    }
//    return false;
//}
//
//bool has_data_changed(const boost::optional< std::string > &change,
//                      const std::string &current_value)
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change))
//    {
//        return ContactChange::get_value(change) != current_value;
//    }
//    return false;
//}
//
//bool has_streets_changed(const std::vector< boost::optional< Nullable< std::string > > > &change,
//                      const Fred::Contact::PlaceAddress &current_value)
//{
//    switch (change.size()) {
//        case 0:
//            return false;
//        case 1:
//            return has_data_changed(change[0], current_value.street1);
//        case 2:
//            return has_data_changed(change[0], current_value.street1) ||
//                   has_data_changed(change[1], current_value.street2);
//        case 3:
//            return has_data_changed(change[0], current_value.street1) ||
//                   has_data_changed(change[1], current_value.street2) ||
//                   has_data_changed(change[2], current_value.street3);
//    }
//    throw std::runtime_error("Too many streets.");
//}
//
//bool has_data(const boost::optional< Nullable< std::string > > &change)
//{
//    return ContactChange::does_value_mean< ContactChange::Value::to_set >(change);
//}
//
//bool has_data(const boost::optional< std::string > &change)
//{
//    return ContactChange::does_value_mean< ContactChange::Value::to_set >(change);
//}
//
//bool has_streets(const std::vector< boost::optional< Nullable< std::string > > > &change)
//{
//    bool result = false;
//    switch (change.size())
//    {
//        case 3:  result |= has_data(change[2]);
//        case 2:  result |= has_data(change[1]);
//        case 1:  result |= has_data(change[0]);
//        case 0:  return result;
//        default: throw std::runtime_error("Too many streets.");
//    }
//}
//
//bool has_place_changed(const ContactChange &_changed_data,
//                       const Nullable< Fred::Contact::PlaceAddress > &_current_place)
//{
//    if (_current_place.isnull()) {
//        return has_data(_changed_data.city)              ||
//               has_data(_changed_data.state_or_province) ||
//               has_data(_changed_data.postal_code)       ||
//               has_data(_changed_data.country_code)      ||
//               has_streets(_changed_data.streets);
//    }
//    const Fred::Contact::PlaceAddress current_place = _current_place.get_value();
//    return has_data_changed(_changed_data.city,              current_place.city)            ||
//           has_data_changed(_changed_data.state_or_province, current_place.stateorprovince) ||
//           has_data_changed(_changed_data.postal_code,       current_place.postalcode)      ||
//           has_data_changed(_changed_data.country_code,      current_place.country)         ||
//           has_streets_changed(_changed_data.streets,        current_place);
//}
//
//void set_data(const boost::optional< Nullable< std::string > > &change, std::string &data)
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change)) {
//        data = ContactChange::get_value(change);
//    }
//}
//
//void set_data(const boost::optional< std::string > &change, std::string &data)
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change)) {
//        data = ContactChange::get_value(change);
//    }
//}
//
//void set_data(const boost::optional< Nullable< std::string > > &change, Optional< std::string > &data)
//{
//    if (ContactChange::does_value_mean< ContactChange::Value::to_set >(change)) {
//        data = ContactChange::get_value(change);
//    }
//}
//
//bool should_address_be_disclosed(
//    Fred::OperationContext &_ctx,
//    const Fred::InfoContactData &_current_contact_data,
//    const ContactChange &_change)
//{
//    //don't touch organization => current value has to be checked
//    if (ContactChange::does_value_mean< ContactChange::Value::not_to_touch >(_change.organization))
//    {
//        const bool contact_is_organization = !_current_contact_data.organization.isnull() &&
//                                             !_current_contact_data.organization.get_value().empty();
//        if (contact_is_organization)
//        {
//            return true;
//        }
//    }
//    //change organization => new value has to be checked
//    else {
//        const bool contact_will_be_organization =
//            ContactChange::does_value_mean< ContactChange::Value::to_set >(_change.organization);
//        if (contact_will_be_organization)
//        {
//            return true;
//        }
//    }
//
//    const bool contact_will_lose_identification =
//        has_data_changed(_change.email,        _current_contact_data.email)        ||
//        has_data_changed(_change.telephone,    _current_contact_data.telephone)    ||
//        has_data_changed(_change.name,         _current_contact_data.name)         ||
//        has_data_changed(_change.organization, _current_contact_data.organization) ||
//        has_place_changed(_change,             _current_contact_data.place);
//    if (contact_will_lose_identification)
//    {
//        return true;
//    }
//
//    const Fred::ObjectStatesInfo contact_states(Fred::GetObjectStates(_current_contact_data.id).exec(_ctx));
//    const bool contact_is_identified = contact_states.presents(Fred::Object_State::identified_contact) ||
//                                       contact_states.presents(Fred::Object_State::validated_contact);
//    return !contact_is_identified;
//}
//
//template < ContactDisclose::Item::Enum ITEM >
//void set_ContactUpdate_discloseflag(Fred::UpdateContactByHandle &update_op, bool value);
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::name >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_disclosename(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::organization >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_discloseorganization(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::address >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_discloseaddress(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::telephone >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_disclosetelephone(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::fax >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_disclosefax(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::email >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_discloseemail(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::vat >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_disclosevat(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::ident >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_discloseident(value);
//}
//
//template < >
//void set_ContactUpdate_discloseflag< ContactDisclose::Item::notify_email >(Fred::UpdateContactByHandle &update_op, bool value)
//{
//    update_op.set_disclosenotifyemail(value);
//}
//
//template < ContactDisclose::Item::Enum ITEM >
//void set_ContactUpdate_discloseflag(const ContactDisclose &_disclose, Fred::UpdateContactByHandle &update_op)
//{
//    BOOST_MPL_ASSERT_MSG(ITEM != ContactDisclose::Item::address,
//                         discloseflag_address_has_its_own_method,
//                         (ContactDisclose::Item::Enum));
//    const bool to_disclose = _disclose.should_be_disclosed< ITEM >(is_the_default_policy_to_disclose());
//    set_ContactUpdate_discloseflag< ITEM >(update_op, to_disclose);
//}
//
//void set_ContactUpdate_discloseflag_address(
//    Fred::OperationContext &_ctx,
//    const ContactChange &_change,
//    const Fred::InfoContactData &_contact_data_before_update,
//    Fred::UpdateContactByHandle &update_op)
//{
//    bool address_has_to_be_disclosed =
//        _change.disclose->should_be_disclosed< ContactDisclose::Item::address >(is_the_default_policy_to_disclose());
//
//    if (!address_has_to_be_disclosed) {
//        static const bool address_has_to_be_hidden = true;
//        const bool address_can_be_hidden = !should_address_be_disclosed(_ctx,
//                                                                        _contact_data_before_update,
//                                                                        _change);
//        if (address_has_to_be_hidden && !address_can_be_hidden) {
//            throw ObjectStatusProhibitsOperation();
//        }
//        address_has_to_be_disclosed = !address_has_to_be_hidden || !address_can_be_hidden;
//    }
//
//    update_op.set_discloseaddress(address_has_to_be_disclosed);
//}
//
//Fred::InfoContactData info_contact_by_handle(const std::string &handle, Fred::OperationContext &ctx)
//{
//    try {
//        // TODO admin_contact_verification_modification AdminContactVerificationObjectStates::conditionally_cancel_final_states( ) relies on this exclusive lock
//        return Fred::InfoContactByHandle(handle).set_lock().exec(ctx).info_contact_data;
//    }
//    catch (const Fred::InfoContactByHandle::Exception &e) {
//        e.is_set_unknown_contact_handle() ? throw NonexistentHandle() : throw;
//    }
//}
//
///**
// * Ensures ident and identtype are either both empty or both non-empty.
// */
//struct Ident
//{
//    /**
//     * @throws SsnTypeWithoutSsn
//     * @throws SsnWithoutSsnType
//     */
//    Ident(const boost::optional< Nullable< std::string > > &_ident_value,
//          const Nullable< ContactChange::IdentType::Enum > &_ident_type)
//    :   ident_value_(_ident_value),
//        ident_type_(_ident_type)
//    {
//        const bool ident_value_and_ident_type_have_to_be_deleted =
//            ContactChange::does_value_mean< ContactChange::Value::to_delete >(ident_value_);
//        if (ident_value_and_ident_type_have_to_be_deleted) {
//            return;
//        }
//
//        const bool ident_type_has_to_be_changed = !ident_type_.isnull();
//        const bool ident_value_has_to_be_changed =
//            ContactChange::does_value_mean< ContactChange::Value::to_set >(ident_value_);
//        if (ident_type_has_to_be_changed != ident_value_has_to_be_changed) {
//            ident_type_has_to_be_changed ? throw SsnTypeWithoutSsn()
//                                         : throw SsnWithoutSsnType();
//        }
//    }
//
//    template < ContactChange::Value::Meaning MEANING >
//    bool does_value_mean()const
//    {
//        return ContactChange::does_value_mean< MEANING >(ident_value_);
//    }
//
//    Fred::PersonalIdUnion get()const
//    {
//        const std::string value = ContactChange::get_value(ident_value_);
//        switch (ident_type_.get_value())
//        {
//            case ContactChange::IdentType::op:       return Fred::PersonalIdUnion::get_OP(value);
//            case ContactChange::IdentType::pass:     return Fred::PersonalIdUnion::get_PASS(value);
//            case ContactChange::IdentType::ico:      return Fred::PersonalIdUnion::get_ICO(value);
//            case ContactChange::IdentType::mpsv:     return Fred::PersonalIdUnion::get_MPSV(value);
//            case ContactChange::IdentType::birthday: return Fred::PersonalIdUnion::get_BIRTHDAY(value);
//        }
//        throw std::runtime_error("Invalid Epp::ContactChange::IdentType::Enum value.");
//    }
//
//    const boost::optional< Nullable< std::string > > ident_value_;
//    const Nullable< ContactChange::IdentType::Enum > ident_type_;
//};
//
//} // namespace Epp::{anonymous}

unsigned long long domain_update_impl(
    Fred::OperationContext &_ctx,
    const std::string& _domain_fqdn,
    const std::string& _registrant_change,
    const Optional<std::string>& _auth_info_pw_change,
    const std::string& _nsset_change,
    const std::string& _keyset_change,
    const std::vector<std::string>& _admin_contacts_add,
    const std::vector<std::string>& _admin_contacts_rem,
    const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    //TODO JZ
    //boost::gregorian::date current_local_date = boost::posix_time::microsec_clock::local_time().date();

    //const Fred::Zone::Data zone_data = Fred::Zone::find_zone_in_fqdn(_ctx,
    //        Fred::Zone::rem_trailing_dot(_data.fqdn));

    //if (!Fred::registrar_zone_access(_registrar_id, zone_data.id, current_local_date, _ctx)) {
    //    throw AuthorizationError();
    //}


    // TODO enum domain

    try {
        if (Fred::Domain::get_domain_registrability_by_domain_fqdn(_ctx, _domain_fqdn) != Fred::Domain::DomainRegistrability::registered) {
            throw NonexistentHandle();
        }
    }
    catch (const Fred::Domain::ExceptionInvalidFqdn&) {
        throw NonexistentHandle();
    }
    catch (const NonexistentHandle&) {
        throw;
    }

    const Fred::InfoRegistrarData session_registrar =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;
    const Fred::InfoDomainData domain_data_before_update =
        Fred::InfoDomainByHandle(_domain_fqdn).set_lock().exec(_ctx).info_domain_data;

    const bool is_sponsoring_registrar = (domain_data_before_update.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_operation_permitted = (is_sponsoring_registrar || is_system_registrar);

    if (!is_operation_permitted) {
        throw AuthorizationError();
    }

    if (!is_system_registrar) {
        // do it before any object state related checks
        Fred::LockObjectStateRequestLock(domain_data_before_update.id).exec(_ctx);
        Fred::PerformObjectStateRequest(domain_data_before_update.id).exec(_ctx);

        const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(domain_data_before_delete.id).exec(ctx));
        if (domain_states.presents(Fred::Object_State::server_update_prohibited) ||
            domain_states.presents(Fred::Object_State::delete_candidate))
        {
            throw ObjectStatusProhibitsOperation();
        }
    }

    ParameterValuePolicyError parameter_value_policy_error;

    std::set<std::string> admin_contact_add_duplicity;
    unsigned short admin_contact_add_error_position = 1;
    for (
        std::vector<std::string>::const_iterator admin_contact_add = _admin_contacts_add.begin();
        admin_contact_add != _admin_contacts_add.end();
        ++admin_contact_add)
    {
        if (Fred::Contact::get_handle_registrability(_ctx, admin_contact_add) != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_add_error_position++,
                    Reason::admin_notexist));
        }
        else if (false) { // TODO
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_add_error_position,
                    Reason::admin_exist));
        }
        else if (!admin_contact_add_duplicity.insert(boost::algorithm::to_upper_copy(admin_contact_add.second)))
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_add_error_position,
                    Reason::duplicated_contact));
        }
    }

    std::set<std::string> admin_contact_rem_duplicity;
    unsigned short admin_contact_rem_error_position = 1;
    for (
        std::vector<std::string>::const_iterator admin_contact_rem = _admin_contact_rem.begin();
        admin_contact_rem != _admin_contact_rem.end();
        ++admin_contact_rem)
    {
        if (Fred::Contact::get_handle_registrability(_ctx, admin_contact_rem) != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_rem_error_position,
                    Reason::admin_notexist));
        }
        else if (false) { // TODO
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_rem_error_position,
                    Reason::admin_notexist)); // XXX
        }
        else if (!admin_contact_rem_duplicity.insert(boost::algorithm::to_upper_copy(admin_contact_add.second)))
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_rem_error_position,
                    Reason::duplicated_contact));
        }
    }

    // tmpcontact_rem
    // TODO

    // nsset
    if (!_nsset_change.empty()
    && (Fred::Nsset::get_handle_registrability(_ctx, _nsset_change)
        != Fred::NssetHandleState::Registrability::registered))
    {
        parameter_value_policy_error.add(
            Error::of_vector_parameter(
                Param::domain_nsset,
                0,
                Reason::nsset_notexist));
    }

    if (!_keyset_change.empty()
    && (Fred::KeySet::get_handle_registrability(_ctx, _keyset_change)
        != Fred::KeySet::HandleState::Registrability::registered))
    {
        parameter_value_policy_error.add(
            Error::of_vector_parameter(
                Param::domain_keyset,
                0,
                Reason::keyset_notexist));
    }

    // registrant_chg
    /*
            throw ObjectStatusProhibitsOperation();

        parameter_value_policy_error.add(
            Error::of_scalar_parameter(
                Param::domain_registrant,
                Reason::registrant_notexist));
     */

    if (!parameter_value_policy_error.is_empty()) {
        throw parameter_value_policy_error;
    }

    // 
    //// when deleting or not-changing, no check of data is needed
    //if (ContactChange::does_value_mean< ContactChange::Value::to_set >(_change.country_code)) {
    //    if (!is_country_code_valid(_ctx, ContactChange::get_value(_change.country_code))) {
    //        AggregatedParamErrors exception;
    //        exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
    //        throw exception;
    //    }
    //}

    //// update itself
    //{
    //    Fred::UpdateContactByHandle update(_domain_fqdn, callers_registrar.handle);

    //    set_ContactUpdate_member(_change.name,         update, &Fred::UpdateContactByHandle::set_name);
    //    set_ContactUpdate_member(_change.organization, update, &Fred::UpdateContactByHandle::set_organization);
    //    set_ContactUpdate_member(_change.telephone,    update, &Fred::UpdateContactByHandle::set_telephone);
    //    set_ContactUpdate_member(_change.fax,          update, &Fred::UpdateContactByHandle::set_fax);
    //    set_ContactUpdate_member(_change.email,        update, &Fred::UpdateContactByHandle::set_email);
    //    set_ContactUpdate_member(_change.notify_email, update, &Fred::UpdateContactByHandle::set_notifyemail);
    //    set_ContactUpdate_member(_change.vat,          update, &Fred::UpdateContactByHandle::set_vat);
    //    set_ContactUpdate_member(_change.auth_info_pw, update, &Fred::UpdateContactByHandle::set_authinfo);

    //    {
    //        const Ident ident(_change.ident, _change.ident_type);
    //        if (ident.does_value_mean< ContactChange::Value::to_set >()) {
    //            update.set_personal_id(ident.get());
    //        }
    //        else if (ident.does_value_mean< ContactChange::Value::to_delete >()) {
    //            update.set_personal_id(Nullable< Fred::PersonalIdUnion >());
    //        }
    //    }

    //    if (_change.disclose.is_initialized()) {
    //        _change.disclose->check_validity();
    //        set_ContactUpdate_discloseflag_address(_ctx, _change, contact_data_before_update, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::name         >(*_change.disclose, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::organization >(*_change.disclose, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::telephone    >(*_change.disclose, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::fax          >(*_change.disclose, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::email        >(*_change.disclose, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::vat          >(*_change.disclose, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::ident        >(*_change.disclose, update);
    //        set_ContactUpdate_discloseflag< ContactDisclose::Item::notify_email >(*_change.disclose, update);
    //    }
    //    else {
    //        const bool address_was_hidden = !contact_data_before_update.discloseaddress;
    //        if (address_was_hidden) {
    //            const bool address_has_to_be_disclosed = should_address_be_disclosed(_ctx,
    //                                                                                 contact_data_before_update,
    //                                                                                 _change);
    //            if (address_has_to_be_disclosed) {
    //                update.set_discloseaddress(true);
    //            }
    //        }
    //    }

    //    if (has_place_changed(_change, contact_data_before_update.place))
    //    {
    //        Fred::Contact::PlaceAddress new_place;
    //        switch (_change.streets.size())
    //        {
    //            case 3: set_data(_change.streets[2], new_place.street3);
    //            case 2: set_data(_change.streets[1], new_place.street2);
    //            case 1: set_data(_change.streets[0], new_place.street1);
    //            case 0: break;
    //            default: throw std::runtime_error("Too many streets.");
    //        }
    //        set_data(_change.city,              new_place.city);
    //        set_data(_change.state_or_province, new_place.stateorprovince);
    //        set_data(_change.postal_code,       new_place.postalcode);
    //        set_data(_change.country_code,      new_place.country);
    //        update.set_place(new_place);
    //    }

    //    if (_logd_request_id.isset()) {
    //        update.set_logd_request_id(_logd_request_id.get_value());
    //    }

    //    try {
    //        const unsigned long long new_history_id = update.exec(_ctx);
    //        return new_history_id;

    //    }
    //    catch(const Fred::UpdateContactByHandle::ExceptionType &e) {

    //        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
    //        if (e.is_set_forbidden_company_name_setting() ||
    //            e.is_set_unknown_registrar_handle() ||
    //            e.is_set_unknown_ssntype())
    //        {
    //            throw;
    //        }

    //        if (e.is_set_unknown_contact_handle())
    //        {
    //            throw NonexistentHandle();
    //        }

    //        if (e.is_set_unknown_country())
    //        {
    //            AggregatedParamErrors exception;
    //            exception.add(Error::of_scalar_parameter(Param::contact_cc, Reason::country_notexist));
    //            throw exception;
    //        }

    //        /* in the improbable case that exception is incorrectly set */
    //        throw;
    //    }
    //}
}

}

}
