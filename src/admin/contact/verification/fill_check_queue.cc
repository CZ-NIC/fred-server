#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

#include "admin/contact/verification/fill_check_queue.h"
#include "fredlib/opcontext.h"
#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_testsuite_name.h"
#include "fredlib/contact/verification/enum_check_status.h"

namespace  Admin {
namespace ContactVerificationQueue {

    std::string to_string(allowed_contact_roles _in) {
        switch(_in) {
            case owner:
                return "owner";
            case admin_c:
                return "admin_c";
            case tech_c:
                return "tech_c";
        }

        throw Fred::InternalError(std::string("unknown role"));
    }

    allowed_contact_roles from_string(const std::string& _in) {
        if(_in == "owner") {
            return owner;
        }
        if(_in == "admin_c") {
            return admin_c;
        }
        if(_in == "admin_c") {
            return admin_c;
        }

        throw Fred::InternalError(std::string("unknown role (") + _in + ")");
    }

    static void set_contact_filter_query(
        const contact_filter&               _filter,
        const std::string&                  _object_registry_table_alias,
        std::vector<std::string>&           _joins,
        std::vector<std::string>&           _conditions,
        std::vector<Database::QueryParam>&  _params
    ) {
        using std::string;
        using std::vector;
        using std::set;

        vector<string> joins;
        vector<string> conditions;
        vector<Database::QueryParam> params;

        std::string (* int2str)(const int&) = &boost::lexical_cast<string>;

        bool contact_joined = false;

        if(_filter.country_code.isset()) {
            joins.push_back("JOIN contact AS c ON "+ _object_registry_table_alias + ".id = c.id");
            conditions.push_back("c.country = $" + int2str(params.size() + 1) + "::text");
            params.push_back(static_cast<string>(_filter.country_code));
            contact_joined = true;
        }

        if(_filter.states.empty() == false) {
            if(contact_joined == false) {
                joins.push_back("JOIN contact AS c ON "+ _object_registry_table_alias + ".id = c.id");
                contact_joined = true;
            }

            joins.push_back("JOIN object_state AS o_s ON o_s.object_id = c.id");
            joins.push_back("JOIN enum_object_states AS enum_o_s ON o_s.state_id = enum_o_s.id");
            conditions.push_back( "string_to_array(enum_o_s.name, ',') <@ string_to_array($" + int2str(params.size() + 1) + "::text, ',')");
            params.push_back( boost::join(_filter.states, ", ") );
        }

        if(_filter.roles.empty() == false) {
            if(_filter.roles.count(owner) == 1) {
                if(contact_joined == false) {
                    joins.push_back("JOIN contact AS c ON "+ _object_registry_table_alias + ".id = c.id");
                    contact_joined = true;
                }
                joins.push_back("JOIN domain AS d ON d.registrant = c.id");
            }

            if(_filter.roles.count(admin_c) == 1) {
                if(contact_joined == false) {
                    joins.push_back("JOIN contact AS c ON "+ _object_registry_table_alias + ".id = c.id");
                    contact_joined = true;
                }
                joins.push_back("JOIN domain_contact_map AS d_c_m ON d_c_m.registrant = c.id");
                conditions.push_back("d_c_m.role = 1");
            }

            if(_filter.roles.count(tech_c) == 1) {
                if(contact_joined == false) {
                    joins.push_back("JOIN contact AS c ON "+ _object_registry_table_alias + ".id = c.id");
                    contact_joined = true;
                }
                joins.push_back(
                    "JOIN ( "
                    "   SELECT contactid AS contact_id FROM nsset_contact_map "
                    "   UNION "
                    "   SELECT contactid AS contact_id FROM keyset_contact_map "
                    ") AS tech_cont ON c.id = tech_cont.contact_id"
                );
            }
        }

        _joins = joins;
        _conditions = conditions;
        _params = params;
    }

