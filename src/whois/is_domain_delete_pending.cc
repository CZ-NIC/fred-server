#include "src/whois/is_domain_delete_pending.h"

namespace Whois {

    bool is_domain_delete_pending(const std::string &_fqdn, Fred::OperationContext& _ctx) {

        Database::Result result = _ctx.get_conn().exec_params(
            "SELECT "
                "oreg.name, "
                "oreg.id, "
                "oreg.crdate "
            "FROM object_registry oreg "
                "JOIN object_state os ON os.object_id = oreg.id "
                "JOIN enum_object_states eos ON  eos.id = os.state_id "
            "WHERE oreg.type = 3 "
                "AND eos.name = 'deleteCandidate' "
                "AND os.valid_from < current_timestamp "
                "AND (os.valid_to > current_timestamp OR os.valid_to IS NULL) "
                "AND ("
                    "oreg.erdate IS NULL "
                    "OR ( "
                        "oreg.erdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' "
                        ">= "
                        "date_trunc('day', (current_timestamp::timestamp AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague') "
                        "AND "
                        "oreg.erdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' "
                        "< "
                        "date_trunc('day', (current_timestamp::timestamp AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague' +  interval '1 day') "
                    ") "
                ") "
                "AND oreg.name = $1::text ",
            Database::query_param_list(_fqdn)
        );

        if (result.size() > 0) {
            LOGGER(PACKAGE).debug(
                boost::format("delete pending check for fqdn %1% selected id=%2%")
                % static_cast<std::string>(result[0][0])
                % static_cast<std::string>(result[0][1])
            );

            Database::Result check = _ctx.get_conn().exec_params(
                "SELECT oreg.name, oreg.id FROM object_registry oreg"
                " WHERE oreg.type = 3"
                " AND oreg.name = $1::text"
                " AND oreg.id != $2::bigint"
                " AND oreg.crdate > $3::timestamp",
                Database::query_param_list
                    (_fqdn)
                    (static_cast<unsigned long long>(result[0][1]))
                    (static_cast<std::string>(result[0][2]))
            );

            if (check.size() > 0) {
                LOGGER(PACKAGE).debug(
                    boost::format("delete pending check found newer domain %1% with id=%2%")
                    % static_cast<std::string>(check[0][0])
                    % static_cast<std::string>(check[0][1])
                );

                return false;
            }
            else {
                return true;
            }
        }
        else {
            return false;
        }
    }
}
