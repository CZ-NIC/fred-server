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
 *  registrar info
 */

#ifndef INFO_REGISTRAR_H_
#define INFO_REGISTRAR_H_

#include <string>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/printable.h"
#include "info_registrar_output.h"

namespace Fred
{
    /**
    * Registrar info by handle.
    * Registrar handle to get info about the registrar is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoRegistrarByHandle::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoRegistrarByHandle : public Util::Printable
    {
        const std::string handle_;/**< handle of the registrar */
        bool lock_;/**< lock object_registry row for registrar */

    public:
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown handle of the registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        /**
        * Info registrar constructor with mandatory parameter.
        * @param handle sets handle of the registrar into @ref handle_ attribute
        */
        InfoRegistrarByHandle(const std::string& handle);

        /**
        * Sets registrar lock flag.
        * @param lock sets lock registrar flag into @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoRegistrarByHandle& set_lock(bool lock = true);

        /**
        * Executes getting info about the registrar.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the registrar
        */
        InfoRegistrarOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoRegistrarByHandle

    /**
    * Registrar info by id.
    * Registrar id to get info about the registrar is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoRegistrarById::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoRegistrarById : public Util::Printable
    {
        const unsigned long long id_;/**< object id of the registrar */
        bool lock_;/**< lock object_registry row for registrar */

    public:
        DECLARE_EXCEPTION_DATA(unknown_registrar_id, unsigned long long);/**< exception members for unknown id of the registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_registrar_id<Exception>
        {};

        /**
        * Info registrar constructor with mandatory parameter.
        * @param id sets object id of the registrar into @ref id_ attribute
        */
        explicit InfoRegistrarById(unsigned long long id);

        /**
        * Sets registrar lock flag.
        * @param lock sets lock registrar flag into @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoRegistrarById& set_lock(bool lock = true);

        /**
        * Executes getting info about the registrar.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the registrar
        */
        InfoRegistrarOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//class InfoRegistrarById

}//namespace Fred

#endif//INFO_REGISTRAR_H_