    static std::vector<long long> select_never_checked_contacts(
            Fred::OperationContext& _ctx,
            unsigned                _max_count,
            const std::string&      _testsuite_name,
            contact_filter          _filter
    ) {
        using std::string;
        using std::vector;
        using std::set;

        string object_registry_alias = "o_r";

        vector<string> joins;
        vector<string> conditions;
        vector<Database::QueryParam> params;

        set_contact_filter_query(_filter, object_registry_alias, joins, conditions, params);

        params.push_back(_testsuite_name);
        std::string testsuite_param_index = boost::lexical_cast<string>(params.size());
        params.push_back(_max_count); // hint: params.size() ++
        std::string max_count_param_index = boost::lexical_cast<string>(params.size());

        std::string joined_conditions = boost::algorithm::join(conditions, ") AND (" );
        if(joined_conditions.length() > 0) {
            joined_conditions = " AND (" + joined_conditions + ")";
        }

        Database::Result never_checked_contacts_res = _ctx.get_conn().exec_params(
            // because postgres can't do "SELECT DISTINCT ... FOR SHARE" another sub-SELECT and JOIN is used
            "SELECT object_registry.id AS contact_id_ "
            "   FROM object_registry "
            "       JOIN ("
            // DISTINCT is needed because by JOINs id can be at multiple rows in result
            "           SELECT DISTINCT " + object_registry_alias + ".id AS contact_id2_ "
            "               FROM object_registry AS " + object_registry_alias + " "
            "                   " + boost::algorithm::join(joins, " ") + " "
            // has not yet been checked...
            "               WHERE NOT EXISTS ( "
            "                   SELECT * "
            "                       FROM contact_history AS c_h "
            "                           JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "
            // ...using "this" testsuite (IMPORTANT)
            "                           JOIN enum_contact_testsuite AS enum_c_t ON c_ch.enum_contact_testsuite_id = enum_c_t.id "
            "                       WHERE c_h.id = " + object_registry_alias + ".id "
            "                           AND enum_c_t.name = $" + testsuite_param_index + "::varchar "
            "               ) "
            "               AND " + object_registry_alias + ".type = 1 "
            +               joined_conditions +
            "               LIMIT $" + max_count_param_index + "::integer "
            "       ) AS distinct_ ON distinct_.contact_id2_ = object_registry.id "
            "   FOR SHARE OF object_registry; ",
            params
        );

        std::vector<long long> result;

        if(never_checked_contacts_res.size() == 0) {
            return result;
        }
        result.reserve(never_checked_contacts_res.size());

        for(Database::Result::Iterator it = never_checked_contacts_res.begin(); it != never_checked_contacts_res.end(); ++it) {
            result.push_back( static_cast<long long>( (*it)["contact_id_"]) );
        }

        return result;
    }

    static std::vector<long long> select_oldest_checked_contacts(
        Fred::OperationContext& _ctx,
        unsigned                _max_count,
        const std::string&      _testsuite_name,
        contact_filter          _filter
    ) {
        using std::string;
        using std::vector;
        using std::set;

        string object_registry_alias = "o_r";

        vector<string> joins;
        vector<string> conditions;
        vector<Database::QueryParam> params;

        set_contact_filter_query(_filter, object_registry_alias, joins, conditions, params);
        params.push_back(_testsuite_name);
        std::string testsuite_param_index = boost::lexical_cast<string>(params.size());
        params.push_back(_max_count); // hint: params.size() ++
        std::string max_count_param_index = boost::lexical_cast<string>(params.size());
        params.push_back("contact"); // hint: params.size() ++
        std::string contact_param_index = boost::lexical_cast<string>(params.size());

        std::string joined_conditions = boost::algorithm::join(conditions, ") AND (" );
        if(joined_conditions.length() > 0) {
            joined_conditions = " AND (" + joined_conditions + ")";
        }

        Database::Result oldest_checked_contacts_res = _ctx.get_conn().exec_params(
            // because postgres can't do "SELECT DISTINCT ... FOR SHARE" another sub-SELECT and JOIN is used
            "SELECT object_registry.id AS contact_id_ "
            "   FROM object_registry "
            "       JOIN ("
            // DISTINCT is needed because by JOINs id can be at multiple rows in result
            "           SELECT DISTINCT ON (" + object_registry_alias + ".id) "
            "               " + object_registry_alias + ".id AS contact_id2_, "
            // need this because of GROUP BY
            "               filtered_.last_update_ "
            "               FROM object_registry AS " + object_registry_alias + " "
            "                   JOIN ("
            "                       SELECT o_r.name AS name_, MAX(c_ch.update_time) AS last_update_ "
            "                           FROM contact_check AS c_ch "
            "                               JOIN contact_history AS c_h ON c_ch.contact_history_id = c_h.historyid "
            "                               JOIN object_registry AS o_r ON c_h.id = o_r.id "
            "                               JOIN enum_contact_testsuite AS enum_c_t ON c_ch.enum_contact_testsuite_id = enum_c_t.id "
            "                           WHERE enum_c_t.name = $" + testsuite_param_index + "::text "
            "                           GROUP BY name_ "
            "                   ) AS filtered_ ON " + object_registry_alias + ".name = filtered_.name_ "
            "                   " + boost::algorithm::join(joins, " ") + " "
            "                   JOIN enum_object_type AS e_o_t ON " + object_registry_alias + ".type = e_o_t.id "
            "               WHERE e_o_t.name = $"+contact_param_index+"::varchar "
            "               " + joined_conditions + " "
            "       ) AS distinct_ ON object_registry.id = distinct_.contact_id2_ "
            // because postgres can order primarily only by DISTINCT value
            "   ORDER BY distinct_.last_update_ ASC "
            "   LIMIT $" + max_count_param_index + "::integer "
            "   FOR SHARE OF object_registry ",
            params
        );

        std::vector<long long> result;

        if(oldest_checked_contacts_res.size() == 0) {
            return result;
        }
        result.reserve(oldest_checked_contacts_res.size());

        for(Database::Result::Iterator it = oldest_checked_contacts_res.begin(); it != oldest_checked_contacts_res.end(); ++it) {
            result.push_back( static_cast<long long>( (*it)["contact_id_"] ) );
        }

        return result;
    }

