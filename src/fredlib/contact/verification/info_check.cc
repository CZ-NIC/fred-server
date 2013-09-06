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
 *  @file info_check.cc
 *  get info about contact check
 */

#include "fredlib/contact/verification/info_check.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <string>

namespace Fred
{

    std::string InfoContactCheckOutput::ContactTestResultState::to_string(const std::string& _each_line_prefix) const {
        std::string result;

        result =
            _each_line_prefix + "<InfoContactCheckOutput::ContactTestResultState> {" + "\n"
            + _each_line_prefix + _each_line_prefix + " status_name: " +      status_name + "\n"
            + _each_line_prefix + _each_line_prefix + " error_msg: " +        error_msg.print_quoted() + "\n"
            + _each_line_prefix + _each_line_prefix + " local_update_time:" + boost::posix_time::to_simple_string(local_update_time) + "\n"
            + _each_line_prefix + _each_line_prefix + " logd_request_id:" +   logd_request_id.print_quoted()
            + "\n" + _each_line_prefix + "}\n";

        return result;
    }

    std::string InfoContactCheckOutput::ContactTestResultData::to_string(const std::string& _each_line_prefix) const {
        std::string result;

        result =
            _each_line_prefix + "<InfoContactCheckOutput::ContactTestResultData> {" + "\n"
            + _each_line_prefix + _each_line_prefix + " test_name: " +         test_name + "\n"
            + _each_line_prefix + _each_line_prefix + " local_create_time: " + boost::posix_time::to_simple_string(local_create_time) + "\n"
            + _each_line_prefix + _each_line_prefix + " state_history: \n";

        for(std::vector<ContactTestResultState>::const_iterator it = state_history.begin(); it != state_history.end(); ++it) {
            result += _each_line_prefix + it->to_string(_each_line_prefix + _each_line_prefix );
        }
        result += "\n" + _each_line_prefix + "}\n";

        return result;
    }

    std::string InfoContactCheckOutput::ContactCheckState::to_string(const std::string& _each_line_prefix) const {
        std::string result;

        result =
            _each_line_prefix + "<InfoContactCheckOutput::ContactCheckState> {" + "\n"
            + _each_line_prefix + _each_line_prefix + "status_name: " +      status_name + "\n"
            + _each_line_prefix + _each_line_prefix + "local_update_time:" + boost::posix_time::to_simple_string(local_update_time) + "\n"
            + _each_line_prefix + _each_line_prefix + "logd_request_id:" +   logd_request_id.print_quoted()
            + "\n" + _each_line_prefix + "}\n";

        return result;
    }

    std::string InfoContactCheckOutput::to_string(const std::string& _each_line_prefix) const {
        std::string result;

        result =
            _each_line_prefix + "<InfoContactCheckOutput> {" + "\n"
            + _each_line_prefix + _each_line_prefix + "handle: " +             handle + "\n"
            + _each_line_prefix + _each_line_prefix + "testsuite_name: " +     testsuite_name + "\n"
            + _each_line_prefix + _each_line_prefix + "contact_history_id: " + boost::lexical_cast<std::string>(contact_history_id) + "\n"
            + _each_line_prefix + _each_line_prefix + "local_create_time:" +   boost::posix_time::to_simple_string(local_create_time) + "\n"
            + _each_line_prefix + _each_line_prefix + "check_state_history: \n";
        for(std::vector<ContactCheckState>::const_iterator it = check_state_history.begin(); it != check_state_history.end(); ++it) {
            result += it->to_string(_each_line_prefix + _each_line_prefix );
        }
        result += _each_line_prefix + _each_line_prefix + "tests: \n";
        for(std::vector<ContactTestResultData>::const_iterator it = tests.begin(); it != tests.end(); ++it) {
            result += it->to_string(_each_line_prefix + _each_line_prefix );
        }
        result += "\n" + _each_line_prefix + "}\n";

        return result;
    }

    InfoContactCheck::InfoContactCheck( const std::string& _handle)
        : handle_(_handle)
    {}

