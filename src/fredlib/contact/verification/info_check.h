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
 *  @file create_check.h
 *  create contact check
 */

#ifndef CONTACT_VERIFICATION_INFO_CHECK_11537653419_
#define CONTACT_VERIFICATION_INFO_CHECK_11537653419_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/db/nullable.h"

namespace Fred
{
    /**
     * Return structure of operation @ref InfoContactCheck. Includes type definition of it's own parts.
     * All times are returned as local @ref InfoContactCheck::exec().
     *
     * consists of:
     * + handle
     * + testsuite name
     * + contact history id
     * + create time
     * + check state history []
     *    + status
     *    + update time
     *    + logd request id
     * + test result data []
     *    + test name
     *    + create time
     *    + test state history []
     *       + status
     *       + error msg
     *       + update time
     *       + logd request id
     */
    struct InfoContactCheckOutput {
        struct ContactTestResultState {
            std::string              status_name;
            Nullable<std::string>    error_msg;
            boost::posix_time::ptime local_update_time;
            Nullable<long long>      logd_request_id;

            std::string to_string(const std::string& _each_line_prefix = "\t") const;
            bool operator==(const ContactTestResultState& rhs) const {
                return this->to_string() == rhs.to_string();
            };
        };

        struct ContactTestResultData {
            std::string                         test_name;
            boost::posix_time::ptime            local_create_time;
            std::vector<ContactTestResultState> state_history;  /* current state is also included */

            std::string to_string(const std::string& _each_line_prefix = "\t") const;
            bool operator==(const ContactTestResultData& rhs) const {
                return this->to_string() == rhs.to_string();
            };
        };

        struct ContactCheckState {
            std::string              status_name;
            boost::posix_time::ptime local_update_time;
            Nullable<long long>      logd_request_id;

            std::string to_string(const std::string& _each_line_prefix = "\t") const;
            bool operator==(const ContactCheckState& rhs) const {
                return this->to_string() == rhs.to_string();
            };
        };


        std::string                        handle;
        std::string                        testsuite_name;
        long                               contact_history_id;
        boost::posix_time::ptime           local_create_time;
        std::vector<ContactCheckState>     check_state_history; /* current state is also included */
        std::vector<ContactTestResultData> tests;

        std::string to_string(const std::string& _each_line_prefix = "\t") const;
        bool operator==(const InfoContactCheckOutput& rhs) const {
            return this->to_string() == rhs.to_string();
        };
    };

    /**
     * Get info from existing record in contact_check table. Has no sideeffects.
     */
    class InfoContactCheck {
            std::string handle_;

        public:
            struct ExceptionUnknownCheckHandle : virtual Fred::OperationException {
                const char* what() const throw() {return "unknown check handle";}
            };

            /**
             * constructor with only parameter
             * @param _handle     identifies which contact_check to update by it's handle.
             */
            InfoContactCheck( const std::string& _handle);

            /**
             * commit operation
             * @param _output_timezone Postgres time zone input type (as string e. g. "Europe/Prague") for conversion to local time values.
             * @return Data of existing check in InfoContactCheckOutput structured.
             */
            InfoContactCheckOutput exec(OperationContext& _ctx, const std::string& _output_timezone = "Europe/Prague");
            // serialization
            friend std::ostream& operator<<(std::ostream& _os, const InfoContactCheck& _i);
            std::string to_string() const;

        private:
            /**
             * Get data for tests of specific check.
             * @param _check_id     specifies check which tests data should be retrieved
             */
            static std::vector<InfoContactCheckOutput::ContactTestResultData> get_test_data(OperationContext& _ctx, long long _check_id, const std::string& _output_timezone = "Europe/Prague");
            /**
             * Get data for historical states (explicitly: except the current state) of specific check.
             * @param _check_id     specifies check which history should be retrieved
             */
            static std::vector<InfoContactCheckOutput::ContactCheckState> get_check_historical_states(OperationContext& _ctx, long long _check_id, const std::string& _output_timezone = "Europe/Prague");
    };
}
#endif // #include guard end
