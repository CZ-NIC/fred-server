#include "src/epp/domain/renew_domain.h"

#include "src/epp/domain/impl/domain_billing.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/epp/reason.h"
#include "src/epp/util.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/renew_domain.h"
#include "src/fredlib/object/states_info.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/registrar_zone_access.h"
#include "src/fredlib/zone/zone.h"
#include "util/db/param_query_composition.h"
#include "util/map_at.h"
#include "util/optional_value.h"
#include "util/util.h"

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace Epp {
namespace Domain {

RenewDomainResult renew_domain(
        Fred::OperationContext& _ctx,
        const RenewDomainInputData& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const bool _rifd_epp_operations_charging)
{
    // start of db transaction, utc timestamp without timezone, will be timestamp of domain creation crdate
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
            static_cast<std::string>(_ctx.get_conn().exec(
                                             "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));

    // warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time =
        boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    // check registrar logged in
    if (_registrar_id == 0)
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    // get zone data
    // check fqdn has known zone
    Fred::Zone::Data zone_data;
    try
    {
        zone_data = Fred::Zone::find_zone_in_fqdn(_ctx, Fred::Zone::rem_trailing_dot(_data.fqdn));
    }
    catch (const Fred::Zone::Exception& e)
    {
        if (e.is_set_unknown_zone_in_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    // check registrar zone access permission
    if (!Fred::is_zone_accessible_by_registrar(
                _registrar_id,
                zone_data.id,
                current_local_date,
                _ctx))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    // check if fqdn is registered and get domain info data and lock domain for update
    Fred::InfoDomainData info_domain_data;
    try
    {
        info_domain_data = Fred::InfoDomainByHandle(
                Fred::Zone::rem_trailing_dot(_data.fqdn))
                           .set_lock()
                           .exec(_ctx, "UTC")
                           .info_domain_data;
    }
    catch (const Fred::InfoDomainByHandle::Exception& ex)
    {
        if (ex.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        throw;
    }

    // check current exdate
    try
    {
        const boost::gregorian::date current_exdate = boost::gregorian::from_simple_string(
                _data.current_exdate);
        if (current_exdate != info_domain_data.expiration_date)
        {
            throw std::runtime_error("input exdate");
        }
    }
    catch (const std::exception&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_cur_exp_date,
                                                         Reason::curexpdate_not_expdate)));
    }

    EppResultFailure parameter_value_range_errors = EppResultFailure(
            EppResultCode::parameter_value_range_error);
    // check length of new domain registration period
    unsigned domain_registration_in_months = 0;
    try
    {
        domain_registration_in_months = boost::numeric_cast<unsigned>(
                _data.period.get_length_of_domain_registration_in_months());
    }
    catch (const boost::numeric::bad_numeric_cast&)
    {
        parameter_value_range_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_range));
    }
    if (domain_registration_in_months == 0)
    {
        // get min domain registration period by zone
        domain_registration_in_months = zone_data.ex_period_min;
    }
    if ((domain_registration_in_months < zone_data.ex_period_min)
        || (domain_registration_in_months > zone_data.ex_period_max))
    {
        parameter_value_range_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_range));
    }
    if (domain_registration_in_months % zone_data.ex_period_min != 0)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::parameter_value_policy_error)
                                         .add_extended_error(
                                                 EppExtendedError::of_scalar_parameter(
                                                         Param::domain_period,
                                                         Reason::period_policy)));
    }

    // check if domain renew is possible
    Database::Result exdate_result = _ctx.get_conn().exec_params(
            Database::ParamQuery
                ("SELECT (").param_date(info_domain_data.expiration_date)
                (" + ").param_bigint(domain_registration_in_months)(
                    " * ('1 month'::interval))::date as new_exdate, ")
                ("(").param_date(current_local_date)
                (" + ").param_bigint(zone_data.ex_period_max)(" * ('1 month'::interval))::date as max_exdate"));

    const boost::gregorian::date new_exdate =
            boost::gregorian::from_simple_string(exdate_result[0]["new_exdate"]);
    const boost::gregorian::date max_exdate =
            boost::gregorian::from_simple_string(exdate_result[0]["max_exdate"]);

    // curExpDate + domain_registration_in_months <= current_local_date + zone.ex_period_max (in months)
    if (new_exdate > max_exdate)
    {
        parameter_value_range_errors.add_extended_error(
                EppExtendedError::of_scalar_parameter(
                        Param::domain_period,
                        Reason::period_range));
    }

    // check no ENUM validation date in case of non ENUM domain
    if (!zone_data.is_enum && !_data.enum_validation_list.empty())
    {
        EppResultFailure parameter_value_policy_errors = EppResultFailure(
                EppResultCode::parameter_value_policy_error);
        for (unsigned i = 0; i < _data.enum_validation_list.size(); ++i)
        {
            parameter_value_policy_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_ext_val_date,
                            boost::numeric_cast<unsigned short>(i),
                            Reason::valexpdate_not_used));
        }
        throw EppResponseFailure(parameter_value_policy_errors);
    }

    // check new ENUM domain validation expiration
    if (zone_data.is_enum && !_data.enum_validation_list.empty())
    {
        const boost::gregorian::date new_valexdate = _data.enum_validation_list.rbegin()->get_valexdate();

        // ENUM validation expiration date is optional, if missing ENUM domain is not currently validated
        const boost::optional<boost::gregorian::date> current_valexdate =
                info_domain_data.enum_domain_validation.isnull()
                        ? boost::optional<boost::gregorian::date>()
                        : boost::optional<boost::gregorian::date>(
                                  info_domain_data.enum_domain_validation.get_value().validation_expiration);

        if (is_new_enum_domain_validation_expiration_date_invalid(
                    new_valexdate,
                    current_local_date,
                    zone_data.enum_validation_period,
                    current_valexdate,
                    _ctx))
        {
            parameter_value_range_errors.add_extended_error(
                    EppExtendedError::of_vector_parameter(
                            Param::domain_ext_val_date,
                            boost::numeric_cast<unsigned short>(_data.enum_validation_list.size() - 1),
                            Reason::valexpdate_not_valid));
        }
    }

    // check sponsoring or system registrar
    const Fred::InfoRegistrarData session_registrar =
            Fred::InfoRegistrarById(_registrar_id).set_lock().exec(_ctx).info_registrar_data;

    const bool is_system_registrar = session_registrar.system.get_value_or(false);

    if (info_domain_data.sponsoring_registrar_handle != session_registrar.handle &&
        !is_system_registrar)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::authorization_error));
    }

    //  do it before any object state related checks
    Fred::LockObjectStateRequestLock(info_domain_data.id).exec(_ctx);
    Fred::PerformObjectStateRequest(info_domain_data.id).exec(_ctx);

    const Fred::ObjectStatesInfo domain_states(Fred::GetObjectStates(info_domain_data.id).exec(_ctx));

    if (!session_registrar.system.get_value_or_default() &&
            (domain_states.presents(Fred::Object_State::server_renew_prohibited)
            ||
            domain_states.presents(Fred::Object_State::delete_candidate)))
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_status_prohibits_operation));
    }

    if (!parameter_value_range_errors.empty())
    {
        throw EppResponseFailure(EppResultFailure(parameter_value_range_errors));
    }

    try
    {
        Fred::RenewDomain renew_domain = Fred::RenewDomain(
                _data.fqdn,
                session_registrar.handle,
                new_exdate);

        if (zone_data.is_enum && !_data.enum_validation_list.empty())
        {
            renew_domain.set_enum_validation_expiration(_data.enum_validation_list.rbegin()->get_valexdate())
                    .set_enum_publish_flag(_data.enum_validation_list.rbegin()->get_publish());
        }

        if (_logd_request_id.isset())
        {
            renew_domain.set_logd_request_id(_logd_request_id.get_value());
        }

        unsigned long long renewed_domain_history_id = renew_domain.exec(_ctx);

        if (_rifd_epp_operations_charging &&
            !is_system_registrar)
        {
            renew_domain_bill_item(
                    _data.fqdn,
                    current_utc_time,
                    _registrar_id,
                    info_domain_data.id,
                    domain_registration_in_months,
                    info_domain_data.expiration_date,
                    new_exdate,
                    _ctx);
        }

        return RenewDomainResult(
                info_domain_data.id,
                renewed_domain_history_id,
                current_utc_time,
                info_domain_data.expiration_date,
                new_exdate,
                domain_registration_in_months);

    }
    catch (const Fred::RenewDomain::Exception& e)
    {
        throw;
    }
    catch (const BillingFailure&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::billing_failure));
    }
}


} // namespace Epp::Domain
} // namespace EPp
