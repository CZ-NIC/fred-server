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
 *  contact info
 */

#ifndef INFO_CONTACT_H_
#define INFO_CONTACT_H_

#include <string>
#include <vector>
#include <set>

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
    * Contact info data structure.
    */
    struct InfoContactOutput : public Util::Printable
    {
        InfoContactData info_contact_data;/**< data of the contact */
        boost::posix_time::ptime utc_timestamp;/**< timestamp of getting the contact data in UTC */
        boost::posix_time::ptime local_timestamp;/**< timestamp of getting the contact data in local time zone viz @ref local_timestamp_pg_time_zone_name */

        /**
        * Empty constructor of the contact data structure.
        */
        InfoContactOutput()
        {}

        /**
        * Equality of the contact data structure.
        * Compares only data of the contact @ref info_contact_data, not the timestamps.
        * @param rhs is right hand side of the contact data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoContactOutput& rhs) const
        {
            return info_contact_data == rhs.info_contact_data;
        }

        /**
        * Inequality of the contact data structure.
        * Compares only data of the contact, not the timestamps.
        * @param rhs is right hand side of the contact data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoContactOutput& rhs) const
        {
            return !this->operator ==(rhs);
        }

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };
    /**
    * Info of contact.
    * Contact handle to get info about is set via constructor.
    * Info is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, the instance of @ref InfoContact::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class InfoContact : public Util::Printable
    {
        const std::string handle_;/**< contact identifier */
        bool lock_;/**< lock object_registry row flag*/

    public:
        DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);/**< exception members for unknown contact handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_contact_handle<Exception>
        {};

        /**
        * Info contact constructor with mandatory parameter.
        * @param handle sets contact identifier into @ref handle_ attribute
        */
        InfoContact(const std::string& handle);

        /**
        * Sets contact lock flag.
        * @param lock sets lock contact flag into @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_lock(bool lock = true);

        /**
        * Executes getting info about the contact.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the contact
        */
        InfoContactOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };//class InfoContact
}//namespace Fred

#endif//INFO_CONTACT_H_
