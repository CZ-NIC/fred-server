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

    static std::vector<std::string> select_never_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length);
    static std::vector<std::string> select_oldest_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length);

    std::vector< boost::tuple<std::string, std::string, long long> > fill_automatic_check_queue(unsigned _max_queue_length) {
        Fred::OperationContext ctx;

        // how many enqueued checks are there?
        Database::Result queue_count_res = ctx.get_conn().exec_params(
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

        std::vector< boost::tuple<std::string, std::string, long long> > result;

        if(checks_to_enqueue_count > 0) {
            result.reserve(checks_to_enqueue_count);

            // enqueuing never checked contacts with priority
            std::vector<std::string> to_enqueue = select_never_checked_contacts(ctx, checks_to_enqueue_count);
            std::string temp_handle;
            BOOST_FOREACH(const std::string& contact_name, to_enqueue) {
                temp_handle = Fred::CreateContactCheck(contact_name, Fred::TestsuiteName::AUTOMATIC).exec(ctx);
                Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(temp_handle).exec(ctx);

                result.push_back(
                    boost::make_tuple(
                        temp_handle,
                        contact_name,
                        info.contact_history_id
                    )
                );
            }
            checks_to_enqueue_count -= to_enqueue.size();
            to_enqueue.empty();

            /* Filling the queue until it's full even if that means planning several consecutive runs of checks.
             *
             * Never checked contacts cannot be used for this as checks for those are all planned before these "repeated" checks
             * and once those are gone the only non-empty set is contacts ready for repeated check.
             */
            while(checks_to_enqueue_count > 0) {
                to_enqueue = select_oldest_checked_contacts(ctx, checks_to_enqueue_count);

                /* just a safety measure
                 * should never be triggered but if something goes wrong then it's better than undless loop
                 */
                if(to_enqueue.empty()) {
                    break;
                }

                BOOST_FOREACH(const std::string& contact_name, to_enqueue) {
                    temp_handle = Fred::CreateContactCheck(contact_name, Fred::TestsuiteName::AUTOMATIC).exec(ctx);
                    Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(temp_handle).exec(ctx);

                    result.push_back(
                        boost::make_tuple(
                            temp_handle,
                            contact_name,
                            info.contact_history_id
                        )
                    );
                }
                checks_to_enqueue_count -= to_enqueue.size();
                to_enqueue.empty();
            }
        }

        ctx.commit_transaction();

        return result;
    }

    std::vector<std::string> select_never_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length) {
        Database::Result never_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT o_r.name AS contact_handle_ "
            "   FROM contact AS c "
            "       JOIN object_registry AS o_r USING(id) "
            "       JOIN contact_history AS c_h USING(id) "
            "       LEFT JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "    // left join not null trick
            "   WHERE c_ch.handle IS NULL "
            "   LIMIT $1::integer "
            "   FOR SHARE OF o_r; ",
            Database::query_param_list(_max_queue_length)
        );

        std::vector<std::string> result;

        if(never_checked_contacts_res.size() == 0) {
            return result;
        }
        result.reserve(never_checked_contacts_res.size());

        for(Database::Result::Iterator it = never_checked_contacts_res.begin(); it != never_checked_contacts_res.end(); ++it) {
            result.push_back( static_cast<std::string>( (*it)["contact_handle_"]) );
        }

        return result;
    }

    std::vector<std::string> select_oldest_checked_contacts(Fred::OperationContext& _ctx, unsigned _max_queue_length) {
        Database::Result oldest_checked_contacts_res = _ctx.get_conn().exec_params(
            "SELECT obj_reg.name AS contact_handle_ "
            "   FROM object_registry AS obj_reg "
            "       JOIN ("
            "           SELECT o_r.name AS name_, MAX(c_ch.update_time) AS last_update_ "
            "               FROM contact_check AS c_ch "
            "                   JOIN contact_history AS c_h ON c_ch.contact_history_id = c_h.historyid "
            "                   JOIN object_registry AS o_r ON c_ch.id = o_r.id "
            "               GROUP BY name_ "
            "       ) AS filtered_ ON obj_reg.name = filtered_.name_ "
            "   WHERE obj_reg.type = 1 "
            "   ORDER BY filtered_.last_update_ DESC "
            "   LIMIT $1::integer "
            "   FOR SHARE OF obj_reg; ",
            Database::query_param_list(_max_queue_length)
        );

        std::vector<std::string> result;

        if(oldest_checked_contacts_res.size() == 0) {
            return result;
        }
        result.reserve(oldest_checked_contacts_res.size());

        for(Database::Result::Iterator it = oldest_checked_contacts_res.begin(); it != oldest_checked_contacts_res.end(); ++it) {
            result.push_back( static_cast<std::string>( (*it)["contact_handle_"] ) );
        }

        return result;
    }
}