    // exec and serialization
    InfoContactCheckOutput InfoContactCheck::exec(OperationContext& _ctx, const std::string& _output_timezone) {
        try {
            InfoContactCheckOutput result;

            // get check data and lock record
            // check_history data SHOULD remain safe - are updated only via trigger at check table
            Database::Result contact_check_data = _ctx.get_conn().exec_params(
                "SELECT "
                "    check_.id                 AS id_, "
                "    check_.create_time "
                "        AT TIME ZONE 'utc' "                       /* conversion from 'utc' ... */
                "        AT TIME ZONE $1::text AS create_time_, "   /* ... to _output_timezone */
                "    check_.contact_history_id AS contact_history_id_, "
                "    testsuite.name            AS testsuite_name_, "
                "    check_.logd_request_id     AS logd_request_id_, "
                "    check_.update_time "
                "        AT TIME ZONE 'utc' "
                "        AT TIME ZONE $1::text AS update_time_, "   /* conversion from 'utc' ... */
                "    status.name               AS status_name_ "    /* ... to _output_timezone */
                "FROM contact_check AS check_ "
                "JOIN enum_contact_testsuite AS testsuite "
                "    ON check_.enum_contact_testsuite_id = testsuite.id "
                "JOIN enum_contact_check_status AS status "
                "    ON check_.enum_contact_check_status_id = status.id "
                "WHERE check_.handle=$2::uuid "
                "FOR UPDATE OF check_;",
                Database::query_param_list(_output_timezone)(handle_) );

            if (contact_check_data.size() != 1) {
                if(_ctx.get_conn().
                        exec_params(
                            "SELECT handle FROM contact_check WHERE handle=$1::uuid",
                            Database::query_param_list(handle_))
                        .size() == 0)
                {
                    throw ExceptionUnknownCheckHandle();
                }

                BOOST_THROW_EXCEPTION(Fred::InternalError("contact_check (get) info failed"));
            }

            // "basic" check data
            result.handle = handle_;
            result.local_create_time = boost::posix_time::time_from_string(static_cast<std::string>(contact_check_data[0]["create_time_"]));
            result.contact_history_id = static_cast<long>(contact_check_data[0]["contact_history_id_"]);
            result.testsuite_name = static_cast<std::string>(contact_check_data[0]["testsuite_name_"]);
            long long temp_check_id = contact_check_data[0]["id_"]; /* only used within this function, output contains handle instead */

            // check tests data
            result.tests = get_test_data(_ctx, temp_check_id, _output_timezone);

            // add all but current check state to check history
            result.check_state_history = get_check_historical_states(_ctx, temp_check_id, _output_timezone);

            // add current state to check history
            InfoContactCheckOutput::ContactCheckState temp_check_history_state;
            temp_check_history_state.logd_request_id = contact_check_data[0]["logd_request_id_"];
            temp_check_history_state.status_name = static_cast<std::string>( contact_check_data[0]["status_name_"]);
            temp_check_history_state.local_update_time = boost::posix_time::time_from_string(static_cast<std::string>( contact_check_data[0]["update_time_"]));
            result.check_state_history.push_back(temp_check_history_state);

            return result;

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }
    }


    std::ostream& operator<<(std::ostream& os, const InfoContactCheck& i) {
        os << "#InfoContactCheck handle_: " << i.handle_;

        return os;
    }

