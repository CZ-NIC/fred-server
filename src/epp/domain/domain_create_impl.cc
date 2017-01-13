#include "src/epp/domain/domain_create_impl.h"
#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/reason.h"
#include "src/epp/impl/util.h"

#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/nsset/check_nsset.h"
#include "src/fredlib/keyset/check_keyset.h"

#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/registrar_zone_access.h"
#include "src/fredlib/zone/zone.h"

#include "util/db/param_query_composition.h"
#include "util/optional_value.h"
#include "util/map_at.h"
#include "util/util.h"

#include <vector>
#include <map>

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>



namespace Epp {

DomainCreateResult domain_create_impl(
    Fred::OperationContext& _ctx,
    const DomainCreateInputData& _data,
    const unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
) {
    ParameterValuePolicyError parameter_value_policy_error;

    //start of db transaction, utc timestamp without timezone, will be timestamp of domain creation crdate
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
        static_cast<std::string>(_ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));

    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    //check registrar logged in
    if(_registrar_id == 0) {
        throw AuthErrorServerClosingConnection();
    }

    const Fred::Domain::DomainRegistrability::Enum domain_registrability
        = Fred::Domain::get_domain_registrability_by_domain_fqdn(_ctx, _data.fqdn);

    //check fqdn has known zone
    if(domain_registrability == Fred::Domain::DomainRegistrability::zone_not_in_registry)
    {
        throw ParameterValuePolicyError().add(Error::of_scalar_parameter(
            Param::domain_fqdn, Reason::not_applicable_domain));
    }

    //check fqdn syntax
    if(Fred::Domain::get_domain_fqdn_syntax_validity(_ctx, _data.fqdn)
        != Fred::Domain::DomainFqdnSyntaxValidity::valid )
    {
        throw ParameterValueSyntaxError().add(Error::of_scalar_parameter(
            Param::domain_fqdn, Reason::bad_format_fqdn));
    }

    //check fqdn is not already registered
    if(domain_registrability == Fred::Domain::DomainRegistrability::registered)
    {
        throw ObjectExists();
    }

    //check fqdn is not blacklisted
    if(domain_registrability == Fred::Domain::DomainRegistrability::blacklisted)
    {
        parameter_value_policy_error.add(Error::of_scalar_parameter(
            Param::domain_fqdn,Reason::blacklisted_domain));
    }

    //get zone data
    const Fred::Zone::Data zone_data = Fred::Zone::find_zone_in_fqdn(_ctx,
            Fred::Zone::rem_trailing_dot(_data.fqdn));

    //check registrar zone access permission
    if(!Fred::is_zone_accessible_by_registrar(_registrar_id, zone_data.id,
            current_local_date, _ctx))
    {
        throw AuthorizationError();
    }

    //check nsset exists if set
    if(!_data.nsset.empty()
    && (Fred::Nsset::get_handle_registrability(_ctx,_data.nsset)
        != Fred::NssetHandleState::Registrability::registered))
    {
        parameter_value_policy_error.add(Error::of_scalar_parameter(
                    Param::domain_nsset,Reason::nsset_notexist));
    }

    //check keyset exists if set
    if(!_data.keyset.empty()
    && (Fred::KeySet::get_handle_registrability(_ctx,_data.keyset)
        != Fred::KeySet::HandleState::registered))
    {
        parameter_value_policy_error.add(Error::of_scalar_parameter(
                    Param::domain_keyset,Reason::keyset_notexist));
    }

    //check registrant contact set
    if(_data.registrant.empty())
    {
        throw RequiredSpecificParameterMissing().add(Error::of_scalar_parameter(
                Param::domain_registrant,Reason::registrant_notexist));
    }

    //check registrant contact exists
    if((Fred::Contact::get_handle_registrability(_ctx,_data.registrant)
        != Fred::ContactHandleState::Registrability::registered))
    {
        parameter_value_policy_error.add(Error::of_scalar_parameter(
                Param::domain_registrant,Reason::registrant_notexist));
    }

    //check length of initial domain registration period
    unsigned domain_registration_in_months = 0;
    try
    {
        domain_registration_in_months = boost::numeric_cast<unsigned>(
            _data.period.get_length_of_domain_registration_in_months());
    }
    catch(const boost::numeric::bad_numeric_cast&)
    {
        throw ParameterValueRangeError().add(Error::of_scalar_parameter(
            Param::domain_period, Reason::period_range));
    }
    if(domain_registration_in_months == 0)
    {
        //get min domain registration period by zone
        domain_registration_in_months = zone_data.ex_period_min;
    }
    if((domain_registration_in_months < zone_data.ex_period_min)
    || (domain_registration_in_months > zone_data.ex_period_max))
    {
        throw ParameterValueRangeError().add(Error::of_scalar_parameter(
            Param::domain_period, Reason::period_range));
    }
    if(domain_registration_in_months % zone_data.ex_period_min != 0)
    {
        parameter_value_policy_error.add(
            Error::of_scalar_parameter(
                Param::domain_period,
                Reason::period_policy));
    }

