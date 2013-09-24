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
 *  keyset update
 */

#ifndef UPDATE_KEYSET_H_
#define UPDATE_KEYSET_H_

#include <string>
#include <vector>

#include "fredlib/keyset/keyset_dns_key.h"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"


namespace Fred
{

    /**
    * Update of keyset.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the update.
    * Update is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref UpdateKeyset::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class UpdateKeyset : public Util::Printable
    {
        const std::string handle_;/**< keyset identifier */
        const std::string registrar_;/**< handle of registrar performing the update */
        Optional<std::string> sponsoring_registrar_;/**< handle of registrar administering the object */
        Optional<std::string> authinfo_;/**< transfer password */
        std::vector<std::string> add_tech_contact_; /**< technical contact handles to be added */
        std::vector<std::string> rem_tech_contact_; /**< technical contact handles to be removed */
        std::vector<DnsKey> add_dns_key_; /**< DNS keys to be added */
        std::vector<DnsKey> rem_dns_key_; /**< DNS keys to be removed */
        Nullable<unsigned long long> logd_request_id_; /**< id of the record in logger database, id is used in other calls to logging within current request */

    public:
        DECLARE_EXCEPTION_DATA(unknown_keyset_handle, std::string);/**< exception members for unknown keyset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_sponsoring_registrar_handle, std::string);/**< exception members for unknown sponsoring registrar of the keyset generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);/**< exception members for vector of unknown technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_dns_key, DnsKey);/**< exception members for vector of duplicated keys that wasn't added to keyset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_dns_key, DnsKey);/**< exception members for vector of unassigned keys that wasn't removed from keyset generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);/**< exception members for vector of already set technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_technical_contact_handle, std::string);/**< exception members for vector of unknown technical contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_keyset_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_unknown_sponsoring_registrar_handle<Exception>
        , ExceptionData_vector_of_unknown_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_technical_contact_handle<Exception>
        , ExceptionData_vector_of_unassigned_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_dns_key<Exception>
        , ExceptionData_vector_of_unassigned_dns_key<Exception>
        {};

        /**
        * Update keyset constructor with mandatory parameters.
        * @param handle sets keyset identifier into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        */
        UpdateKeyset(const std::string& handle
                , const std::string& registrar);

        /**
        * Update keyset constructor with mandatory parameters.
        * @param handle sets keyset identifier into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        * @param sponsoring_registrar sets sponsoring registrar handle into @ref sponsoring_registrar_ attribute
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @param add_tech_contact sets list of technical contact handles to be added into @ref add_tech_contact_ attribute
        * @param rem_tech_contact sets list of technical contact handles to be removed into @ref rem_tech_contact_ attribute
        * @param add_dns_key sets list of DNS keys to be added into @ref add_dns_key_ attribute
        * @param rem_dns_key sets list of DNS keys to be removed into @ref rem_dns_key_ attribute
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
        UpdateKeyset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& sponsoring_registrar
                , const Optional<std::string>& authinfo
                , const std::vector<std::string>& add_tech_contact
                , const std::vector<std::string>& rem_tech_contact
                , const std::vector<DnsKey>& add_dns_key
                , const std::vector<DnsKey>& rem_dns_key
                , const Optional<unsigned long long> logd_request_id
                );

        /**
        * Sets keyset sponsoring registrar.
        * @param sponsoring_registrar sets registrar administering the keyset into @ref sponsoring_registrar_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateKeyset& set_sponsoring_registrar(const std::string& sponsoring_registrar);

        /**
        * Sets keyset transfer password.
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateKeyset& set_authinfo(const std::string& authinfo);

        /**
        * Adds technical contact handle.
        * @param tech_contact adds technical contact handle to be added into @ref add_tech_contact_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateKeyset& add_tech_contact(const std::string& tech_contact);

        /**
        * Removes technical contact handle.
        * @param tech_contact adds technical contact handle to be removed into @ref rem_tech_contact_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateKeyset& rem_tech_contact(const std::string& tech_contact);

        /**
        * Adds DNS key.
        * @param dns_key adds DNS key instance to be added into @ref add_dns_key_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateKeyset& add_dns_key(const DnsKey& dns_key);

        /**
        * Removes DNS key.
        * @param dns_key adds DNS key instance to be removed into @ref rem_dns_key_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateKeyset& rem_dns_key(const DnsKey& dns_key);

        /**
        * Sets logger request id.
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateKeyset& set_logd_request_id(unsigned long long logd_request_id);

        /**
        * Executes update.
        * @param ctx contains reference to database and logging interface
        * @return new history_id
        */
        unsigned long long exec(OperationContext& ctx);

        /**
        * Dumps state of the instance into the string.
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };//class UpdateKeyset

}//namespace Fred

#endif//UPDATE_KEYSET_H_
