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
 *  contact history info
 */

#ifndef INFO_CONTACT_HISTORY_H_
#define INFO_CONTACT_HISTORY_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "fredlib/contact/info_contact_data.h"

namespace Fred
{
    /**
    * Element of the contact history data.
    */
    struct InfoContactHistoryOutput
    {
        InfoContactData info_contact_data;/**< data of the contact */

        Nullable<unsigned long long> next_historyid; /**< next historyid of the contact history*/
        boost::posix_time::ptime history_valid_from;/**< history data valid from time */
        Nullable<boost::posix_time::ptime> history_valid_to;/**< history data valid to time, null means open end */
        Nullable<unsigned long long> logd_request_id; /**< id of the request that changed contact data*/

        /**
        * Empty constructor of the contact history data structure.
        */
        InfoContactHistoryOutput()
        {}
    };

    /**
    * The contact history info.
    * Contact registry object identifier to get history info about the contact is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by caller.
    * In case of wrong input data or other predictable and superable failure the instance of @ref InfoContactHistory::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies the instance of @ref InternalError or other exception is thrown.
    */
    class InfoContactHistory
    {
        const std::string roid_;/**< registry object identifier of the contact */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool lock_;/**< lock object_registry row for contact */

    public:
        DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier, std::string);/**< exception members for unknown registry object identifier of the contact generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_registry_object_identifier<Exception>
        {};

        /**
        * Info contact history constructor with mandatory parameter.
        * @param roid sets registry object identifier of the contact into @ref roid_ attribute
        */
        InfoContactHistory(const std::string& roid);

        /**
        * Info contact history constructor with all parameters.
        * @param roid sets registry object identifier of the contact into @ref roid_ attribute
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        */
        InfoContactHistory(const std::string& roid, const Optional<boost::posix_time::ptime>& history_timestamp);

        /**
        * Sets timestamp of history state we want to get.
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContactHistory& set_history_timestamp(boost::posix_time::ptime history_timestamp);

        /**
        * Sets contact lock flag.
        * @param lock sets lock contact flag into @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContactHistory& set_lock(bool lock = true);

        /**
        * Executes getting history info about the contact.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the contact
        */
        std::vector<InfoContactHistoryOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into stream
        * @param os contains output stream reference
        * @param i reference of instance to be dumped into the stream
        * @return output stream reference
        */
        friend std::ostream& operator<<(std::ostream& os, const InfoContactHistory& i);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string();

    };//class InfoContactHistory

}//namespace Fred

#endif//INFO_CONTACT_HISTORY_H_
