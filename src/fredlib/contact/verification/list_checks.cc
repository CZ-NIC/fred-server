/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 *  (get) list (of) contact check
 */

#include "src/fredlib/contact/verification/list_checks.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/opexception.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>

#include <utility>

namespace Fred
{

    std::string ListChecksItem::to_string(const std::string& _each_line_prefix) const {
        std::string result;

        result =
            _each_line_prefix + "<ListChecksItem> {" + "\n"
            + _each_line_prefix + _each_line_prefix + " check_handle: " +       check_handle + "\n"
            + _each_line_prefix + _each_line_prefix + " testsuite_handle: " +   testsuite_handle + "\n"
            + _each_line_prefix + _each_line_prefix + " contact_history_id:" +  boost::lexical_cast<std::string>(contact_history_id) + "\n"
            + _each_line_prefix + _each_line_prefix + " local_create_time: " +  boost::posix_time::to_simple_string(local_create_time) + "\n"
            + _each_line_prefix + _each_line_prefix + " status_handle:" +       status_handle + "\n"
            + _each_line_prefix + "}\n";

        return result;
    }

    ListContactChecks::ListContactChecks(
        Optional<unsigned long>          _max_item_count,
        Optional<std::string>            _testsuite_handle,
        Optional<unsigned long long>     _contact_id,
        Optional<std::string>            _status_handle
    ) :
        max_item_count_(_max_item_count),
        testsuite_handle_(_testsuite_handle),
        contact_id_(_contact_id),
        status_handle_(_status_handle)
    { }

    ListContactChecks& ListContactChecks::set_max_item_count(unsigned long _max_item_count) {
        max_item_count_ = _max_item_count;

        return *this;
    }

    ListContactChecks& ListContactChecks::set_testsuite_handle(const std::string& _testsuite_handle) {
        testsuite_handle_ = _testsuite_handle;

        return *this;
    }

    ListContactChecks& ListContactChecks::set_contact_id(unsigned long long _contact_id) {
        contact_id_ = _contact_id;

        return *this;
    }

    ListContactChecks& ListContactChecks::set_status_handle(const std::string& _status_handle) {
        status_handle_ = _status_handle;

        return *this;
    }

