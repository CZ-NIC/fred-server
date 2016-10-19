#include "src/epp/disclose_policy.h"
#include "src/epp/domain/domain_update_impl.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/param.h"
#include "src/epp/reason.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/keyset/handle_state.h"
#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/nsset/handle_state.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/optional_value.h"

#include <boost/mpl/assert.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>

namespace Epp {

namespace Domain {

namespace {

template <class T>
class MatchesHandle {
    std::string handle_;

public:
    MatchesHandle(const std::string& _handle) : handle_(_handle) {}

    bool operator()(const T& item) const
    {
        return item.handle == handle_;
    }
};

} // namespace Epp::{anonymous}

unsigned long long domain_update_impl(
    Fred::OperationContext &_ctx,
    const std::string& _domain_fqdn,
    const Optional<std::string>& _registrant_chg,
    const Optional<std::string>& _auth_info_pw_chg,
    const Optional<std::string>& _nsset_chg,
    const Optional<std::string>& _keyset_chg,
    const std::vector<std::string>& _admin_contacts_add,
    const std::vector<std::string>& _admin_contacts_rem,
    const std::vector<std::string>& _tmpcontacts_rem,
    const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    // TODO checkRegistrarZoneAccess (JZ: 15099-epp_domain_create)
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

    Fred::LockObjectStateRequestLock(domain_data_before_update.id).exec(_ctx);
    // process object state requests
    Fred::PerformObjectStateRequest(domain_data_before_update.id).exec(_ctx);
    const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(domain_data_before_update.id).exec(_ctx));

    if (!is_system_registrar) {
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
        std::vector<std::string>::const_iterator admin_contact_add_iter = _admin_contacts_add.begin();
        admin_contact_add_iter != _admin_contacts_add.end();
        ++admin_contact_add_iter)
    {
        if (Fred::Contact::get_handle_registrability(_ctx, *admin_contact_add_iter) != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_add_error_position++,
                    Reason::admin_notexist));
        }
        else if (
            std::find_if(
                domain_data_before_update.admin_contacts.begin(),
                domain_data_before_update.admin_contacts.end(),
                MatchesHandle<Fred::ObjectIdHandlePair>(*admin_contact_add_iter)
            ) != domain_data_before_update.admin_contacts.end()
        ) {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_add_error_position,
                    Reason::admin_exist));
        }
        else if (admin_contact_add_duplicity.insert(boost::algorithm::to_upper_copy(*admin_contact_add_iter)).second == false)
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
        std::vector<std::string>::const_iterator admin_contact_rem_iter = _admin_contacts_rem.begin();
        admin_contact_rem_iter != _admin_contacts_rem.end();
        ++admin_contact_rem_iter)
    {
        if (Fred::Contact::get_handle_registrability(_ctx, *admin_contact_rem_iter) != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_rem_error_position,
                    Reason::admin_notexist));
        }
        else if (
            std::find_if (
                domain_data_before_update.admin_contacts.begin(),
                domain_data_before_update.admin_contacts.end(),
                MatchesHandle<Fred::ObjectIdHandlePair>(*admin_contact_rem_iter)
            ) == domain_data_before_update.admin_contacts.end()
        ) {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_rem_error_position,
                    Reason::admin_notexist)); // XXX přidat vhodný důvod chyby - snaha o odebrání existujícího ale nepřipojeného kontaktu
        }
        else if (admin_contact_rem_duplicity.insert(boost::algorithm::to_upper_copy(*admin_contact_rem_iter)).second == false)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_rem_error_position,
                    Reason::duplicated_contact));
        }
    }

    unsigned short tmpcontact_rem_error_position = 1;
    for (
        std::vector<std::string>::const_iterator tmpcontact_rem = _tmpcontacts_rem.begin();
        tmpcontact_rem != _tmpcontacts_rem.end();
        ++tmpcontact_rem)
    {
        parameter_value_policy_error.add(
            Error::of_vector_parameter(
                Param::domain_tmpcontact,
                tmpcontact_rem_error_position,
                Reason::admin_notexist)); // XXX přidat vhodný důvod chyby - tmpcontacts jsou obsolete
    }

    if (_nsset_chg.isset()
    && (Fred::Nsset::get_handle_registrability(_ctx, _nsset_chg.get_value())
        != Fred::NssetHandleState::Registrability::registered))
    {
        parameter_value_policy_error.add(
            Error::of_vector_parameter(
                Param::domain_nsset,
                0,
                Reason::nsset_notexist));
    }

    if (_keyset_chg.isset()
    && (Fred::KeySet::get_handle_registrability(_ctx, _keyset_chg.get_value())
        != Fred::KeySet::HandleState::registered))
    {
        parameter_value_policy_error.add(
            Error::of_vector_parameter(
                Param::domain_keyset,
                0,
                Reason::keyset_notexist));
    }

    if (_registrant_chg.isset()) {
        if (!is_system_registrar) {
            if (domain_states.presents(Fred::Object_State::server_registrant_change_prohibited)) {
                throw ObjectStatusProhibitsOperation();
            }
        }

        if (Fred::Contact::get_handle_registrability(_ctx, _registrant_chg.get_value()) != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_registrant,
                    0,
                    Reason::registrant_notexist));
        }
    }

    if (!parameter_value_policy_error.is_empty()) {
        throw parameter_value_policy_error;
    }

    // TODO enum

    {

        const std::string registrar_handle =
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle;

        Fred::UpdateDomain update_domain = Fred::UpdateDomain(
            _domain_fqdn,
            registrar_handle,
            _registrant_chg, // Optional
            _auth_info_pw_chg, // Optional
            _nsset_chg, // Optional Nullable
            _keyset_chg, // Optinal Nullable
            _admin_contacts_add,
            _admin_contacts_rem,
            Optional<boost::gregorian::date>(), // expiration_date
            Optional<boost::gregorian::date>(), // enum_validation_expiration
            Optional<bool>(), // enum_publish_flag
            Optional<unsigned long long>() // logd_request_id
        );

        try {
            const unsigned long long domain_new_history_id = update_domain.exec(_ctx);
            return domain_new_history_id;

        }
        catch(const Fred::UpdateDomain::Exception& e) {

            if (e.is_set_unknown_domain_fqdn()) {
                throw NonexistentHandle();
            }

            if (e.is_set_unknown_registrar_handle()) {
                // TODO
            }


            if (e.is_set_invalid_expiration_date()) {
                // TODO
            }

            // add_unassigned_admin_contact_handle()

            /* in the improbable case that exception is incorrectly set */
            throw;
        }
    }

}

}

}
