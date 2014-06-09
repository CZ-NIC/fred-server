#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

#include "src/admin/contact/verification/fill_check_queue.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/admin/contact/verification/enqueue_check.h"
#include "src/fredlib/object_state/object_state_name.h"

#include "util/log/context.h"

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
        if(_in == "tech_c") {
            return tech_c;
        }

        throw Fred::InternalError(std::string("unknown role (") + _in + ")");
    }

    static std::string is_contact_mojeid_query(std::string contact_id_column) {
        return
            "EXISTS ( "
                "SELECT * "
                    "FROM object_state AS o_s "
                        "JOIN enum_object_states AS enum_o_s "
                            "ON enum_o_s.id = o_s.state_id "
                    "WHERE o_s.object_id = " + contact_id_column + " "
                        "AND enum_o_s.name = '" + Fred::ObjectState::MOJEID_CONTACT + "' "
                        "AND o_s.valid_from <= CURRENT_TIMESTAMP "
                        "AND (o_s.valid_to IS NULL OR o_s.valid_to > CURRENT_TIMESTAMP) "
            ") ";
    }

    static std::string get_already_checked_contacts_query(Fred::OperationContext& _ctx, const std::string& _testsuite_handle) {
        return
            "SELECT "
                    "c_h.id AS contact_id_, "
                    "MAX(c_ch.update_time) AS last_update_ "
                "FROM contact_history AS c_h "
                    "JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "
            // using correct testsuite (IMPORTANT)
                    "JOIN enum_contact_testsuite AS enum_c_t ON c_ch.enum_contact_testsuite_id = enum_c_t.id "
                "WHERE enum_c_t.handle = '"+_ctx.get_conn().escape(_testsuite_handle)+"' "
                "GROUP BY contact_id_ ";
    }

    static void set_contact_filter_query(
        const contact_filter&               _filter,
        const std::string&                  _contact_alias,
        std::vector<std::string>&           _joins,
        std::vector<std::string>&           _conditions
    ) {
        using std::string;
        using std::vector;

        vector<string> joins;
        vector<string> conditions;

        Fred::OperationContext ctx;
        if(_filter.country_code.isset()) {
            conditions.push_back(
                _contact_alias+".country = upper('"
                + ctx.get_conn().escape(
                    _filter.country_code.get_value_or_default()
                ) + "')"
            );
        }

        if( !_filter.states.empty() ) {
            joins.push_back("JOIN object_state AS o_s ON o_s.object_id = "+_contact_alias+".id");
            joins.push_back("JOIN enum_object_states AS enum_o_s ON o_s.state_id = enum_o_s.id");
            conditions.push_back(
                boost::to_lower_copy(
                    "string_to_array(lower(enum_o_s.name), ',') <@ string_to_array('"
                    + ctx.get_conn().escape(boost::join(_filter.states, "', '"))
                    + "', ',')"
                    ));
            conditions.push_back("o_s.valid_to IS NULL");
        }

        if( !_filter.roles.empty() ) {
            if(_filter.roles.count(owner) == 1) {
                joins.push_back("JOIN domain AS d ON d.registrant = "+_contact_alias+".id");
            }

            if(_filter.roles.count(admin_c) == 1) {
                joins.push_back("JOIN domain_contact_map AS d_c_m ON d_c_m.contactid = "+_contact_alias+".id");
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

    static std::string get_contact_filter_query(
        const contact_filter& _filter
    ) {
        using std::string;
        using std::vector;

        // create temporary view for filtered contact ids
        vector<string> joins;
        vector<string> conditions;

        set_contact_filter_query(_filter, "c", joins, conditions);

        std::string joined_conditions = boost::algorithm::join(conditions, ") AND (" );
        if( !joined_conditions.empty() ) {
            joined_conditions = " WHERE (" + joined_conditions + ")";
        }

        return
            "SELECT DISTINCT c.id AS contact_id_ "
                "FROM contact AS c "
                + boost::algorithm::join(joins, " " ) +" "
                + joined_conditions ;
    }

    static std::string get_contacts_with_enqueued_check_query(Fred::OperationContext& _ctx) {

        return
            "SELECT "
                    "c_h.id AS contact_id_ "
                "FROM contact_history AS c_h "
                    "JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "
                    "JOIN enum_contact_check_status AS enum_c_ch_s ON c_ch.enum_contact_check_status_id = enum_c_ch_s.id "
                "WHERE "
                    "enum_c_ch_s.handle = '"+_ctx.get_conn().escape(Fred::ContactCheckStatus::ENQUEUE_REQ)+"' "
                    "OR "
                    "enum_c_ch_s.handle = '"+_ctx.get_conn().escape(Fred::ContactCheckStatus::ENQUEUED)+"' "
                "GROUP BY contact_id_ ";
    }

    static std::vector<unsigned long long> select_never_checked_contacts(
        Fred::OperationContext& _ctx,
        unsigned                _max_count,
        const std::string&      _testsuite_handle,
        contact_filter          _filter
    ) {
        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_filter AS "
            + get_contact_filter_query(_filter) );

        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_already_checked AS "
            + get_already_checked_contacts_query(_ctx, _testsuite_handle) );

        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_with_active_check AS "
            + get_contacts_with_enqueued_check_query(_ctx) );

        Database::Result never_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT o_r.id AS contact_id_ "
                "FROM object_registry AS o_r "
                    "JOIN ( "
                        "SELECT contact_id_ AS id from temp_filter "
                        "EXCEPT "
                        "SELECT contact_id_ AS id from temp_already_checked "
                    ") as filter ON o_r.id = filter.id "
                "WHERE NOT " + is_contact_mojeid_query("o_r.id") + " "
                    "AND NOT EXISTS (SELECT * FROM temp_with_active_check AS temp_u_e WHERE temp_u_e.contact_id_ = o_r.id ) "
                "LIMIT $1::integer "
                "FOR SHARE OF o_r ",
            Database::query_param_list(_max_count)
        );

        std::vector<unsigned long long> result;

        if(never_checked_contacts_res.size() == 0) {
            return result;
        }
        result.reserve(never_checked_contacts_res.size());

        for(Database::Result::Iterator it = never_checked_contacts_res.begin(); it != never_checked_contacts_res.end(); ++it) {
            result.push_back( static_cast<unsigned long long>( (*it)["contact_id_"]) );
        }

        return result;
    }

    static std::vector<unsigned long long> select_oldest_checked_contacts(
        Fred::OperationContext& _ctx,
        unsigned                _max_count,
        const std::string&      _testsuite_handle,
        contact_filter          _filter
    ) {
        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_filter AS "
            + get_contact_filter_query(_filter) );

        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_already_checked AS "
            + get_already_checked_contacts_query(_ctx, _testsuite_handle)
        );

        _ctx.get_conn().exec(
            "CREATE OR REPLACE TEMP VIEW temp_with_active_check AS "
            + get_contacts_with_enqueued_check_query(_ctx) );

        Database::Result oldest_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT o_r.id AS contact_id_ "
                "FROM object_registry AS o_r "
                    "JOIN temp_filter ON temp_filter.contact_id_ = o_r.id "
                    "JOIN temp_already_checked ON temp_already_checked.contact_id_ = o_r.id "
                "WHERE NOT " + is_contact_mojeid_query("o_r.id") + " "
                    "AND NOT EXISTS (SELECT * FROM temp_with_active_check AS temp_u_e WHERE temp_u_e.contact_id_ = o_r.id ) "
                "ORDER BY temp_already_checked.last_update_ ASC "
                "LIMIT $1::integer "
                "FOR SHARE OF o_r ",
            Database::query_param_list(_max_count)
        );


        std::vector<unsigned long long> result;

        if(oldest_checked_contacts_res.size() == 0) {
            return result;
        }
        result.reserve(oldest_checked_contacts_res.size());

        for(Database::Result::Iterator it = oldest_checked_contacts_res.begin(); it != oldest_checked_contacts_res.end(); ++it) {
            result.push_back( static_cast<unsigned long long>( (*it)["contact_id_"] ) );
        }

        return result;
    }

    fill_check_queue::fill_check_queue(const std::string& _testsuite_handle, unsigned _max_queue_length)
    :
        testsuite_handle_(_testsuite_handle),
        max_queue_length_(_max_queue_length)
    { }

    fill_check_queue& fill_check_queue::set_contact_filter(Optional<contact_filter> _filter) {
        filter_ = _filter.get_value_or_default();

        return *this;
    }
    fill_check_queue& fill_check_queue::set_logd_request_id(Optional<unsigned long long> _logd_request_id) {
        logd_request_id_ = _logd_request_id;

        return *this;
    }

    std::vector< boost::tuple<std::string, unsigned long long, unsigned long long> > fill_check_queue::exec() {
        Logging::Context log("fill_check_queue::exec");

        Fred::OperationContext ctx1;

        // how many enqueued checks are there?
        Database::Result queue_count_res = ctx1.get_conn().exec_params(
            "SELECT COUNT(c_ch.id) as count_ "
                "FROM contact_check AS c_ch "
                    "JOIN enum_contact_check_status AS enum_status ON c_ch.enum_contact_check_status_id = enum_status.id "
                    "JOIN enum_contact_testsuite AS enum_c_t ON c_ch.enum_contact_testsuite_id = enum_c_t.id "
                "WHERE enum_status.handle = ANY($1::varchar[]) "
                "AND enum_c_t.handle = $2::varchar ",
            Database::query_param_list
                (   std::string("{")
                    + boost::join(Fred::ContactCheckStatus::get_not_yet_resolved(), ",")
                    + "}"
                )
                (testsuite_handle_)
        );

        if(queue_count_res.size() != 1) {
            throw Fred::InternalError("cannot get count of contact_checks");
        }

        int checks_to_enqueue_count = static_cast<int>(max_queue_length_) - static_cast<int>(queue_count_res[0]["count_"]);

        std::vector< boost::tuple<std::string, unsigned long long, unsigned long long> > result;

        std::vector<unsigned long long> to_enqueue_never_checked;
        std::string temp_handle;

        if(checks_to_enqueue_count > 0) {
            result.reserve(checks_to_enqueue_count);

            // enqueuing never checked contacts with priority
            to_enqueue_never_checked = select_never_checked_contacts(
                ctx1,
                checks_to_enqueue_count,
                testsuite_handle_,
                filter_
            );

            BOOST_FOREACH(unsigned long long contact_id, to_enqueue_never_checked) {
                temp_handle = enqueue_check(
                    ctx1,
                    contact_id,
                    testsuite_handle_,
                    logd_request_id_
                );

                Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(uuid::from_string(temp_handle)).exec(ctx1);

                result.push_back(
                    boost::make_tuple(
                        temp_handle,
                        contact_id,
                        info.contact_history_id
                    )
                );
            }

            ctx1.commit_transaction();

            checks_to_enqueue_count -= to_enqueue_never_checked.size();
        }

        if(checks_to_enqueue_count > 0) {

            std::vector<unsigned long long> to_enqueue_oldest_checked;

            Fred::OperationContext ctx2;
            to_enqueue_oldest_checked = select_oldest_checked_contacts(
                ctx2,
                checks_to_enqueue_count,
                testsuite_handle_,
                filter_
            );

            if( !to_enqueue_oldest_checked.empty() ) {

                for(std::vector<unsigned long long>::const_iterator contact_id_it = to_enqueue_oldest_checked.begin();
                    contact_id_it != to_enqueue_oldest_checked.end();
                    ++contact_id_it
                ) {
                    // skip those checks which were enqueued as never checked
                    if( std::find(
                            to_enqueue_never_checked.begin(),
                            to_enqueue_never_checked.end(),
                            *contact_id_it
                        ) != to_enqueue_never_checked.end()
                    ) {
                        continue;
                    }

                    temp_handle = enqueue_check(
                        ctx2,
                        *contact_id_it,
                        testsuite_handle_,
                        logd_request_id_
                    );

                    Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(uuid::from_string(temp_handle)).exec(ctx2);

                    result.push_back(
                        boost::make_tuple(
                            temp_handle,
                            *contact_id_it,
                            info.contact_history_id
                        )
                    );
                }
                ctx2.commit_transaction();
            }
        }

        return result;
    }
}
}
