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
 *  create domain
 */

#ifndef CREATE_DOMAIN_H_
#define CREATE_DOMAIN_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "fredlib/domain/domain_name.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"


namespace Fred
{

    /**
    * Create of domain.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the create.
    * Create is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref CreateDomain::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class CreateDomain
    {
        const std::string fqdn_;/**< fully qualified domain name */
        const std::string registrar_;/**< handle of registrar performing the create */
        Optional<std::string> authinfo_;/**< transfer password */
        const std::string registrant_;/**< registrant contact handle*/
        Optional<Nullable<std::string> > nsset_;/**< nsset handle or NULL if missing */
        Optional<Nullable<std::string> > keyset_;/**< keyset handle or NULL if missing */
        std::vector<std::string> admin_contacts_; /**< admin contact handles */
        Optional<boost::gregorian::date> expiration_date_;/**< the expiration date of the domain */
        Optional<boost::gregorian::date> enum_validation_expiration_;/**< the expiration date of the ENUM domain validation, prohibited for non-ENUM domains */
        Optional<bool> enum_publish_flag_;/**< flag for publishing ENUM number and associated contact in public directory, prohibited for non-ENUM domains */
        Nullable<unsigned long long> logd_request_id_; /**< id of the record in logger database, id is used in other calls to logging within current request */

    public:
        DECLARE_EXCEPTION_DATA(unknown_zone_fqdn, std::string);
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_registrant_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_nsset_handle, Nullable<std::string>);
        DECLARE_EXCEPTION_DATA(unknown_keyset_handle, Nullable<std::string>);
        DECLARE_EXCEPTION_DATA(invalid_fqdn_syntax, std::string);
        DECLARE_EXCEPTION_DATA(invalid_expiration_date, boost::gregorian::date);
        DECLARE_EXCEPTION_DATA(invalid_enum_validation_expiration_date, boost::gregorian::date);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_admin_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_admin_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_zone_fqdn<Exception>
        , ExceptionData_unknown_registrant_handle<Exception>
        , ExceptionData_unknown_nsset_handle<Exception>
        , ExceptionData_unknown_keyset_handle<Exception>
        , ExceptionData_vector_of_unknown_admin_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_admin_contact_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_invalid_fqdn_syntax<Exception>
        , ExceptionData_invalid_expiration_date<Exception>
        , ExceptionData_invalid_enum_validation_expiration_date<Exception>
        {};

        CreateDomain(const std::string& fqdn
                , const std::string& registrar
                , const std::string& registrant);
        CreateDomain(const std::string& fqdn
                , const std::string& registrar
                , const std::string& registrant
                , const Optional<std::string>& authinfo
                , const Optional<Nullable<std::string> >& nsset
                , const Optional<Nullable<std::string> >& keyset
                , const std::vector<std::string>& admin_contacts
                , const Optional<boost::gregorian::date>& expiration_date
                , const Optional<boost::gregorian::date>& enum_validation_expiration
                , const Optional<bool>& enum_publish_flag
                , const Optional<unsigned long long> logd_request_id);

        CreateDomain& set_authinfo(const std::string& authinfo);
        CreateDomain& set_nsset(const Nullable<std::string>& nsset);
        CreateDomain& set_nsset(const std::string& nsset);
        CreateDomain& set_keyset(const Nullable<std::string>& keyset);
        CreateDomain& set_keyset(const std::string& keyset);
        CreateDomain& set_admin_contacts(const std::vector<std::string>& admin_contacts);
        CreateDomain& set_expiration_date(const Optional<boost::gregorian::date>& expiration_date);
        CreateDomain& set_enum_validation_expiration(const boost::gregorian::date& valexdate);
        CreateDomain& set_enum_publish_flag(bool enum_publish_flag);
        CreateDomain& set_logd_request_id(unsigned long long logd_request_id);
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

        friend std::ostream& operator<<(std::ostream& os, const CreateDomain& i);
        std::string to_string();
    };//CreateDomain
}
#endif // CREATE_DOMAIN_H_
