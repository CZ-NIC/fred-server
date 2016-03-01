#include "src/whois/domain_expiration_datetime.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/object_state_name.h"


namespace Whois
{

    boost::posix_time::ptime domain_expiration_datetime_estimate(
        Fred::OperationContext &_ctx,
        const boost::gregorian::date &_exdate
    )
    {
        /*
         * expiration_datetime = (domain.exdate + enum_parameters.regular_day_outzone_procedure_period::hour)
         *                          at time zone (enum_parameters.regular_day_procedure_zone)
         */
        Database::Result r = _ctx.get_conn().exec_params(
            "WITH"
                " proc_hour AS"
                " ("
                    " SELECT (val || ' hour')::interval AS value"
                        " FROM enum_parameters"
                      " WHERE"
                            " name = 'regular_day_outzone_procedure_period'"
                " ),"
                " proc_tz AS"
                " ("
                    " SELECT val AS value"
                        " FROM enum_parameters"
                      " WHERE"
                            " name = 'regular_day_procedure_zone'"
                " )"
                " SELECT"
                    " (($1::date + (SELECT value FROM proc_hour)::interval)::timestamp"
                        " AT TIME ZONE (SELECT value FROM proc_tz))::timestamp AS expiration_datetime",
            Database::query_param_list(_exdate)
        );
        if (r.size() == 1)
        {
            return time_from_string(static_cast<std::string>(r[0]["expiration_datetime"]));
        }
        throw std::runtime_error("domain expiration computation failed");
    }


    Optional<boost::posix_time::ptime> domain_expiration_datetime_actual(
        Fred::OperationContext &_ctx,
        unsigned long long _domain_id
    )
    {
        std::vector<Fred::ObjectStateData> states = Fred::GetObjectStates(_domain_id).exec(_ctx);
        for (std::vector<Fred::ObjectStateData>::iterator it = states.begin(); it != states.end(); ++it)
        {
            if (it->state_name == Fred::ObjectState::EXPIRED)
            {
                return Optional<boost::posix_time::ptime>(it->valid_from_time);
            }
        }
        return Optional<boost::posix_time::ptime>();
    }

}
