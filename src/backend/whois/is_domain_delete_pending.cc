#include "src/backend/whois/is_domain_delete_pending.hh"

namespace Fred {
namespace Backend {
namespace Whois {

bool is_domain_delete_pending(const std::string& _fqdn, LibFred::OperationContext& _ctx, const std::string& _timezone)
{

    Database::Result result = _ctx.get_conn().exec_params(
            // clang-format off
            "WITH "
            "domain_object_type AS ( "
                "SELECT id FROM enum_object_type WHERE name = 'domain' "
            "), "
            "delete_candidate_state AS ( "
                "SELECT id FROM enum_object_states WHERE name = 'deleteCandidate' "
            "), "
            "erdate_interval AS ( "
                "SELECT "
                    // conversions are necessary because we are interested in start/end of day in local time zone
                    "date_trunc('day', NOW() AT TIME ZONE 'UTC' AT TIME ZONE $2::text) AS from_, "
                    "date_trunc('day', NOW() AT TIME ZONE 'UTC' AT TIME ZONE $2::text +  interval '1 day') AS to_ "
            ")"
            "SELECT "
                "oreg.name, "
                "oreg.id, "
                "oreg.crdate "
            "FROM object_registry AS oreg "
                "JOIN object_state AS os        ON oreg.id = os.object_id  "
                "JOIN delete_candidate_state    ON os.state_id = delete_candidate_state.id "
                "JOIN domain_object_type        ON oreg.type = domain_object_type.id, "
                "erdate_interval "
            "WHERE "
                "os.valid_from < NOW() "
                "AND (os.valid_to > NOW() OR os.valid_to IS NULL) "
                "AND ("
                    "oreg.erdate IS NULL "
                    "OR ( "
                        "oreg.erdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $2::text >= erdate_interval.from_ "
                        "AND "
                        "oreg.erdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $2::text < erdate_interval.to_"
                    ") "
                ") "
                "AND oreg.name = $1::text ",
            Database::query_param_list
                (_fqdn)
                (_timezone)
            // clang-format on
            );

    if (result.size() > 0)
    {
        LOGGER.debug(
                boost::format("delete pending check for fqdn %1% selected id=%2%") % static_cast<std::string>(result[0][0]) % static_cast<std::string>(result[0][1]));

        Database::Result check = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT oreg.name, oreg.id FROM object_registry oreg"
                " WHERE oreg.type = 3"
                " AND oreg.name = $1::text"
                " AND oreg.id != $2::bigint"
                " AND oreg.crdate > $3::timestamp",
                Database::query_param_list
                    (_fqdn)
                    (static_cast<unsigned long long>(result[0][1]))
                    (static_cast<std::string>(result[0][2]))
                // clang-format on
                );

        if (check.size() > 0)
        {
            LOGGER.debug(
                    boost::format("delete pending check found newer domain %1% with id=%2%") % static_cast<std::string>(check[0][0]) % static_cast<std::string>(check[0][1]));

            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}
} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
