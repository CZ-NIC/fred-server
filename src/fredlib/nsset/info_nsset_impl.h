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
 *  nsset info implementation
 */

#ifndef INFO_NSSET_IMPL_H_
#define INFO_NSSET_IMPL_H_

#include <string>
#include <vector>
#include <utility>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/printable.h"
#include "info_nsset_output.h"

namespace Fred
{
    /**
    * Nsset info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    */
    class InfoNsset
    {
        Optional<std::string> handle_;/**< nsset handle */
        Optional<std::string> nsset_roid_;/**< registry object identifier of the nsset */
        Optional<unsigned long long> nsset_id_;/**< object id of the nsset */
        Optional<unsigned long long> nsset_historyid_;/**< history id of the nsset */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool history_query_;/**< flag to query history records of the nsset */
        bool lock_;/**< lock object_registry row for nsset */

        std::pair<std::string, Database::QueryParams> make_nsset_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator @return pair of query string with query params*/
        std::pair<std::string, Database::QueryParams> make_tech_contact_query(unsigned long long id, unsigned long long historyid);/**< nsset technical contacts query generator @return pair of query string with query params*/
        std::pair<std::string, Database::QueryParams> make_dns_host_query(unsigned long long nssetid, unsigned long long historyid);/**< nsset DNS hosts query generator @return pair of query string with query params*/
        std::pair<std::string, Database::QueryParams> make_dns_ip_query(unsigned long long hostid, unsigned long long historyid);/**< nsset DNS hosts query generator @return pair of query string with query params*/
    public:

        /**
        * Default constructor.
        * Sets @ref history_query_ and @ref lock_ to false
        */
        InfoNsset();

        /**
        * Sets nsset handle.
        * @param handle sets nsset handle we want to get @ref handle_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_handle(const std::string& handle);

        /**
        * Sets registry object identifier of the nsset.
        * @param nsset_roid sets registry object identifier of the nsset we want to get @ref nsset_roid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_roid(const std::string& nsset_roid);

        /**
        * Sets database identifier of the nsset.
        * @param nsset_id sets object identifier of the nsset we want to get @ref nsset_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_id(unsigned long long nsset_id);

        /**
        * Sets history identifier of the nsset.
        * @param nsset_historyid sets history identifier of the nsset we want to get @ref nsset_historyid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_historyid(unsigned long long nsset_historyid);

        /**
        * Sets timestamp of history state we want to get.
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_history_timestamp(boost::posix_time::ptime history_timestamp);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_history_query(bool history_query);


        /**
        * Sets nsset lock flag.
        * @param lock sets lock nsset flag into @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_lock(bool lock = true);

        /**
        * Executes getting info about the nsset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the nsset
        */
        std::vector<InfoNssetOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

        /**
        * Executes explain analyze and getting info about the nsset for testing purposes.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @param result info data about the nsset
        * @return query and plan
        */
        std::string explain_analyze(OperationContext& ctx, std::vector<InfoNssetOutput>& result, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return query plan
    };//class InfoNsset

}//namespace Fred

#endif//INFO_NSSET_IMPL_H_
