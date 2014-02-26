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
 *  contact info implementation
 */

#ifndef INFO_CONTACT_IMPL_H_
#define INFO_CONTACT_IMPL_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/printable.h"
#include "info_contact_output.h"

namespace Fred
{
    /**
    * Contact info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    */
    class InfoContact
    {
        Optional<std::string> contact_handle_;/**< handle of the contact */
        Optional<std::string> contact_roid_;/**< registry object identifier of the contact */
        Optional<unsigned long long> contact_id_;/**< object id of the contact */
        Optional<unsigned long long> contact_historyid_;/**< history id of the contact */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool history_query_;/**< flag to query history records of the contact */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

        std::pair<std::string, Database::QueryParams> make_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator @return pair of query string with query params*/

    public:
        /**
         * Default constructor.
         * Sets @ref history_query_ and @ref lock_ to false
         */
        InfoContact();

        /**
        * Sets handle of the contact.
        * @param contact_handle sets handle of the contact we want to get @ref contact_handle_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_handle(const std::string& contact_handle);

        /**
        * Sets registry object identifier of the contact.
        * @param contact_roid sets registry object identifier of the contact we want to get @ref contact_roid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_roid(const std::string& contact_roid);

        /**
        * Sets database identifier of the contact.
        * @param contact_id sets object identifier of the contact we want to get @ref contact_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_id(unsigned long long contact_id);

        /**
        * Sets history identifier of the contact.
        * @param contact_historyid sets history identifier of the contact we want to get @ref contact_historyid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_historyid(unsigned long long contact_historyid);

        /**
        * Sets timestamp of the history state we want to get.
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_history_timestamp(const boost::posix_time::ptime& history_timestamp);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_history_query(bool history_query);

        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_lock();

        /**
        * Executes getting info about the contact.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @return info data about the contact
        */
        std::vector<InfoContactOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

        /**
        * Executes explain analyze and getting info about the contact for testing purposes.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @param result info data about the contact
        * @return query and plan
        */
        std::string explain_analyze(OperationContext& ctx, std::vector<InfoContactOutput>& result, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return query plan

    };//classInfoContact

}//namespace Fred

#endif//INFO_CONTACT_IMPL_H_