    std::string InfoContactCheck::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    std::vector<InfoContactCheckOutput::ContactTestResultData> InfoContactCheck::get_test_data(OperationContext& _ctx, long long _check_id, const std::string& _output_timezone) {
        std::vector<InfoContactCheckOutput::ContactTestResultData> result;

        // test_history data SHOULD remain safe - are updated only via trigger
        Database::Result contact_test_result = _ctx.get_conn().exec_params(
            "SELECT "
            "    test.id                   AS id_, "
            "    test.create_time "
            "        AT TIME ZONE 'utc' "                       /* conversion from 'utc' ... */
            "        AT TIME ZONE $1::text AS create_time_, "   /* ... to _output_timezone */
            "    testdef.name              AS test_name_, "
            "    test.error_msg            AS error_msg_, "
            "    test.logd_request_id      AS logd_request_id_, "
            "    test.update_time "
            "        AT TIME ZONE 'utc' "                       /* conversion from 'utc' ... */
            "        AT TIME ZONE $1::text AS update_time_, "   /* ... to _output_timezone */
            "    status.name               AS status_name_ "
            "FROM contact_test_result AS test "
            "JOIN enum_contact_test AS testdef "
            "    ON test.enum_contact_test_id = testdef.id "
            "JOIN enum_contact_test_status AS status "
            "    ON test.enum_contact_test_status_id = status.id "
            "WHERE test.contact_check_id=$2::bigint "
            "ORDER BY id_ ASC "
            "FOR UPDATE OF test;",
            Database::query_param_list(_output_timezone)(_check_id));

        if(contact_test_result.size() == 0) {
            return result;
        }

        result.reserve(contact_test_result.size());

        // there can be 0-n tests (zero being check still enqueued to run or trivial empty testcase instance)
        std::vector<std::string> test_ids;
        for(Database::Result::Iterator it = contact_test_result.begin(); it != contact_test_result.end(); ++it) {
            test_ids.push_back(
                boost::lexical_cast<std::string>(
                    static_cast<long>( (*it)["id_"]) )
                );
        }

        // get history "timelines" for all tests at once - IMPORTANT are clustered by test id
        Database::Result contact_test_history_result = _ctx.get_conn().exec_params(
            "SELECT "
            "    history.contact_test_result_id AS id_, "
            "    history.error_msg         AS error_msg_, "
            "    history.logd_request_id   AS logd_request_id_, "
            "    history.update_time "
            "        AT TIME ZONE 'utc' "                     /* conversion from 'utc' ... */
            "        AT TIME ZONE $1::text AS update_time_, " /* ... to _output_timezone */
            "    status.name               AS status_name_ "
            "FROM contact_test_result_history AS history "
            "JOIN enum_contact_test_status AS status "
            "    ON history.enum_contact_test_status_id = status.id "
            "WHERE history.contact_test_result_id = ANY($2::bigint[]) "
            "ORDER BY id_ ASC, update_time_ ASC, history.id ASC;",
            Database::query_param_list
                (_output_timezone)
                ("{" + boost::algorithm::join(test_ids, ",") + "}")
        );

        // iterator through test history "timelines"
        Database::Result::Iterator it_test_histories = contact_test_history_result.begin();

        // for each test
        for(Database::Result::Iterator it_tests = contact_test_result.begin(); it_tests != contact_test_result.end(); ++it_tests) {

            InfoContactCheckOutput::ContactTestResultData temp_test_data;
            temp_test_data.local_create_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it_tests)["create_time_"]));
            temp_test_data.test_name = static_cast<std::string>( (*it_tests)["test_name_"]);

            // for each history state of this test (NOTE - states are clustered by test id)
            while( it_test_histories != contact_test_history_result.end() ) {
                if( static_cast<long long>( (*it_test_histories)["id_"] ) != static_cast<long long>( (*it_tests)["id_"] ) ) {
                    break;
                }

                InfoContactCheckOutput::ContactTestResultState temp_test_history_state;
                temp_test_history_state.error_msg = (*it_test_histories)["error_msg_"]; // nullable has it's own implicit casting
                temp_test_history_state.local_update_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it_test_histories)["update_time_"]));
                temp_test_history_state.logd_request_id = (*it_test_histories)["logd_request_id_"]; // nullable has it's own implicit casting
                temp_test_history_state.status_name = static_cast<std::string>( (*it_test_histories)["status_name_"]);

                // add to this test history
                temp_test_data.state_history.push_back(temp_test_history_state);
                ++it_test_histories;
            }
            // current state of this test
            InfoContactCheckOutput::ContactTestResultState temp_test_current_state;
            temp_test_current_state.error_msg = (*it_tests)["error_msg_"]; // nullable has it's own implicit casting
            temp_test_current_state.local_update_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it_tests)["update_time_"]));
            temp_test_current_state.logd_request_id = (*it_tests)["logd_request_id_"]; // nullable has it's own implicit casting
            temp_test_current_state.status_name = static_cast<std::string>( (*it_tests)["status_name_"]);

            // add to this test history
            temp_test_data.state_history.push_back(temp_test_current_state);

            // "save" complete history of the current test (this iteration) to return structure
            result.push_back(temp_test_data);
        }
        if( it_test_histories != contact_test_history_result.end() ) {
            BOOST_THROW_EXCEPTION(Fred::InternalError("found state history(s) for non-existent contact_tests"));
        }

        return result;
    }

    std::vector<InfoContactCheckOutput::ContactCheckState> InfoContactCheck::get_check_historical_states(OperationContext& _ctx, long long _check_id, const std::string& _output_timezone) {
        std::vector<InfoContactCheckOutput::ContactCheckState> result;

        // get check state history
        Database::Result contact_check_historical_data = _ctx.get_conn().exec_params(
            "SELECT "
            "    history.logd_request_id   AS logd_request_id_, "
            "    history.update_time "
            "        AT TIME ZONE 'utc' "                       /* conversion from 'utc' ... */
            "        AT TIME ZONE $1::text AS update_time_, "   /* ... to _output_timezone */
            "    status.name               AS status_name_ "
            "FROM contact_check_history AS history "
            "JOIN enum_contact_check_status AS status "
            "    ON history.enum_contact_check_status_id = status.id "
            "WHERE history.contact_check_id=$2::bigint "
            ""
            "ORDER BY update_time_ ASC, history.id ASC;",
            Database::query_param_list(_output_timezone)(_check_id) );

        if(contact_check_historical_data.size() == 0) {
            return result;
        }

        result.reserve(contact_check_historical_data.size());

        // for each check historical state
        for(Database::Result::Iterator it_check_history = contact_check_historical_data.begin(); it_check_history != contact_check_historical_data.end(); ++it_check_history) {

           InfoContactCheckOutput::ContactCheckState temp_check_history_state;

           temp_check_history_state.logd_request_id = (*it_check_history)["logd_request_id_"];
           temp_check_history_state.status_name = static_cast<std::string>( (*it_check_history)["status_name_"]);
           temp_check_history_state.local_update_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it_check_history)["update_time_"]));

           result.push_back(temp_check_history_state);
        }

        return result;
    }
} // namespace Fred
