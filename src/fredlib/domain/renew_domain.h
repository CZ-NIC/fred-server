/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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
 *  domain renew
 */

#ifndef RENEW_DOMAIN_H_b6cdb6bc48cf4d9eaba249a46ac7f6a7
#define RENEW_DOMAIN_H_b6cdb6bc48cf4d9eaba249a46ac7f6a7

#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

namespace Fred
{
    /**
    * Renew of domain.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the renew.
    * Renew is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * In case of wrong input data or other predictable and superable failure,
    * an instance of @ref RenewDomain::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies,
    * an instance of @ref InternalError or other exception is thrown.
    */
    class RenewDomain  : public Util::Printable
    {
        const std::string fqdn_;/**< fully qualified domain name */
        const std::string registrar_;/**< handle of registrar performing the renew */
        boost::gregorian::date expiration_date_;/**< the expiration date of the domain */
        /**< the expiration date of the ENUM domain validation, prohibited for non-ENUM domains */
        Optional<boost::gregorian::date> enum_validation_expiration_;
        /**< flag for publishing ENUM number and associated contact in public directory,
         *  prohibited for non-ENUM domains */
        Optional<bool> enum_publish_flag_;
        /**< id of the record in logger database,
         *  id is used in other calls to logging within current request */
        Nullable<unsigned long long> logd_request_id_;

    public:

        /**< exception members for unknown fully qualified domain name
         *  generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_domain_fqdn, std::string);
        /**< exception members for invalid expiration date of the domain
         *  generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(invalid_expiration_date, boost::gregorian::date);
        /**< exception members for invalid ENUM validation expiration date of the domain
         *  generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(invalid_enum_validation_expiration_date, boost::gregorian::date);
        /**< exception members for unknown registrar
         *  generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_domain_fqdn<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_invalid_expiration_date<Exception>
        , ExceptionData_invalid_enum_validation_expiration_date<Exception>
        {};

        /**
        * Renew domain constructor with mandatory parameters.
        * @param fqdn sets fully qualified domain name into @ref fqdn_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        * @param expiration_date sets domain expiration date into @ref expiration_date_ attribute
        */
        RenewDomain(const std::string& fqdn,
            const std::string& registrar,
            const boost::gregorian::date& expiration_date
            );

        /**
        * Renew domain constructor with all parameters.
        * @param fqdn sets fully qualified domain name into @ref fqdn_ attribute
        * @param registrar sets handle of registrar performing the renew into @ref registrar_ attribute
        * @param expiration_date sets domain expiration date into @ref expiration_date_ attribute
        * @param enum_validation_expiration sets the expiration date of the ENUM domain validation
        *  into @ref expiration_date_ attribute, it is prohibited to set this parameter for non-ENUM domains
        * @param enum_publish_flag sets flag for publishing ENUM number and associated contact
        * in public directory into @ref enum_publish_flag_ attribute
        * , it is prohibited to set this parameter for non-ENUM domains
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
        RenewDomain(const std::string& fqdn,
            const std::string& registrar,
            const boost::gregorian::date& expiration_date,
            const Optional<boost::gregorian::date>& enum_validation_expiration,
            const Optional<bool>& enum_publish_flag,
            const Optional<unsigned long long>& logd_request_id
            );

        /**
        * Sets expiration date of the ENUM domain validation.
        * @param valexdate sets the expiration date of the ENUM domain validation
        * into @ref expiration_date_ attribute
        * , it is prohibited to set this parameter for non-ENUM domains
        * @return operation instance reference to allow method chaining
        */
        RenewDomain& set_enum_validation_expiration(const boost::gregorian::date& valexdate);

        /**
        * Sets flag for publishing ENUM number and associated contact in public directory.
        * @param enum_publish_flag sets flag for publishing ENUM number and associated contact
        * in public directory into @ref enum_publish_flag_ attribute
        * , it is prohibited to set this parameter for non-ENUM domains
        * @return operation instance reference to allow method chaining
        */
        RenewDomain& set_enum_publish_flag(bool enum_publish_flag);

        /**
        * Sets logger request id
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        RenewDomain& set_logd_request_id(unsigned long long logd_request_id);

        /**
        * Executes renew
        * @param ctx contains reference to database and logging interface
        * @return new history_id
        */
        unsigned long long exec(OperationContext& ctx);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };
}

#endif