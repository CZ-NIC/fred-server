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
 *  domain history info
 */

#ifndef INFO_DOMAIN_H_
#define INFO_DOMAIN_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/printable.h"
#include "info_domain_output.h"

namespace Fred
{
    /**
    * Domain info by fully qualified domain name.
    * Domain fully qualified name to get info about is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoDomainByHandle::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoDomainByHandle : public Util::Printable
    {
        const std::string fqdn_;/**< fully qualified domain name */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_fqdn, std::string);/**< exception members for unknown fully qualified domain name generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_fqdn<Exception>
        {};

        /**
        * Info domain constructor with mandatory parameter.
        * @param fqdn sets fully qualified domain name into @ref fqdn_ attribute
        */
        InfoDomainByHandle(const std::string& fqdn);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainByHandle& set_lock();

        /**
        * Executes getting info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the domain
        */
        InfoDomainOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoDomainByHandle

    /**
    * Domain info by id.
    * Domain id to get info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoDomainById::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoDomainById : public Util::Printable
    {
        const unsigned long long id_;/**< object id of the domain */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_object_id, unsigned long long);/**< exception members for unknown object id of the domain generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_id<Exception>
        {};

        /**
        * Info domain constructor with mandatory parameter.
        * @param id sets object id of the domain into @ref id_ attribute
        */
        explicit InfoDomainById(unsigned long long id);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainById& set_lock();

        /**
        * Executes getting info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the domain
        */
        InfoDomainOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoDomainById

    /**
    * Domain history info.
    * Domain registry object identifier to get history info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoDomainHistory::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoDomainHistory  : public Util::Printable
    {
        const std::string roid_;/**< registry object identifier of the domain */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier, std::string);/**< exception members for unknown registry object identifier of the domain generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_registry_object_identifier<Exception>
        {};

        /**
        * Info domain history constructor with mandatory parameter.
        * @param roid sets registry object identifier of the domain into @ref roid_ attribute
        */
        InfoDomainHistory(const std::string& roid);

        /**
        * Info domain history constructor with all parameters.
        * @param roid sets registry object identifier of the domain into @ref roid_ attribute
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        */
        InfoDomainHistory(const std::string& roid, const Optional<boost::posix_time::ptime>& history_timestamp);

        /**
        * Sets timestamp of history state we want to get.
        * @param history_timestamp sets timestamp of history state we want to get @ref history_timestamp_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomainHistory& set_history_timestamp(boost::posix_time::ptime history_timestamp);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainHistory& set_lock();

        /**
        * Executes getting history info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the domain
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoDomainHistory

    /**
    * Domain info by id including history.
    * Domain id to get info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref HistoryInfoDomainById::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class HistoryInfoDomainById : public Util::Printable
    {
        unsigned long long id_;/**< object id of the domain */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_object_id, unsigned long long);/**< exception members for unknown object id of the domain generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_id<Exception>
        {};

        /**
        * Info domain history constructor with mandatory parameter.
        * @param id sets object id of the domain into @ref id_ attribute
        */
        explicit HistoryInfoDomainById(unsigned long long id);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        HistoryInfoDomainById& set_lock();

        /**
        * Executes getting history info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the domain
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class HistoryInfoDomainById

    /**
    * Domain info by historyid.
    * Domain historyid to get info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref HistoryInfoDomainByHistoryid::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class HistoryInfoDomainByHistoryid : public Util::Printable
    {
        unsigned long long historyid_;/**< history id of the domain */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        DECLARE_EXCEPTION_DATA(unknown_object_historyid, unsigned long long);/**< exception members for unknown object historyid of the domain generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_historyid<Exception>
        {};

        /**
        * Info domain history constructor with mandatory parameter.
        * @param historyid sets object historyid of the domain into @ref historyid_ attribute
        */
        explicit HistoryInfoDomainByHistoryid(unsigned long long historyid);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        HistoryInfoDomainByHistoryid& set_lock();

        /**
        * Executes getting history info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the domain
        */
        InfoDomainOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class HistoryInfoDomainByHistoryid

}//namespace Fred

#endif//INFO_DOMAIN_H_
