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
 *  create keyset
 */

#ifndef CREATE_KEYSET_H_
#define CREATE_KEYSET_H_

#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/keyset/keyset_dns_key.h"

namespace Fred
{

    /**
    * Create of keyset.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the create.
    * Create is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref CreateKeyset::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class CreateKeyset
    {
        const std::string handle_;/**< keyset identifier */
        const std::string registrar_;/**< handle of registrar performing the create */
        Optional<std::string> authinfo_;/**< transfer password */
        std::vector<DnsKey> dns_keys_; /**< list of DNS keys */
        std::vector<std::string> tech_contacts_; /**< technical contact handles */
        Nullable<unsigned long long> logd_request_id_; /**< id of the record in logger database, id is used in other calls to logging within current request */

    public:
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_dns_key, DnsKey);/**< exception members for vector of duplicated keys in keyset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);/**< exception members for vector of unknown technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);/**< exception members for vector of already set technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_vector_of_unknown_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_dns_key<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        /**
        * Create keyset constructor with mandatory parameters.
        * @param handle sets keyset handle into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        */
        CreateKeyset(const std::string& handle
                , const std::string& registrar);

        /**
        * Create keyset constructor with all parameters.
        * @param handle sets keyset handle into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @param dns_keys sets DNS keys into @ref dns_keys_ attribute
        * @param tech_contact sets list of technical contact handles into @ref tech_contacts_ attribute
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
        CreateKeyset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& authinfo
                , const std::vector<DnsKey>& dns_keys
                , const std::vector<std::string>& tech_contacts
                , const Optional<unsigned long long> logd_request_id
                );

        /**
        * Sets keyset transfer password.
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateKeyset& set_authinfo(const std::string& authinfo);

        /**
        * Sets keyset DNS keys.
        * @param dns_keys sets DNS keys into @ref dns_keys_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateKeyset& set_dns_keys(const std::vector<DnsKey>& dns_keys);

        /**
        * Sets keyset technical contacts.
        * @param tech_contacts sets list of technical contact handles into @ref tech_contacts_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateKeyset& set_tech_contacts(const std::vector<std::string>& tech_contacts);

        /**
        * Sets logger request id
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateKeyset& set_logd_request_id(unsigned long long logd_request_id);

        /**
        * Executes create
        * @param ctx contains reference to database and logging interface
        * @param returned_timestamp_pg_time_zone_name is postgresql time zone name of the returned timestamp
        * @return timestamp of the contact creation
        */
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into stream
        * @param os contains output stream reference
        * @param i reference of instance to be dumped into the stream
        * @return output stream reference
        */
        friend std::ostream& operator<<(std::ostream& os, const CreateKeyset& i);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string();
    };//CreateKeyset
}
#endif // CREATE_KEYSET_H_
