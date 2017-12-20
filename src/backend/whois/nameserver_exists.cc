#include "src/backend/whois/nameserver_exists.hh"


namespace Whois {
    bool nameserver_exists(const std::string& ns_fqdn, LibFred::OperationContext& ctx ) {

        return
            ctx.get_conn().exec_params(
                "SELECT 123 FROM host WHERE fqdn = lower($1::varchar) LIMIT 1",
                Database::query_param_list(ns_fqdn)
            ).size() > 0;
    }
}
