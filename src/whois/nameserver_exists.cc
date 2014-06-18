#include "src/whois/nameserver_exists.h"


namespace Whois {
    bool nameserver_exists(const std::string& ns_fqdn, Fred::OperationContext& ctx ) {

        return
            ctx.get_conn().exec_params(
                "SELECT id FROM host WHERE fqdn = $1::varchar LIMIT 1",
                Database::query_param_list(ns_fqdn)
            ).size() > 0;
    }
}