    //check expiration date of ENUM domain validation
    if(zone_data.is_enum
        && (_data.enum_validation_list.empty()
            || _data.enum_validation_list.rbegin()->get_valexdate().is_special()
            )
    )
    {
        throw RequiredParameterMissing();
    }

    //check no ENUM validation date in case of non ENUM domain
    if(!zone_data.is_enum && !_data.enum_validation_list.empty())
    {
        for(unsigned i = 0; i < _data.enum_validation_list.size(); ++i )
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_ext_val_date,
                    boost::numeric_cast<unsigned short>(i),
                    Reason::valexpdate_not_used));
        }
    }

    //check range of ENUM domain validation expiration
    if(zone_data.is_enum)
    {
        const boost::gregorian::date new_valexdate = _data.enum_validation_list.rbegin()->get_valexdate();

        if(is_new_enum_domain_validation_expiration_date_invalid(
            new_valexdate, current_local_date, zone_data.enum_validation_period,
            boost::optional<boost::gregorian::date>(), _ctx))
        {
            throw ParameterValueRangeError().add(Error::of_vector_parameter(
                Param::domain_ext_val_date,
                boost::numeric_cast<unsigned short>(_data.enum_validation_list.size() - 1),
                Reason::valexpdate_not_valid));
        }
    }

    //check admin contacts
    std::set<std::string> admin_contact_duplicity;
    for(unsigned i = 0; i < _data.admin_contacts.size(); ++i)
    {
        if(Fred::Contact::get_handle_registrability(_ctx,_data.admin_contacts.at(i))
            != Fred::ContactHandleState::Registrability::registered)
        {
            parameter_value_policy_error.add(
                Error::of_vector_parameter(
                    Param::domain_admin,
                    boost::numeric_cast<unsigned short>(i),
                    Reason::admin_notexist));
        }
        else
        {//check admin contact duplicity
            if(admin_contact_duplicity.insert(boost::algorithm::to_upper_copy(
                    _data.admin_contacts.at(i))).second == false)
            {
                parameter_value_policy_error.add(Error::of_vector_parameter(Param::domain_admin,
                    boost::numeric_cast<unsigned short>(i),
                    Reason::duplicated_contact));
            }
        }
    }

    //throw accumulated errors
    if(!parameter_value_policy_error.is_empty())
    {
        throw parameter_value_policy_error;
    }

    try {

        //domain_expiration_date = current_local_date + domain_registration_in_months using postgresql date arithmetics
        const boost::gregorian::date domain_expiration_date = boost::gregorian::from_simple_string(
            static_cast<std::string>(_ctx.get_conn().exec_params(Database::ParamQuery
                ("SELECT (").param_date(current_local_date)
                (" + ").param_bigint(domain_registration_in_months)
                    (" * ('1 month'::interval))::date ")
            )[0][0]));

        const std::string registrar_handle = Fred::InfoRegistrarById(
                _registrar_id).exec(_ctx).info_registrar_data.handle;

        const Optional<boost::gregorian::date> enum_validation_expiration
        ( zone_data.is_enum
            ?  Optional<boost::gregorian::date>(_data.enum_validation_list.rbegin()->get_valexdate())
            : Optional<boost::gregorian::date>());

        const Optional<bool> enum_publish_flag
        ( zone_data.is_enum
            ?  Optional<bool>(_data.enum_validation_list.rbegin()->get_publish())
            : Optional<bool>());

        const Fred::CreateDomain::Result result = Fred::CreateDomain(
            _data.fqdn,
            registrar_handle,
            _data.registrant,
            _data.authinfo ? Optional<std::string>(*_data.authinfo) : Optional<std::string>(),
            _data.nsset.empty()
                ? Optional<Nullable<std::string> >()
                : Optional<Nullable<std::string> >(_data.nsset),
            _data.keyset.empty()
                ? Optional<Nullable<std::string> >()
                : Optional<Nullable<std::string> >(_data.keyset),
            _data.admin_contacts,
            domain_expiration_date,
            enum_validation_expiration,
            enum_publish_flag,
            _logd_request_id
        ).exec(_ctx,"UTC");

        if(result.creation_time.is_special())
        {
            throw std::runtime_error("domain create failed");
        }

        return DomainCreateResult(
            result.create_object_result.object_id,
            result.create_object_result.history_id,
            result.creation_time,
            domain_expiration_date,
            domain_registration_in_months / 12);

    } catch (const Fred::CreateDomain::Exception& e) {

        /* general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority */
        if(e.is_set_unknown_registrar_handle()) {
            throw;
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
