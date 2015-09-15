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
        * @throws Exception in case of wrong input data or other predictable and superable failure.
        * @throws InternalError otherwise
        */
        InfoDomainOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };

    /**
    * Domain info by id.
    * Domain id to get info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
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
        * @throws Exception in case of wrong input data or other predictable and superable failure.
        * @throws InternalError otherwise
        */
        InfoDomainOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };

    /**
    * Domain history info.
    * Output data are arranged in descending order by historyid.
    * Domain registry object identifier to get history info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoDomainHistory  : public Util::Printable
    {
        const std::string roid_;/**< registry object identifier of the domain */
        Optional<boost::posix_time::ptime> history_timestamp_;/**< timestamp of history state we want to get (in time zone set in @ref local_timestamp_pg_time_zone_name parameter) */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
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
        * @return history info data about the domain in descending order by historyid
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };

    /**
    * Domain info by id including history.
    * Output data are arranged in descending order by historyid.
    * Domain id to get info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoDomainHistoryById : public Util::Printable
    {
        const unsigned long long id_;/**< object id of the domain */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */

    public:
        /**
        * Info domain history constructor with mandatory parameter.
        * @param id sets object id of the domain into @ref id_ attribute
        */
        explicit InfoDomainHistoryById(unsigned long long id);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainHistoryById& set_lock();

        /**
        * Executes getting history info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the domain in descending order by historyid
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };

    /**
    * Domain info by historyid.
    * Domain historyid to get info about the domain is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoDomainHistoryByHistoryid : public Util::Printable
    {
        const unsigned long long historyid_;/**< history id of the domain */
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
        explicit InfoDomainHistoryByHistoryid(unsigned long long historyid);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainHistoryByHistoryid& set_lock();

        /**
        * Executes getting history info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return history info data about the domain
        * @throws Exception in case of wrong input data or other predictable and superable failure.
        * @throws InternalError otherwise
        */
        InfoDomainOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };

    /**
    * Domains info by registrant handle.
    * Registrant handle of domains to get info about is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoDomainByRegistrantHandle : public Util::Printable
    {
        const std::string registrant_handle_;/**< registrant handle */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<unsigned long long> limit_;/**< max number of returned InfoDomainOutput structures */

    public:
        DECLARE_EXCEPTION_DATA(invalid_registrant_handle, std::string);/**< exception members for syntactically invalid registrant handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrant_handle, std::string);/**< exception members for unknown registrant handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_invalid_registrant_handle<Exception>
        , ExceptionData_unknown_registrant_handle<Exception>
        {};

        /**
        * Info domain constructor with mandatory parameter.
        * @param registrant_handle sets registrant handle into @ref registrant_handle_ attribute
        */
        InfoDomainByRegistrantHandle(const std::string& registrant_handle);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainByRegistrantHandle& set_lock();

        /**
        * Sets limit on number of returned InfoDomainOutput structures.
        * Filter query ordered by domainid.
        * If not set, there is no limit.
        * Sets  @ref limit_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomainByRegistrantHandle& set_limit(unsigned long long limit);

        /**
        * Executes getting info about domains.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about domains
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };

    /**
    * Domain info by administrator contact handle.
    * Administrator contact handle of domains to get info about is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoDomainByAdminContactHandle : public Util::Printable
    {
        const std::string admin_contact_handle_;/**< administrator contact handle */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<unsigned long long> limit_;/**< max number of returned InfoDomainOutput structures */

    public:
        DECLARE_EXCEPTION_DATA(invalid_admin_contact_handle, std::string);/**< exception members for syntactically invalid admin contact handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_admin_contact_handle, std::string);/**< exception members for unknown admin contact handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_invalid_admin_contact_handle<Exception>
        , ExceptionData_unknown_admin_contact_handle<Exception>
        {};

        /**
        * Info domain constructor with mandatory parameter.
        * @param admin_contact_handle sets domain administrator contact handle into @ref admin_contact_handle_ attribute
        */
        InfoDomainByAdminContactHandle(const std::string& admin_contact_handle);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainByAdminContactHandle& set_lock();

        /**
        * Sets limit on number of returned InfoDomainOutput structures.
        * Filter query ordered by domainid.
        * If not set, there is no limit.
        * Sets  @ref limit_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomainByAdminContactHandle& set_limit(unsigned long long limit);

        /**
        * Executes getting info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about domains
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };

    /**
    * Domains info by nsset handle.
    * Nsset handle of domains to get info about is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoDomainByNssetHandle : public Util::Printable
    {
        const std::string nsset_handle_;/**< nsset handle */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<unsigned long long> limit_;/**< max number of returned InfoDomainOutput structures */

    public:
        DECLARE_EXCEPTION_DATA(invalid_nsset_handle, std::string);/**< exception members for syntactically invalid nsset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_nsset_handle, std::string);/**< exception members for unknown nsset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_invalid_nsset_handle<Exception>
        , ExceptionData_unknown_nsset_handle<Exception>
        {};

        /**
        * Info domain constructor with mandatory parameter.
        * @param nsset_handle sets domain nsset handle into @ref nsset_handle_ attribute
        */
        InfoDomainByNssetHandle(const std::string& nsset_handle);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainByNssetHandle& set_lock();

        /**
        * Sets limit on number of returned InfoDomainOutput structures.
        * Filter query ordered by domainid.
        * If not set, there is no limit.
        * Sets  @ref limit_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomainByNssetHandle& set_limit(unsigned long long limit);

        /**
        * Executes getting info about domains.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about domains
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };


    /**
    * Domains info by keyset handle.
    * Keyset handle of domains to get info about is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoDomainByKeysetHandle : public Util::Printable
    {
        const std::string keyset_handle_;/**< keyset handle */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<unsigned long long> limit_;/**< max number of returned InfoDomainOutput structures */

    public:
        DECLARE_EXCEPTION_DATA(invalid_keyset_handle, std::string);/**< exception members for syntactically invalid keyset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_keyset_handle, std::string);/**< exception members for unknown keyset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_invalid_keyset_handle<Exception>
        , ExceptionData_unknown_keyset_handle<Exception>
        {};

        /**
        * Info domain constructor with mandatory parameter.
        * @param keyset_handle sets domain keyset handle into @ref keyset_handle_ attribute
        */
        InfoDomainByKeysetHandle(const std::string& keyset_handle);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomainByKeysetHandle& set_lock();

        /**
        * Sets limit on number of returned InfoDomainOutput structures.
        * Filter query ordered by domainid.
        * If not set, there is no limit.
        * Sets  @ref limit_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomainByKeysetHandle& set_limit(unsigned long long limit);

        /**
        * Executes getting info about domains.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about domains
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };


}//namespace Fred

#endif//INFO_DOMAIN_H_
