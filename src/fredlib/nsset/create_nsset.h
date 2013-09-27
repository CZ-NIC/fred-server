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
 *  create nsset
 */

#ifndef CREATE_NSSET_H_
#define CREATE_NSSET_H_

#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

#include "fredlib/nsset/nsset_dns_host.h"

namespace Fred
{

    /**
    * Create of nsset.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the create.
    * Create is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref CreateNsset::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class CreateNsset : public Util::Printable
    {
        const std::string handle_;/**< nsset identifier */
        const std::string registrar_;/**< handle of registrar performing the create */
        Optional<std::string> authinfo_;/**< transfer password */
        Optional<short> tech_check_level_; /**< level of technical checks*/
        std::vector<DnsHost> dns_hosts_; /**< DNS hosts of the nsset */
        std::vector<std::string> tech_contacts_; /**< technical contact handles */
        Nullable<unsigned long long> logd_request_id_; /**< id of the record in logger database, id is used in other calls to logging within current request */

    public:
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_dns_host, std::string);/**< exception members for vector of duplicated DNS hosts in nsset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(invalid_dns_host_ipaddr, std::string);/**< exception members for vector of invalid DNS hosts in nsset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);/**< exception members for vector of unknown technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);/**< exception members for vector of already set technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_vector_of_unknown_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_dns_host<Exception>
        , ExceptionData_vector_of_invalid_dns_host_ipaddr<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        /**
        * Create nsset constructor with mandatory parameters.
        * @param handle sets nsset handle into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        */
        CreateNsset(const std::string& handle
                , const std::string& registrar);
        /**
        * Create nsset constructor with all parameters.
        * @param handle sets nsset handle into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @param tech_check_level sets level of technical checks into @ref tech_check_level_ attribute
        * @param dns_hosts sets DNS hosts into @ref dns_hosts_ attribute
        * @param tech_contact sets list of technical contact handles into @ref tech_contacts_ attribute
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
        CreateNsset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& authinfo
                , const Optional<short>& tech_check_level
                , const std::vector<DnsHost>& dns_hosts
                , const std::vector<std::string>& tech_contacts
                , const Optional<unsigned long long> logd_request_id
                );

        /**
        * Sets nsset transfer password.
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateNsset& set_authinfo(const std::string& authinfo);

        /**
        * Sets nsset level of technical checks.
        * @param tech_check_level sets level of technical checks into @ref tech_check_level_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateNsset& set_tech_check_level(short tech_check_level);

        /**
        * Sets nsset DNS hosts.
        * @param dns_hosts sets list of DNS hosts into @ref dns_hosts_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateNsset& set_dns_hosts(const std::vector<DnsHost>& dns_hosts);

        /**
        * Sets nsset technical contacts.
        * @param tech_contacts sets list of technical contact handles into @ref tech_contacts_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateNsset& set_tech_contacts(const std::vector<std::string>& tech_contacts);

        /**
        * Sets logger request id
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateNsset& set_logd_request_id(unsigned long long logd_request_id);

        /**
        * Executes create
        * @param ctx contains reference to database and logging interface
        * @param returned_timestamp_pg_time_zone_name is postgresql time zone name of the returned timestamp
        * @return timestamp of the contact creation
        */
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };//CreateNsset

}
#endif // CREATE_NSSET_H_
