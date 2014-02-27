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

#ifndef CONTACT_VERIFICATION_LIST_CHECKS_213454867445_
#define CONTACT_VERIFICATION_LIST_CHECKS_213454867445_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>

#include "util/printable.h"

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Fred
{
    /**
     * Return structure of operation @ref ListContactCheck.
     * All times are returned as local @ref ListContactChecks::exec().
     */
    struct ListChecksItem {
        std::string                         check_handle;
        std::string                         testsuite_handle;
        unsigned long long                  contact_history_id;
        unsigned long long                  contact_id;
        std::string                         contact_handle;
        boost::posix_time::ptime            local_create_time;
        boost::posix_time::ptime            local_update_time;
        boost::posix_time::ptime            local_last_contact_update;
        std::string                         status_handle;

        std::string to_string(const std::string& _each_line_prefix = "\t") const;
    };

    /**
     * Get list of existing record in contact_check table. Has no sideeffects.
     */
    class ListContactChecks : public Util::Printable {
            Optional<unsigned long>          max_item_count_;
            Optional<std::string>            testsuite_handle_;
            Optional<unsigned long long>     contact_id_;
            Optional<std::string>            status_handle_;

        public:
            /**
             * constructor with only mandatory parameter
             * @param _max_item_count     how many records shall be returned at most.
             */
            ListContactChecks() { };

            /**
             * constructor with all available parameters including optional ones
             * @param _max_item_count   how many records shall be returned at most.
             * @param _testsuite_handle filter: only checks with given testsuite are returned
             * @param _contact_id       filter: only checks of given contact (connected by historyid) are returned
             * @param _status_handle    filter: only checks with given status are returned
             */
            ListContactChecks(
                Optional<unsigned long>         _max_item_count,
                Optional<std::string>           _testsuite_handle,
                Optional<unsigned long long>    _contact_id,
                Optional<std::string>           _status_handle
            );

            /**
             * setter of optional max_item_count_
             * Call with another value for re-set, no need to unset first.
             */
            ListContactChecks& set_max_item_count(unsigned long _max_item_count);

            /**
             * setter of optional testsuite_handle
             * Call with another value for re-set, no need to unset first.
             */
            ListContactChecks& set_testsuite_handle(const std::string& _testsuite_handle);

            /**
             * setter of optional contact_id
             * Call with another value for re-set, no need to unset first.
             */
            ListContactChecks& set_contact_id(unsigned long long _contact_id);

            /**
             * setter of optional status_handle
             * Call with another value for re-set, no need to unset first.
             */
            ListContactChecks& set_status_handle(const std::string& _status_handle);

            /**
             * commit operation
             * @param _output_timezone Postgres time zone input type (as string e. g. "Europe/Prague") for conversion to local time values.
             * @return Data of existing check in InfoContactCheckOutput structured.
             */
            std::vector<ListChecksItem> exec(OperationContext& _ctx, const std::string& _output_timezone = "Europe/Prague");
            // serialization
            virtual std::string to_string() const;
    };
}
#endif // #include guard end
