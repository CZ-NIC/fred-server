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
 *  nsset info
 */

#ifndef INFO_NSSET_H_
#define INFO_NSSET_H_

#include <string>
#include <vector>
#include <utility>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/printable.h"
#include "info_nsset_output.h"

namespace Fred
{
    /**
    * Nsset info by nsset handle.
    * Nsset handle to get info about is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoNssetByHandle::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoNssetByHandle : public Util::Printable
    {
        const std::string handle_;/**< nsset handle */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_handle, std::string);/**< exception members for unknown nsset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_handle<Exception>
        {};

        /**
        * Info nsset constructor with mandatory parameter.
        * @param handle sets nsset handle into @ref handle_ attribute
        */
        InfoNssetByHandle(const std::string& handle);

        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNssetByHandle& set_lock();

        /**
        * Executes getting info about the nsset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the nsset
        */
        InfoNssetOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoNssetByHandle

    /**
    * Nsset info by id.
    * Nsset id to get info about the nsset is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoNssetById::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoNssetById : public Util::Printable
    {
        const unsigned long long id_;/**< object id of the nsset */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_object_id, unsigned long long);/**< exception members for unknown object id of the nsset generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_id<Exception>
        {};

        /**
        * Info nsset constructor with mandatory parameter.
        * @param id sets object id of the nsset into @ref id_ attribute
        */
        explicit InfoNssetById(unsigned long long id);

        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNssetById& set_lock();

        /**
        * Executes getting info about the nsset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the nsset
        */
        InfoNssetOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoNssetById

    /**
    * Nsset history info.
    * Nsset registry object identifier to get history info about the nsset is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoNssetHistory::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoNssetHistory : public Util::Printable
    {
        const std::string roid_;/**< registry object identifier of the nsset */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier, std::string);/**< exception members for unknown registry object identifier of the nsset generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_registry_object_identifier<Exception>
        {};

        /**
        * Info nsset history constructor with mandatory parameter.
        * @param roid sets registry object identifier of the nsset into @ref roid_ attribute
        */
        InfoNssetHistory(const std::string& roid);

        /**
        * Info nsset history constructor with mandatory parameter.
        * @param roid sets registry object identifier of the nsset into @ref roid_ attribute
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        */
        InfoNssetHistory(const std::string& roid, const Optional<boost::posix_time::ptime>& history_timestamp);

        /**
        * Sets timestamp of history state we want to get.
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNssetHistory& set_history_timestamp(boost::posix_time::ptime history_timestamp);

        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNssetHistory& set_lock();

        /**
        * Executes getting history info about the nsset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the nsset
        */
        std::vector<InfoNssetOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoNssetHistory

    /**
    * Nsset info by id including history.
    * Nsset id to get info about the nsset is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref HistoryInfoNssetById::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoNssetHistoryById : public Util::Printable
    {
        unsigned long long id_;/**< object id of the nsset */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_object_id, unsigned long long);/**< exception members for unknown object id of the nsset generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_id<Exception>
        {};

        /**
        * Info nsset history constructor with mandatory parameter.
        * @param id sets object id of the nsset into @ref id_ attribute
        */
        explicit InfoNssetHistoryById(unsigned long long id);

        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNssetHistoryById& set_lock();

        /**
        * Executes getting history info about the nsset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the nsset
        */
        std::vector<InfoNssetOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class HistoryInfoNssetById

    /**
    * Nsset info by historyid.
    * Nsset historyid to get info about the nsset is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref HistoryInfoNssetByHistoryid::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoNssetHistoryByHistoryid : public Util::Printable
    {
        unsigned long long historyid_;/**< history id of the nsset */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_object_historyid, unsigned long long);/**< exception members for unknown object historyid of the nsset generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_historyid<Exception>
        {};

        /**
        * Info nsset history constructor with mandatory parameter.
        * @param historyid sets object historyid of the nsset into @ref historyid_ attribute
        */
        explicit InfoNssetHistoryByHistoryid(unsigned long long historyid);

        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNssetHistoryByHistoryid& set_lock();

        /**
        * Executes getting history info about the nsset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the nsset
        */
        InfoNssetOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class HistoryInfoNssetByHistoryid

}//namespace Fred

#endif//INFO_NSSET_H_
