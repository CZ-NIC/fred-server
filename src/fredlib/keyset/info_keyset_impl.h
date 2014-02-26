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
 *  keyset info implementation
 */

#ifndef INFO_KEYSET_IMPL_H_
#define INFO_KEYSET_IMPL_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "info_keyset_output.h"
#include "util/printable.h"
namespace Fred
{
    /**
    * Keyset info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    */
    class InfoKeyset
    {
        Optional<std::string> handle_;/**< keyset handle */
        Optional<std::string> keyset_roid_;/**< registry object identifier of the keyset */
        Optional<unsigned long long> keyset_id_;/**< object id of the keyset */
        Optional<unsigned long long> keyset_historyid_;/**< history id of the keyset */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool history_query_;/**< flag to query history records of the keyset */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

        std::pair<std::string, Database::QueryParams> make_keyset_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator @return pair of query string with query params*/
        std::pair<std::string, Database::QueryParams> make_tech_contact_query(unsigned long long id, unsigned long long historyid);/**< keyset technical contacts query generator @return pair of query string with query params*/
        std::pair<std::string, Database::QueryParams> make_dns_keys_query(unsigned long long id, unsigned long long historyid);/**< keyset DNS keys query generator @return pair of query string with query params*/
    public:

        /**
        * Default constructor.
        * Sets @ref history_query_ and @ref lock_ to false
        */
        InfoKeyset();

        /**
        * Sets keyset handle.
        * @param handle sets keyset handle we want to get @ref handle_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoKeyset& set_handle(const std::string& handle);

        /**
        * Sets registry object identifier of the keyset.
        * @param keyset_roid sets registry object identifier of the keyset we want to get @ref keyset_roid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoKeyset& set_roid(const std::string& keyset_roid);

        /**
        * Sets database identifier of the keyset.
        * @param keyset_id sets object identifier of the keyset we want to get @ref keyset_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoKeyset& set_id(unsigned long long keyset_id);

        /**
        * Sets history identifier of the keyset.
        * @param keyset_historyid sets history identifier of the keyset we want to get @ref keyset_historyid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoKeyset& set_historyid(unsigned long long keyset_historyid);

        /**
        * Sets timestamp of history state we want to get.
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoKeyset& set_history_timestamp(boost::posix_time::ptime history_timestamp);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoKeyset& set_history_query(bool history_query);


        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoKeyset& set_lock();

        /**
        * Executes getting info about the keyset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the keyset
        */
        std::vector<InfoKeysetOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

        /**
        * Executes explain analyze and getting info about the keyset for testing purposes.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @param result info data about the keyset
        * @return query and plan
        */
        std::string explain_analyze(OperationContext& ctx, std::vector<InfoKeysetOutput>& result, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return query plan
    };//class InfoKeyset

}//namespace Fred

#endif//INFO_KEYSET_IMPL_H_
