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
 *  domain update
 */

#ifndef UPDATE_DOMAIN_H_
#define UPDATE_DOMAIN_H_

#include <string>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

namespace Fred
{
    /**
    * Update of domain.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the update.
    * Update is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref UpdateDomain::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class UpdateDomain  : public Util::Printable
    {
        const std::string fqdn_;/**< fully qualified domain name */
        const std::string registrar_;/**< handle of registrar performing the update */
        Optional<std::string> sponsoring_registrar_;/**< set handle of registrar administering the object */
        Optional<std::string> registrant_;/**< registrant contact handle*/
        Optional<std::string> authinfo_;/**< transfer password */
        Optional<Nullable<std::string> > nsset_;/**< nsset handle or NULL if missing */
        Optional<Nullable<std::string> > keyset_;/**< keyset handle or NULL if missing */
        std::vector<std::string> add_admin_contact_;/**< admin contact handles to be added */
        std::vector<std::string> rem_admin_contact_;/**< admin contact handles to be removed */
        Optional<boost::gregorian::date> expiration_date_;/**< the expiration date of the domain */
        Optional<boost::gregorian::date> enum_validation_expiration_;/**< the expiration date of the ENUM domain validation, prohibited for non-ENUM domains */
        Optional<bool> enum_publish_flag_;/**< flag for publishing ENUM number and associated contact in public directory, prohibited for non-ENUM domains */
        Nullable<unsigned long long> logd_request_id_;/**< id of the record in logger database, id is used in other calls to logging within current request */

    public:

