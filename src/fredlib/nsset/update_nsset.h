/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  nsset update
 */

#ifndef UPDATE_NSSET_H_
#define UPDATE_NSSET_H_

#include <string>
#include <vector>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"
#include "src/fredlib/nsset/nsset_dns_host.h"

namespace Fred
{

    /**
    * Update of nsset.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the update.
    * Update is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref UpdateNsset::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class UpdateNsset : public Util::Printable
    {
        const std::string handle_;/**< nsset identifier */
        const std::string registrar_;/**< handle of registrar performing the update */
        Optional<std::string> sponsoring_registrar_;/**< handle of registrar administering the object */
        Optional<std::string> authinfo_;/**< transfer password */
        std::vector<DnsHost> add_dns_;/**< DNS hosts to be added */
        std::vector<std::string> rem_dns_;/**< DNS host names to be removed */
        std::vector<std::string> add_tech_contact_; /**< technical contact handles to be added */
        std::vector<std::string> rem_tech_contact_; /**< technical contact handles to be removed */
        Optional<short> tech_check_level_; /**< level of technical checks*/
        Nullable<unsigned long long> logd_request_id_; /**< id of the record in logger database, id is used in other calls to logging within current request */

    public:
        DECLARE_EXCEPTION_DATA(unknown_nsset_handle, std::string);/**< exception members for unknown nsset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_dns_host, std::string);/**< exception members for vector of duplicated DNS hosts in nsset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(invalid_dns_host_ipaddr, std::string);/**< exception members for vector of invalid DNS hosts in nsset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);/**< exception members for vector of unknown technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);/**< exception members for vector of already set technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_technical_contact_handle, std::string);/**< exception members for vector of unknown technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_dns_host, std::string);/**< exception members for vector of unassigned DNS hosts that wasn't removed from nsset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_sponsoring_registrar_handle, std::string);/**< exception members for unknown sponsoring registrar of the nsset generated by macro @ref DECLARE_EXCEPTION_DATA*/

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_nsset_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_unknown_sponsoring_registrar_handle<Exception>
        , ExceptionData_vector_of_unknown_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_technical_contact_handle<Exception>
        , ExceptionData_vector_of_unassigned_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_dns_host<Exception>
        , ExceptionData_vector_of_unassigned_dns_host<Exception>
        , ExceptionData_vector_of_invalid_dns_host_ipaddr<Exception>
        {};

        /**
        * Update nsset constructor with mandatory parameters.
        * @param handle sets nsset identifier into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        */
        UpdateNsset(const std::string& handle
                , const std::string& registrar);

        /**
        * Update nsset constructor with mandatory parameters.
        * @param handle sets nsset identifier into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        * @param sponsoring_registrar sets sponsoring registrar handle into @ref sponsoring_registrar_ attribute
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @param add_dns sets DNS hosts to be added into @ref add_dns_ attribute
        * @param rem_dns sets DNS host names to be removed into @ref rem_dns_ attribute
        * @param add_tech_contact sets list of technical contact handles to be added into @ref add_tech_contact_ attribute
        * @param rem_tech_contact sets list of technical contact handles to be removed into @ref rem_tech_contact_ attribute
        * @param tech_check_level sets level of technical checks into @ref tech_check_level_ attribute
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
        UpdateNsset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& sponsoring_registrar
                , const Optional<std::string>& authinfo
                , const std::vector<DnsHost>& add_dns
                , const std::vector<std::string>& rem_dns
                , const std::vector<std::string>& add_tech_contact
                , const std::vector<std::string>& rem_tech_contact
                , const Optional<short>& tech_check_level
                , const Optional<unsigned long long> logd_request_id
                );

        /**
        * Sets nsset sponsoring registrar.
        * @param sponsoring_registrar sets registrar administering the nsset into @ref sponsoring_registrar_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& set_sponsoring_registrar(const std::string& sponsoring_registrar);

        /**
        * Sets nsset transfer password.
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& set_authinfo(const std::string& authinfo);

        /**
        * Adds DNS host.
        * @param dns adds DNS host to be added into @ref add_dns_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& add_dns(const DnsHost& dns);

        /**
        * Removes DNS host by name.
        * @param fqdn adds name of DNS host to be removed into @ref rem_dns_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& rem_dns(const std::string& fqdn);

        /**
        * Adds technical contact handle.
        * @param tech_contact adds technical contact handle to be added into @ref add_tech_contact_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& add_tech_contact(const std::string& tech_contact);

        /**
        * Removes technical contact handle.
        * @param tech_contact adds technical contact handle to be removed into @ref rem_tech_contact_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& rem_tech_contact(const std::string& tech_contact);

        /**
        * Sets nsset level of technical checks.
        * @param tech_check_level sets level of technical checks into @ref tech_check_level_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& set_tech_check_level(short tech_check_level);

        /**
        * Sets logger request id
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateNsset& set_logd_request_id(unsigned long long logd_request_id);

        /**
        * Executes update.
        * @param ctx contains reference to database and logging interface
        * @return new history_id
        */
        unsigned long long exec(OperationContext& ctx);//return new history_id

        /**
        * Dumps state of the instance into the string.
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };//class UpdateNsset

}//namespace Fred

#endif//UPDATE_NSSET_H_
