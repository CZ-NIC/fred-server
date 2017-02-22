#include "src/epp/disclose_policy.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/domain_update_impl.h"
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
#include "src/fredlib/keyset/handle_state.h"
#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/nsset/handle_state.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/registrar_zone_access.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/mpl/assert.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/cast.hpp>

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

} // namespace Epp::Domain::{anonymous}

unsigned long long domain_update_impl(
    Fred::OperationContext& _ctx,
    const std::string& _domain_fqdn,
    const Optional<std::string>& _registrant_chg,
    const Optional<std::string>& _auth_info_pw_chg,
    const Optional<Nullable<std::string> >& _nsset_chg,
    const Optional<Nullable<std::string> >& _keyset_chg,
    const std::vector<std::string>& _admin_contacts_add,
    const std::vector<std::string>& _admin_contacts_rem,
    const std::vector<std::string>& _tmpcontacts_rem,
    const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    bool _rifd_epp_update_domain_keyset_clear)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    const boost::gregorian::date current_local_date = boost::posix_time::microsec_clock::local_time().date();

    //check fqdn has known zone
    Fred::Zone::Data zone_data;
    try {
        zone_data = Fred::Zone::find_zone_in_fqdn(_ctx,
            Fred::Zone::rem_trailing_dot(_domain_fqdn));
    } catch (const Fred::Zone::Exception& e) {
        if(e.is_set_unknown_zone_in_fqdn()) {
            throw ObjectDoesNotExist();
        }

        throw;
    }


    if (!Fred::is_zone_accessible_by_registrar(_registrar_id, zone_data.id, current_local_date, _ctx)) {
        throw ZoneAuthorizationError();
    }


    Fred::InfoDomainData domain_data_before_update;
    try
    {
        domain_data_before_update = Fred::InfoDomainByHandle(
                Fred::Zone::rem_trailing_dot(_domain_fqdn))
            .set_lock().exec(_ctx, "UTC").info_domain_data;
    }
    catch(const Fred::InfoDomainByHandle::Exception& ex)
    {
        if(ex.is_set_unknown_fqdn())
        {
            throw ObjectDoesNotExist();
        }

        throw;
    }

    ParameterValueRangeError parameter_value_range_error;
    ParameterValuePolicyError parameter_value_policy_error;

    Optional<boost::gregorian::date> req_enum_valexdate;
    Optional<bool> enum_publish_flag;

    if(zone_data.is_enum) {

        if(!_enum_validation_list.empty()) {

            req_enum_valexdate = _enum_validation_list.rbegin()->get_valexdate();

            if(req_enum_valexdate.get_value().is_special())
            {
                parameter_value_range_error.add(Error::of_vector_parameter(
                    Param::domain_ext_val_date,
                    boost::numeric_cast<unsigned short>(_enum_validation_list.size() - 1),
                    Reason::valexpdate_not_valid));
            }
            else {

                const boost::optional<boost::gregorian::date> curr_enum_valexdate
                    = domain_data_before_update.enum_domain_validation.isnull()
                        ? boost::optional<boost::gregorian::date>()
                        : boost::optional<boost::gregorian::date>(
                            domain_data_before_update.enum_domain_validation.get_value().validation_expiration);

                if(is_new_enum_domain_validation_expiration_date_invalid(
                    req_enum_valexdate.get_value(),
                    current_local_date,
                    zone_data.enum_validation_period,
                    curr_enum_valexdate,
                    _ctx))
                {
                    parameter_value_range_error.add(Error::of_vector_parameter(
                        Param::domain_ext_val_date,
                        boost::numeric_cast<unsigned short>(_enum_validation_list.size() - 1),
                        Reason::valexpdate_not_valid));
                }
            }

            enum_publish_flag = _enum_validation_list.rbegin()->get_publish();
        }
    }
    else { // not enum
        for(
            std::vector<Epp::ENUMValidationExtension>::const_iterator enum_validation_list_item_ptr = _enum_validation_list.begin(); 
            enum_validation_list_item_ptr != _enum_validation_list.end(); 
            ++enum_validation_list_item_ptr)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_ext_val_date,
                    enum_validation_list_item_ptr - _enum_validation_list.begin(),
                    Reason::valexpdate_not_used));

        }
    }

    if (!parameter_value_range_error.is_empty()) {
        throw parameter_value_range_error;
    }

    const Fred::InfoRegistrarData session_registrar =
        Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;

    const bool is_sponsoring_registrar = (domain_data_before_update.sponsoring_registrar_handle ==
                                          session_registrar.handle);
    const bool is_system_registrar = session_registrar.system.get_value_or(false);
    const bool is_registrar_authorized = (is_sponsoring_registrar || is_system_registrar);

    if (!is_registrar_authorized) {
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

    std::set<std::string> admin_contact_add_duplicity;
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
                    admin_contact_add_iter - _admin_contacts_add.begin(),
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
                    admin_contact_add_iter - _admin_contacts_add.begin(),
                    Reason::admin_exist));
        }
        else if (admin_contact_add_duplicity.insert(boost::algorithm::to_upper_copy(*admin_contact_add_iter)).second == false)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_add,
                    admin_contact_add_iter - _admin_contacts_add.begin(),
                    Reason::duplicated_contact));
        }
    }

    std::set<std::string> admin_contact_rem_duplicity;
    for (
        std::vector<std::string>::const_iterator admin_contact_rem_iter = _admin_contacts_rem.begin();
        admin_contact_rem_iter != _admin_contacts_rem.end();
        ++admin_contact_rem_iter)
    {
        if (Fred::Contact::get_handle_registrability(_ctx, *admin_contact_rem_iter) != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_rem,
                    admin_contact_rem_iter - _admin_contacts_rem.begin(),
                    Reason::admin_notexist));
        }
        else if (
            std::find_if(
                domain_data_before_update.admin_contacts.begin(),
                domain_data_before_update.admin_contacts.end(),
                MatchesHandle<Fred::ObjectIdHandlePair>(*admin_contact_rem_iter)
            ) == domain_data_before_update.admin_contacts.end()
        ) {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_rem,
                    admin_contact_rem_iter - _admin_contacts_rem.begin(),
                    Reason::admin_not_assigned));
        }
        else if (admin_contact_rem_duplicity.insert(boost::algorithm::to_upper_copy(*admin_contact_rem_iter)).second == false)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin_rem,
                    admin_contact_rem_iter - _admin_contacts_rem.begin(),
                    Reason::duplicated_contact));
        }
    }

    for (
        std::vector<std::string>::const_iterator tmpcontact_rem = _tmpcontacts_rem.begin();
        tmpcontact_rem != _tmpcontacts_rem.end();
        ++tmpcontact_rem)
    {
        parameter_value_policy_error.add(
            Error::of_vector_parameter(
                Param::domain_tmpcontact,
                tmpcontact_rem - _tmpcontacts_rem.begin(),
                Reason::tmpcontacts_obsolete));
    }

    if (_nsset_chg.isset()
    && !_nsset_chg.get_value().isnull()
    && (Fred::Nsset::get_handle_registrability(_ctx, _nsset_chg.get_value().get_value())
        != Fred::NssetHandleState::Registrability::registered))
    {
        parameter_value_policy_error.add(
            Error::of_scalar_parameter(
                Param::domain_nsset,
                Reason::nsset_notexist));
    }

    if (_keyset_chg.isset()
    && !_keyset_chg.get_value().isnull()
    && (Fred::KeySet::get_handle_registrability(_ctx, _keyset_chg.get_value().get_value())
        != Fred::KeySet::HandleState::registered))
    {
        parameter_value_policy_error.add(
            Error::of_scalar_parameter(
                Param::domain_keyset,
                Reason::keyset_notexist));
    }

    if (_registrant_chg.isset())
    {
        if (!is_system_registrar) {
            if (domain_states.presents(Fred::Object_State::server_registrant_change_prohibited)) {
                throw ObjectStatusProhibitsOperation();
            }
        }

        if (Fred::Contact::get_handle_registrability(_ctx, _registrant_chg.get_value()) != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_scalar_parameter(
                    Param::domain_registrant,
                    Reason::registrant_notexist));
        }
    }

    if (!parameter_value_policy_error.is_empty()) {
        throw parameter_value_policy_error;
    }

    const Optional<Nullable<std::string> >& keyset_chg =
        (_nsset_chg.isset() && !_keyset_chg.isset() && _rifd_epp_update_domain_keyset_clear)
            ? Optional<Nullable<std::string> >(Nullable<std::string>())
            : _keyset_chg; // TODO if nsset set, but same as current one?

    const std::string registrar_handle =
        Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle;

    Fred::UpdateDomain update_domain = Fred::UpdateDomain(
        _domain_fqdn,
        registrar_handle,
        _registrant_chg,
        _auth_info_pw_chg,
        _nsset_chg,
        keyset_chg,
        _admin_contacts_add,
        _admin_contacts_rem,
        Optional<boost::gregorian::date>(), // expiration_date
        req_enum_valexdate,
        enum_publish_flag,
        _logd_request_id
    );

    try {
        const unsigned long long domain_new_history_id = update_domain.exec(_ctx);
        return domain_new_history_id;

    }
    catch(const Fred::UpdateDomain::Exception& e) {

        if (e.is_set_unknown_domain_fqdn()) {
            throw ObjectDoesNotExist();
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