        DECLARE_EXCEPTION_DATA(unknown_domain_fqdn, std::string);/**< exception members for unknown fully qualified domain name generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_nsset_handle, Nullable<std::string>); /**< exception members for unknown nsset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_keyset_handle, Nullable<std::string>);/**< exception members for unknown keyset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrant_handle, std::string);/**< exception members for unknown registrant contact handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_admin_contact_handle, std::string);/**< exception members for vector of unassigned admin contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA */
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_admin_contact_handle, std::string);/**< exception members for vector of unknown admin contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_admin_contact_handle, std::string);/**< exception members for vector of already set admin contact handles generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(invalid_expiration_date, boost::gregorian::date);/**< exception members for invalid expiration date of the domain  generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(invalid_enum_validation_expiration_date, boost::gregorian::date);/**< exception members for invalid ENUM validation expiration date of the domain  generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_sponsoring_registrar_handle, std::string);/**< exception members for unknown sponsoring registrar of the domain  generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_domain_fqdn<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_unknown_sponsoring_registrar_handle<Exception>
        , ExceptionData_unknown_nsset_handle<Exception>
        , ExceptionData_unknown_keyset_handle<Exception>
        , ExceptionData_unknown_registrant_handle<Exception>
        , ExceptionData_vector_of_unknown_admin_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_admin_contact_handle<Exception>
        , ExceptionData_vector_of_unassigned_admin_contact_handle<Exception>
        , ExceptionData_invalid_expiration_date<Exception>
        , ExceptionData_invalid_enum_validation_expiration_date<Exception>
        {};

        /**
        * Update domain constructor with mandatory parameters.
        * @param fqdn sets fully qualified domain name into @ref fqdn_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        */
        UpdateDomain(const std::string& fqdn
                , const std::string& registrar);

        /**
        * Update domain constructor with all parameters.
        * @param fqdn sets fully qualified domain name into @ref fqdn_ attribute
        * @param registrar sets handle of registrar performing the update into @ref registrar_ attribute
        * @param sponsoring_registrar sets registrar administering the domain into @ref sponsoring_registrar_ attribute
        * @param registrant sets registrant contact handle into @ref registrant_ attribute
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @param nsset sets nsset handle or NULL in case of no nsset into @ref nsset_ attribute
        * @param keyset sets keyset handle or NULL in case of no keyset into @ref keyset_ attribute
        * @param add_admin_contact sets admin contact handles to be added into @ref add_admin_contact_ attribute
        * @param rem_admin_contact sets admin contact handles to be removed into @ref rem_admin_contact_ attribute
        * @param expiration_date sets domain expiration date into @ref expiration_date_ attribute
        * @param enum_validation_expiration sets the expiration date of the ENUM domain validation into @ref expiration_date_ attribute
        * , it is prohibited to set this parameter for non-ENUM domains
        * @param enum_publish_flag sets flag for publishing ENUM number and associated contact in public directory into @ref enum_publish_flag_ attribute
        * , it is prohibited to set this parameter for non-ENUM domains
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
        UpdateDomain(const std::string& fqdn
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& registrant
            , const Optional<std::string>& authinfo
            , const Optional<Nullable<std::string> >& nsset
            , const Optional<Nullable<std::string> >& keyset
            , const std::vector<std::string>& add_admin_contact
            , const std::vector<std::string>& rem_admin_contact
            , const Optional<boost::gregorian::date>& expiration_date
            , const Optional<boost::gregorian::date>& enum_validation_expiration
            , const Optional<bool>& enum_publish_flag
            , const Optional<unsigned long long> logd_request_id
            );

        /**
        * Sets domain sponsoring registrar.
        * @param sponsoring_registrar sets registrar administering the domain into @ref sponsoring_registrar_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_sponsoring_registrar(const std::string& sponsoring_registrar);

        /**
        * Sets domain registrant.
        * @param registrant sets registrant contact handle into @ref registrant_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_registrant(const std::string& registrant);

        /**
        * Sets domain transfer password.
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_authinfo(const std::string& authinfo);

        /**
        * Sets domain nsset.
        * @param nsset sets nsset handle or NULL in case of no nsset into @ref nsset_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_nsset(const Nullable<std::string>& nsset);

        /**
        * Sets domain nsset handle.
        * @param nsset sets nsset handle into @ref nsset_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_nsset(const std::string& nsset);

        /**
        * Unset domain nsset for update.
        * @param nsset sets NULL with meaning no nsset into @ref nsset_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& unset_nsset();

        /**
        * Sets domain keyset for update.
        * @param keyset sets keyset handle or NULL in case of no keyset into @ref keyset_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_keyset(const Nullable<std::string>& keyset);

        /**
        * Sets domain keyset handle.
        * @param nsset sets keyset handle into @ref keyset_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_keyset(const std::string& keyset);

        /**
        * Unsets domain keyset for update.
        * @param keyset sets NULL with meaning no keyset into @ref keyset_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& unset_keyset();

        /**
        * Adds admin contact handle.
        * @param admin_contact sets admin contact handle to be added into @ref add_admin_contact_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& add_admin_contact(const std::string& admin_contact);

        /**
        * Removes admin contact handle.
        * @param admin_contact sets admin contact handle to be removed into @ref rem_admin_contact_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& rem_admin_contact(const std::string& admin_contact);

        /**
        * Sets domain expiration date.
        * @param exdate sets domain expiration date into @ref expiration_date_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_domain_expiration(const boost::gregorian::date& exdate);

        /**
        * Sets expiration date of the ENUM domain validation.
        * @param valexdate sets the expiration date of the ENUM domain validation into @ref expiration_date_ attribute
        * , it is prohibited to set this parameter for non-ENUM domains
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_enum_validation_expiration(const boost::gregorian::date& valexdate);

        /**
        * Sets flag for publishing ENUM number and associated contact in public directory.
        * @param enum_publish_flag sets flag for publishing ENUM number and associated contact in public directory into @ref enum_publish_flag_ attribute
        * , it is prohibited to set this parameter for non-ENUM domains
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_enum_publish_flag(bool enum_publish_flag);

        /**
        * Sets logger request id
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        UpdateDomain& set_logd_request_id(unsigned long long logd_request_id);

        /**
        * Executes update
        * @param ctx contains reference to database and logging interface
        * @return new history_id
        */
        unsigned long long exec(OperationContext& ctx);//return new history_id

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };//class UpdateDomain

}//namespace Fred

#endif//UPDATE_DOMAIN_H_