    // exec and serialization
    std::vector<ListChecksItem> ListContactChecks::exec(OperationContext& _ctx, const std::string& _output_timezone) {
        _ctx.get_log().debug("ListContactChecks exec() started");
        _ctx.get_log().info(to_string());

        try {
            std::map<std::string, ListChecksItem> checks;

            const std::string check_alias = "check_";
            const std::string enum_testsuite_alias = "enum_c_t";
            const std::string contact_history_alias = "c_h";

            // select handles and basic data
            {
                std::vector<std::string> joins;
                std::vector<std::string> wheres;
                Database::QueryParams params;

                if(testsuite_handle_.isset()) {
                    // enum_contact_testsuite is already used by the fixed part of query
                    //joins.push_back();

                    wheres.push_back(
                        " AND "+ enum_testsuite_alias +".handle = $" + boost::lexical_cast<std::string>(params.size()+1) + "::varchar " );

                    params.push_back(testsuite_handle_.get_value());
                }

                if(contact_id_.isset()) {
                    // contact_historyid is already used by the fixed part of query
                    //joins.push_back();

                    wheres.push_back(
                        " AND "+contact_history_alias+".id = $" + boost::lexical_cast<std::string>(params.size()+1) + "::bigint ");

                    params.push_back(contact_id_.get_value());
                }

                if(status_handle_.isset()) {
                    joins.push_back(
                        " JOIN enum_contact_check_status AS enum_c_ch_status ON "+ check_alias +".enum_contact_check_status_id = enum_c_ch_status.id " );

                    wheres.push_back(
                        " AND enum_c_ch_status.handle = $" + boost::lexical_cast<std::string>(params.size()+1) + "::varchar ");

                    params.push_back(status_handle_.get_value());
                }

                std::string timezone_param_order = boost::lexical_cast<std::string>(params.size()+1);
                params.push_back(_output_timezone);

                Database::Result contact_check_records = _ctx.get_conn().exec_params(
                    "SELECT "
                    "    "+ check_alias +".handle               AS handle_, "

                    "    "+ check_alias +".create_time "
                    "        AT TIME ZONE 'utc' "                                   /* conversion from 'utc' ... */
                    "        AT TIME ZONE $"+timezone_param_order+"::text              "
                    "                                           AS create_time_, "  /* ... to _output_timezone */

                    "    "+ check_alias +".contact_history_id   AS contact_history_id_, "

                    "    "+ contact_history_alias +".id         AS contact_id_, "
                    "    o_r.name                               AS contact_handle_, "

                    "    "+ enum_testsuite_alias +".handle      AS testsuite_handle_, "

                    "    "+ check_alias +".update_time "
                    "        AT TIME ZONE 'utc' "                                   /* conversion from 'utc' ... */
                    "        AT TIME ZONE $"+timezone_param_order+"::text              "
                    "                                           AS update_time_, "  /* ... to _output_timezone */

                    "    status.handle                          AS status_handle_ "

                    "FROM contact_check AS "+ check_alias +" "
                    "   JOIN enum_contact_testsuite     AS "+ enum_testsuite_alias +" "
                    "       ON "+ check_alias +".enum_contact_testsuite_id = "+ enum_testsuite_alias +".id "

                    "   JOIN enum_contact_check_status  AS status "
                    "       ON "+ check_alias +".enum_contact_check_status_id = status.id "
                    "   JOIN contact_history            AS "+ contact_history_alias +" "
                    "       ON "+ check_alias +".contact_history_id = "+contact_history_alias+".historyid "
                    "   JOIN object_registry            AS o_r "
                    "       ON o_r.id = "+contact_history_alias+".id "

                    + boost::join(joins, " ") +

                    "   WHERE true "
                    + boost::join(wheres, " ") +
                    (max_item_count_.isset()
                        ?
                        "   LIMIT " + boost::lexical_cast<std::string>(max_item_count_)
                        :
                        " "
                    ),
                    params);

                for(Database::Result::Iterator it = contact_check_records.begin();
                    it != contact_check_records.end();
                    ++it
                ) {

                   ListChecksItem temp_item;

                   temp_item.check_handle = static_cast<std::string>( (*it)["handle_"] );
                   temp_item.contact_history_id = static_cast<unsigned long long>( (*it)["contact_history_id_"] );
                   temp_item.contact_handle = static_cast<std::string>( (*it)["contact_handle_"] );
                   temp_item.contact_id = static_cast<unsigned long long>( (*it)["contact_id_"] );
                   temp_item.local_create_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it)["create_time_"]));
                   temp_item.local_update_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it)["update_time_"]));
                   temp_item.local_tests_finished_time = Optional<boost::posix_time::ptime>();
                   // TODO - doimplementovat
                   temp_item.local_relevant_contact_update_time = Optional<boost::posix_time::ptime>();
                   temp_item.status_handle = static_cast<std::string>( (*it)["status_handle_"] );
                   temp_item.testsuite_handle  = static_cast<std::string>( (*it)["testsuite_handle_"] );

                   checks.insert(std::make_pair(temp_item.check_handle, temp_item));
                }
            }

            // select time when tests finished
            {
                std::vector<std::string> statuses;
                statuses.push_back(Fred::ContactCheckStatus::AUTO_OK);
                statuses.push_back(Fred::ContactCheckStatus::AUTO_FAIL);
                statuses.push_back(Fred::ContactCheckStatus::AUTO_TO_BE_DECIDED);

                std::vector<std::string> handles;
                for(std::map<std::string, ListChecksItem>::const_iterator it = checks.begin(); it != checks.end(); ++it) {
                    handles.push_back(it->second.check_handle);
                }

                Database::Result contact_check_records = _ctx.get_conn().exec_params(
                    // looking for time when check tests finished ~ check.status was set to auto_*
                    "SELECT"
                    "   "+ check_alias +".handle    AS handle_, "
                    "   MIN(c_c_h.update_time)      AS tests_finished_ "
                    "   FROM contact_check AS "+ check_alias +" "
                    "       LEFT JOIN contact_check_history      AS c_c_h "
                    "           ON "+ check_alias +".id = c_c_h.contact_check_id "
                    "       JOIN enum_contact_check_status          AS enum_c_s "
                    "           ON c_c_h.enum_contact_check_status_id = enum_c_s.id "
                    "   WHERE"
                    "       "+ check_alias +".handle = ANY($1::uuid[]) "
                    "       AND"
                    "       (enum_c_s.handle = ANY($2::varchar[]) "
                    "           OR    "
                    "       c_c_h.contact_check_id IS NULL)"
                    "   GROUP BY "+ check_alias +".handle",
                    Database::query_param_list
                    ( std::string("{") + boost::join(handles, ",") + "}")
                    ( std::string("{") + boost::join(statuses, ",") + "}")
                );
                for(Database::Result::Iterator it = contact_check_records.begin();
                    it != contact_check_records.end();
                    ++it
                ) {
                   checks.at(static_cast<std::string>( (*it)["handle_"] ))
                       .local_tests_finished_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it)["tests_finished_"]));

                }

            }

            std::vector<ListChecksItem> result;
            for(std::map<std::string, ListChecksItem>::const_iterator it = checks.begin();
                it != checks.end();
                ++it
            ) {
                result.push_back(it->second);
            }

            _ctx.get_log().debug("ListContactChecks executed successfully");

            return result;

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }
    }

    std::string ListContactChecks::to_string() const {
        using std::make_pair;
        using boost::lexical_cast;
        using std::string;

        return Util::format_operation_state(
            "ListContactChecks",
            boost::assign::list_of
                (make_pair("max_item_count",    max_item_count_.print_quoted() ))
                (make_pair("testsuite_handle",  testsuite_handle_.print_quoted() ))
                (make_pair("contact_id",        contact_id_.print_quoted() ))
                (make_pair("status_handle",     status_handle_.print_quoted() ))
        );
    }
}
