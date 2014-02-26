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
 *  domain info implementation
 */

#ifndef INFO_DOMAIN_IMPL_H_
#define INFO_DOMAIN_IMPL_H_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/printable.h"
#include "info_domain_output.h"

namespace Fred
{
    /**
    * Domain info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    */
    class InfoDomain
    {
        Optional<std::string> fqdn_;/**< fully qualified domain name */
        Optional<std::string> domain_roid_;/**< registry object identifier of the domain */
        Optional<unsigned long long> domain_id_;/**< object id of the domain */
        Optional<unsigned long long> domain_historyid_;/**< history id of the domain */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool history_query_;/**< flag to query history records of the domain */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

        std::pair<std::string, Database::QueryParams> make_domain_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator @return pair of query string with query params*/
        std::pair<std::string, Database::QueryParams> make_admin_query(unsigned long long id, unsigned long long historyid);/**< info query generator @return pair of query string with query params*/
    public:

        /**
        * Default constructor.
        * Sets @ref history_query_ and @ref lock_ to false
        */
        InfoDomain();

        /**
        * Sets fully qualified domain name.
        * @param fqdn sets fully qualified domain name we want to get @ref fqdn_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomain& set_fqdn(const std::string& fqdn);

        /**
        * Sets registry object identifier of the domain.
        * @param domain_roid sets registry object identifier of the domain we want to get @ref domain_roid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomain& set_roid(const std::string& domain_roid);

        /**
        * Sets database identifier of the domain.
        * @param domain_id sets object identifier of the domain we want to get @ref domain_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomain& set_id(unsigned long long domain_id);

        /**
        * Sets history identifier of the domain.
        * @param domain_historyid sets history identifier of the domain we want to get @ref domain_historyid_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomain& set_historyid(unsigned long long domain_historyid);

        /**
        * Sets timestamp of history state we want to get.
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomain& set_history_timestamp(boost::posix_time::ptime history_timestamp);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomain& set_history_query(bool history_query);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomain& set_lock();

        /**
        * Executes getting info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the domain
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

        /**
        * Executes explain analyze and getting info about the domain for testing purposes.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @param result info data about the domain
        * @return query and plan
        */
        std::string explain_analyze(OperationContext& ctx, std::vector<InfoDomainOutput>& result, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return query plan
    };//class InfoDomain

}//namespace Fred

#endif//INFO_DOMAIN_IMPL_H_
