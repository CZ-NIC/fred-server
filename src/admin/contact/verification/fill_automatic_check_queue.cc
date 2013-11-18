#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include "admin/contact/verification/fill_automatic_check_queue.h"
#include "fredlib/opcontext.h"
#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_testsuite_name.h"
#include "fredlib/contact/verification/enum_check_status.h"

namespace  Admin {

    static std::vector<long long> select_never_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length);
    static std::vector<long long> select_oldest_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length);

    std::vector< boost::tuple<std::string, long long, long long> > fill_automatic_check_queue(unsigned _max_queue_length, Optional<long long> _logd_request_id) {
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

        int checks_to_enqueue_count = static_cast<int>(_max_queue_length) - static_cast<int>(queue_count_res[0]["count_"]);

        std::vector< boost::tuple<std::string, long long, long long> > result;

        std::vector<long long> to_enqueue;
        std::string temp_handle;

        if(checks_to_enqueue_count > 0) {
            result.reserve(checks_to_enqueue_count);

            // enqueuing never checked contacts with priority
            to_enqueue = select_never_checked_contacts(ctx1, checks_to_enqueue_count);

            BOOST_FOREACH(long long contact_id, to_enqueue) {
                temp_handle = Fred::CreateContactCheck(
                    contact_id,
                    Fred::TestsuiteName::AUTOMATIC,
                    _logd_request_id
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
            to_enqueue = select_oldest_checked_contacts(ctx2, checks_to_enqueue_count);
            ctx2.get_conn().exec("ROLLBACK");

            if(to_enqueue.empty()) {
                throw Fred::InternalError("can't find checked contacts");
            }

            std::vector<long long>::iterator contact_id_it = to_enqueue.begin();
            Fred::OperationContext ctx3;

            for(; checks_to_enqueue_count > 0; --checks_to_enqueue_count) {
                temp_handle = Fred::CreateContactCheck(
                    *contact_id_it,
                    Fred::TestsuiteName::AUTOMATIC
                )
                .set_logd_request_id(_logd_request_id)
                .exec(ctx3);

                Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(temp_handle).exec(ctx3);

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
            ctx3.commit_transaction();
        }

        return result;
    }

    std::vector<long long> select_never_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length) {
        Database::Result never_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT o_r.id AS contact_id_ "
            "   FROM contact AS c "
            "       JOIN object_registry AS o_r USING(id) "
            "       JOIN contact_history AS c_h USING(id) "
            "       LEFT JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "    // left join not null trick
            "   WHERE c_ch.handle IS NULL "
            "   LIMIT $1::integer "
            "   FOR SHARE OF o_r; ",
            Database::query_param_list(_max_queue_length)
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

    std::vector<long long> select_oldest_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length) {
        Database::Result oldest_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT obj_reg.id AS contact_id_ "
            "   FROM object_registry AS obj_reg "
            "       JOIN ("
            "           SELECT o_r.name AS name_, MAX(c_ch.update_time) AS last_update_ "
            "               FROM contact_check AS c_ch "
            "                   JOIN contact_history AS c_h ON c_ch.contact_history_id = c_h.historyid "
            "                   JOIN object_registry AS o_r ON c_h.id = o_r.id "
            "               GROUP BY name_ "
            "       ) AS filtered_ ON obj_reg.name = filtered_.name_ "
            "   WHERE obj_reg.type = 1 "
            "   ORDER BY filtered_.last_update_ ASC "
            "   LIMIT $1::integer "
            "   FOR SHARE OF obj_reg; ",
            Database::query_param_list(_max_queue_length)
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
}
