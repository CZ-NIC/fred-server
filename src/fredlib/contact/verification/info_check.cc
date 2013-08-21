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

namespace Fred
{
    InfoContactCheck::InfoContactCheck( const std::string& _handle)
        : handle_(_handle)
    {}

    // exec and serialization
    InfoContactCheckOutput InfoContactCheck::exec(OperationContext& _ctx, const std::string& _output_timezone) {

        InfoContactCheckOutput result;

        try {
            // get check data and lock record
            // check_history data SHOULD remain safe - are updated only via trigger at check table
            Database::Result contact_check_data = _ctx.get_conn().exec_params(
                "SELECT "
                "    check.id                  AS id_, "
                "    check.create_time "
                "        AT TIME ZOME 'utc' "                       /* conversion from 'utc' ... */
                "        AT TIME ZONE $1::text AS create_time_, "   /* ... to _output_timezone */
                "    check.contact_history_id  AS contact_history_id_, "
                "    testsuite.name            AS testsuite_name_ "
                "FROM contact_check AS check"
                "JOIN enum_contact_testsuite AS testsuite "
                "    ON check.enum_contact_testsuite_id = testsuite.id"
                "WHERE check.handle=$2::cnt_chck_handle"
                "FOR UPDATE OF check;",
                Database::query_param_list(_output_timezone)(handle_) );

            if (contact_check_data.size() != 1) {
               BOOST_THROW_EXCEPTION(Fred::InternalError("contact_check (get) info failed"));
            }

            result.handle = handle_;
            result.utc_create_time = boost::posix_time::time_from_string(static_cast<std::string>(contact_check_data[0]["create_time_"]));
            result.contact_history_id = static_cast<long>(contact_check_data[0]["contact_history_id_"]);
            result.testsuite_name = static_cast<std::string>(contact_check_data[0]["testsuite_name_"]);
            long long temp_check_id = contact_check_data[0]["id_"]; /* only used within this function, output contains handle instead */

            // get tests data and lock records
            // test_history data SHOULD remain safe - are update only via trigger
            Database::Result contact_test_result = _ctx.get_conn().exec_params(
                "SELECT "
                "    test.id                   AS id_, "
                "    test.create_time "
                "        AT TIME ZOME 'utc' "                       /* conversion from 'utc' ... */
                "        AT TIME ZONE $1::text AS create_time_, "   /* ... to _output_timezone */
                "    testdef.name              AS test_name_"
                "FROM contact_test_result AS test"
                "JOIN enum_contact_test AS testdef"
                "    ON test.enum_contact_test_id = testdef.id"
                "WHERE test.contact_check_id=$2::bigint"
                "ORDER BY id_ ASC"
                "FOR UPDATE OF test;",
                Database::query_param_list(_output_timezone)(temp_check_id));

            if(contact_test_result.size() > 0) {
                // there can be 0-n tests (zero being check still enqueued to run or trivial empty testcase instance)
                std::vector<long long> test_ids;
                for(Database::Result::Iterator it = contact_test_result.begin(); it != contact_test_result.end(); ++it) {
                    test_ids.push_back(
                        static_cast<long>( (*it)["id_"]) );
                }

                // get history "timelines" for all tests at once
                Database::Result contact_test_history_result = _ctx.get_conn().exec_params(
                    "SELECT "
                    "    test.id                   AS id_, "
                    "    test.error_msg            AS error_msg_, "
                    "    test.logd_request_id      AS logd_request_id_, "
                    "    test.update_time "
                    "        AT TIME ZOME 'utc' "                       /* conversion from 'utc' ... */
                    "        AT TIME ZONE $1::text AS update_time_, "   /* ... to _output_timezone */
                    "    status.name               AS status_name_ "
                    "FROM contact_test_result AS test"
                    "JOIN enum_contact_check_status AS status"
                    "    ON test.enum_contact_status_id = status.id"
                    "WHERE test.id IN"
                    ""
                    "UNION ALL " /* only reason for "ALL" is to disable search for duplicates in postgres*/
                    ""
                    "SELECT "
                    "    history.contact_test_result_id AS id_, "
                    "    history.error_msg         AS error_msg_, "
                    "    history.logd_request_id   AS logd_request_id_, "
                    "    history.update_time "
                    "        AT TIME ZOME 'utc' "                     /* conversion from 'utc' ... */
                    "        AT TIME ZONE $1::text AS update_time_, " /* ... to _output_timezone */
                    "    status.name               AS status_name_ "
                    "FROM contact_test_result_history AS history"
                    "JOIN enum_contact_test_status AS status"
                    "    ON history.enum_contact_test_status_id = status.id"
                    "WHERE history.contact_test_result_id="
                    "$2::bigint[]"
                    ""
                    "ORDER BY id_ ASC, update_time_ ASC;",
                    Database::query_param_list
                        (_output_timezone)
                        ("ARRAY[" + boost::algorithm::join(boost::lexical_cast<std::string>(test_ids), ",") + "]")
                );

                result.tests.reserve(contact_test_result.size());

                // iterator through test history "timelines"
                Database::Result::Iterator it_test_histories = contact_test_history_result.begin();

                // for each test
                for(Database::Result::Iterator it_tests = contact_test_result.begin(); it_tests != contact_test_result.end(); ++it_tests) {

                    InfoContactCheckOutput::ContactTestResultData temp_test_data;
                    temp_test_data.local_create_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it_tests)["create_time_"]));
                    temp_test_data.test_name = static_cast<std::string>( (*it_tests)["test_name_"]);

                    // for each history state of this test
                    while( (*it_test_histories)["id_"] == (*it_tests)["id_"] && it_test_histories != contact_test_history_result.end() ) {
                        InfoContactCheckOutput::ContactTestResultState temp_test_history_state;
                        temp_test_history_state.error_msg = static_cast<std::string>( (*it_test_histories)["error_msg_"]);
                        temp_test_history_state.local_update_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it_test_histories)["update_time_"]));
                        temp_test_history_state.logd_request_id = static_cast<long long>( (*it_test_histories)["logd_request_id_"]);
                        temp_test_history_state.status_name = static_cast<std::string>( (*it_test_histories)["status_name_"]);

                        // add to this test history
                        temp_test_data.state_history.push_back(temp_test_history_state);
                        ++it_test_histories;
                        if( it_test_histories == contact_test_history_result.end() ) {
                            if( it_tests + 1 != contact_test_result.end() ) {
                                BOOST_THROW_EXCEPTION(Fred::InternalError("missing state history for contact_test(s)"));
                            }
                        }
                    }
                    if( it_test_histories != contact_test_history_result.end() ) {
                        BOOST_THROW_EXCEPTION(Fred::InternalError("found state history(s) for non-existent contact_tests"));
                    }

                    // "save" complete history of the current test (this iteration) to return structure
                    result.tests.push_back(temp_test_data);
                }
            }
            // get check state history
            Database::Result contact_check_historical_data = _ctx.get_conn().exec_params(
                "SELECT "
                "    check.logd_request_id     AS logd_request_id_, "
                "    check.update_time "
                "        AT TIME ZOME 'utc' "
                "        AT TIME ZONE $1::text AS update_time_, "
                "    status.name               AS status_name_ "
                "FROM contact_check AS check"
                "JOIN enum_contact_check_status AS status"
                "    ON check.enum_contact_status_id = status.id"
                "WHERE check.id=$1::bigint"
                ""
                "UNION ALL " /* only reason for "ALL" is to disable search for duplicates in postgres*/
                ""
                "SELECT "
                "    history.logd_request_id   AS logd_request_id_, "
                "    history.update_time "
                "        AT TIME ZOME 'utc' "                       /* conversion from 'utc' ... */
                "        AT TIME ZONE $1::text AS update_time_, "   /* ... to _output_timezone */
                "    status.name               AS status_name_"
                "FROM contact_check_history AS history"
                "JOIN enum_contact_check_status AS status"
                "    ON check.enum_contact_status_id = status.id"
                "WHERE history.contact_check_id=$2::bigint"
                ""
                "ORDER BY status_name_ ASC;",
                Database::query_param_list(_output_timezone)(temp_check_id) );

            if (contact_check_historical_data.size() < 1) {
               // at least in (non-historical) contact_check table should be a record
               BOOST_THROW_EXCEPTION(Fred::InternalError("contact_check (get) info failed"));
            }

            result.check_state_history.reserve(contact_check_historical_data.size());

            // for each check historical (current included) state
            for(Database::Result::Iterator it_check_history = contact_check_historical_data.begin(); it_check_history != contact_check_historical_data.end(); ++it_check_history) {

               InfoContactCheckOutput::ContactCheckState temp_check_history_state;

               temp_check_history_state.logd_request_id = static_cast<long long>( (*it_check_history)["logd_request_id_"]);
               temp_check_history_state.status_name = static_cast<std::string>( (*it_check_history)["status_name_"]);
               temp_check_history_state.local_update_time = boost::posix_time::time_from_string(static_cast<std::string>( (*it_check_history)["update_time_"]));

               result.check_state_history.push_back(temp_check_history_state);
            }

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }

        return result;
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
} // namespace Fred
