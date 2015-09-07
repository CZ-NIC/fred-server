/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  registrar info implementation
 */

#ifndef INFO_REGISTRAR_IMPL_H_
#define INFO_REGISTRAR_IMPL_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"
#include "info_registrar_output.h"

namespace Fred
{
    /**
    * Registrar info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    */
    class InfoRegistrar
    {
        Optional<std::string> registrar_handle_;/**< handle of the registrar */
        Optional<unsigned long long> registrar_id_;/**< object id of the registrar */
        bool lock_;/**< lock row for registrar */

        std::pair<std::string, Database::QueryParams> make_registrar_query(const std::string& local_timestamp_pg_time_zone_name);/**< registrar query generator @return pair of query string with query params*/

    public:
        /**
         * Default constructor.
         * Sets @ref lock_ to false
         */
        InfoRegistrar();

        /**
        * Sets handle of the registrar.
        * @param registrar_handle sets handle of the registrar we want to get @ref registrar_handle_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoRegistrar& set_handle(const std::string& registrar_handle);

        /**
        * Sets database identifier of the registrar.
        * @param registrar_id sets object identifier of the registrar we want to get @ref registrar_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoRegistrar& set_id(unsigned long long registrar_id);

        /**
        * Sets the registrar lock flag.
        * @param lock sets lock registrar flag into @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoRegistrar& set_lock(bool lock = true);

        /**
        * Executes getting info about the registrar.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @return info data about the registrar
        */
        std::vector<InfoRegistrarOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

    };//classInfoRegistrar
}//namespace Fred

#endif//INFO_REGISTRAR_IMPL_H_
