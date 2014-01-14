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
        const std::string&                  _contact_alias,
        std::vector<std::string>&           _joins,
        std::vector<std::string>&           _conditions
    ) {
        using std::string;
        using std::vector;
        using std::set;

        vector<string> joins;
        vector<string> conditions;

        Fred::OperationContext ctx;
        if(_filter.country_code.isset()) {
            conditions.push_back(
                _contact_alias+".country = upper('"
                + ctx.get_conn().escape(
                    static_cast<string>(
                        _filter.country_code
                    )
                ) + "')"
            );
        }

        if(_filter.states.empty() == false) {
            joins.push_back("JOIN object_state AS o_s ON o_s.object_id = "+_contact_alias+".id");
            joins.push_back("JOIN enum_object_states AS enum_o_s ON o_s.state_id = enum_o_s.id");
            conditions.push_back(
                "string_to_array(enum_o_s.name, ',') <@ string_to_array('"
                + ctx.get_conn().escape(boost::join(_filter.states, "', '"))
                + "', ',')");
            conditions.push_back("o_s.valid_to IS NULL");
        }

        if(_filter.roles.empty() == false) {
            if(_filter.roles.count(owner) == 1) {
                joins.push_back("JOIN domain AS d ON d.registrant = "+_contact_alias+".id");
            }

            if(_filter.roles.count(admin_c) == 1) {
                joins.push_back("JOIN domain_contact_map AS d_c_m ON d_c_m.registrant = "+_contact_alias+".id");
                conditions.push_back("d_c_m.role = 1");
            }

            if(_filter.roles.count(tech_c) == 1) {
                joins.push_back(
                    "JOIN ( "
                    "   SELECT contactid AS contact_id FROM nsset_contact_map "
                    "   UNION "
                    "   SELECT contactid AS contact_id FROM keyset_contact_map "
                    ") AS tech_cont ON "+_contact_alias+".id = tech_cont.contact_id"
                );
            }
        }

        _joins = joins;
        _conditions = conditions;
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

       // create temporary view for filtered contact ids
        vector<string> joins;
        vector<string> conditions;

        set_contact_filter_query(_filter, "c", joins, conditions);

        std::string joined_conditions = boost::algorithm::join(conditions, ") AND (" );
        if(joined_conditions.length() > 0) {
            joined_conditions = " WHERE (" + joined_conditions + ")";
        }

        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_filter AS "
            "   SELECT DISTINCT c.id "
            "       FROM contact AS c "
            "       "+ boost::algorithm::join(joins, " " ) +" "
            "       "+joined_conditions
        );

       // create temporary view for already checked contact ids
        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_already_checked AS "
            "   SELECT DISTINCT c_h.id "
            "       FROM contact_history AS c_h "
            "           JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "
            // using correct testsuite (IMPORTANT)
            "           JOIN enum_contact_testsuite AS enum_c_t ON c_ch.enum_contact_testsuite_id = enum_c_t.id "
            "       WHERE enum_c_t.name = '"+_ctx.get_conn().escape(_testsuite_name)+"'"
        );

        Database::Result never_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT o_r.id AS contact_id_ "
            "    FROM object_registry AS o_r "
            "        JOIN ( "
            "            SELECT id from temp_filter "
            "            EXCEPT "
            "            SELECT id from temp_already_checked "
            "        ) as filter ON o_r.id = filter.id "
            "    LIMIT $1::integer "
            "    FOR SHARE OF o_r ",
            Database::query_param_list(_max_count)
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

        // create temporary view for filtered contact ids
        vector<string> joins;
        vector<string> conditions;

        set_contact_filter_query(_filter, "c", joins, conditions);

        std::string joined_conditions = boost::algorithm::join(conditions, ") AND (" );
        if(joined_conditions.length() > 0) {
            joined_conditions = " WHERE (" + joined_conditions + ")";
        }

        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_filter AS "
            "   SELECT DISTINCT c.id AS contact_id_ "
            "       FROM contact AS c "
            "       "+ boost::algorithm::join(joins, " " ) +" "
            "       "+joined_conditions
        );

        // create temporary view for already checked contact ids
        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_already_checked AS "
            "   SELECT "
            "       c_h.id AS contact_id_, "
            "       MAX(c_ch.update_time) AS last_update_ "
            "       FROM contact_history AS c_h "
            "           JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "
            // using correct testsuite (IMPORTANT)
            "           JOIN enum_contact_testsuite AS enum_c_t ON c_ch.enum_contact_testsuite_id = enum_c_t.id "
            "       WHERE enum_c_t.name = '"+_ctx.get_conn().escape(_testsuite_name)+"'"
            "       GROUP BY contact_id_ "
        );

        Database::Result oldest_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT o_r.id AS contact_id_ "
            "    FROM object_registry AS o_r "
            "        JOIN temp_filter ON temp_filter.contact_id_ = o_r.id "
            "        JOIN temp_already_checked ON temp_already_checked.contact_id_ = o_r.id "
            "    ORDER BY temp_already_checked.last_update_ ASC "
            "    LIMIT $1::integer "
            "    FOR SHARE OF o_r ",
            Database::query_param_list(_max_count)
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