    fill_check_queue::fill_check_queue(std::string _testsuite_name, unsigned _max_queue_length)
    :
        testsuite_name_(_testsuite_name),
        max_queue_length_(_max_queue_length)
    { }

    fill_check_queue& fill_check_queue::set_contact_filter(Optional<contact_filter> _filter) {
        filter_ = _filter;

        return *this;
    }
    fill_check_queue& fill_check_queue::set_logd_request_id(Optional<long long> _logd_request_id) {
        logd_request_id_ = _logd_request_id;

        return *this;
    }

    std::vector< boost::tuple<std::string, long long, long long> > fill_check_queue::exec() {
        Fred::OperationContext ctx1;

        // how many enqueued checks are there?
        Database::Result queue_count_res = ctx1.get_conn().exec_params(
            "SELECT COUNT(c_ch.id) as count_ "
            "   FROM contact_check AS c_ch "
            "       JOIN enum_contact_check_status AS enum_status ON c_ch.enum_contact_check_status_id = enum_status.id "
            "   WHERE enum_status.name = $1::varchar",
            Database::query_param_list(Fred::ContactCheckStatus::ENQUEUED)
        );

        if(queue_count_res.size() != 1) {
            throw Fred::InternalError("cannot get count of contact_checks");
        }

        int checks_to_enqueue_count = static_cast<int>(max_queue_length_) - static_cast<int>(queue_count_res[0]["count_"]);

        std::vector< boost::tuple<std::string, long long, long long> > result;

        std::vector<long long> to_enqueue;
        std::string temp_handle;

        if(checks_to_enqueue_count > 0) {
            result.reserve(checks_to_enqueue_count);

            // enqueuing never checked contacts with priority
            to_enqueue = select_never_checked_contacts(
                ctx1,
                checks_to_enqueue_count,
                testsuite_name_,
                filter_
            );

            BOOST_FOREACH(long long contact_id, to_enqueue) {
                temp_handle = Fred::CreateContactCheck(
                    contact_id,
                    testsuite_name_,
                    logd_request_id_
                ).exec(ctx1);

                Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(temp_handle).exec(ctx1);

                result.push_back(
                    boost::make_tuple(
                        temp_handle,
                        contact_id,
                        info.contact_history_id
                    )
                );
            }

            ctx1.commit_transaction();

            checks_to_enqueue_count -= to_enqueue.size();
            to_enqueue.empty();
        }

        if(checks_to_enqueue_count > 0) {
            /* Filling the queue until it's full even if that means planning several consecutive runs of checks.
             *
             * Never checked contacts cannot be used for this as checks for those are all planned before these "repeated" checks
             * and once those are gone the only non-empty set is contacts ready for repeated check.
             */

            Fred::OperationContext ctx2;
            to_enqueue = select_oldest_checked_contacts(
                ctx2,
                checks_to_enqueue_count,
                testsuite_name_,
                filter_
            );

            if(to_enqueue.empty() == false) {
                std::vector<long long>::iterator contact_id_it = to_enqueue.begin();

                for(; checks_to_enqueue_count > 0; --checks_to_enqueue_count) {
                    temp_handle = Fred::CreateContactCheck(
                        *contact_id_it,
                        testsuite_name_
                    )
                    .set_logd_request_id(logd_request_id_)
                    .exec(ctx2);

                    Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(temp_handle).exec(ctx2);

                    result.push_back(
                        boost::make_tuple(
                            temp_handle,
                            *contact_id_it,
                            info.contact_history_id
                        )
                    );

                    ++contact_id_it;
                    if(contact_id_it == to_enqueue.end() ) {
                        contact_id_it = to_enqueue.begin();
                    }
                }
                ctx2.commit_transaction();
            }
        }

        return result;
    }
}
}
