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
 *  @file update_domain.h
 *  domain update
 */

#ifndef UPDATE_DOMAIN_H_
#define UPDATE_DOMAIN_H_

#include <string>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

namespace Fred
{
    class UpdateDomain
    {
        const std::string fqdn_;//domain identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> registrant_;//set registrant
        Optional<std::string> authinfo_;//set authinfo
        Optional<Nullable<std::string> > nsset_;//set nsset to NULL or value
        Optional<Nullable<std::string> > keyset_;//set keyset
        std::vector<std::string> add_admin_contact_; //admin contacts to be added
        std::vector<std::string> rem_admin_contact_; //admin contacts to be removed
        Optional<boost::gregorian::date> expiration_date_;/**< the expiration date of the domain */
        Optional<boost::gregorian::date> enum_validation_expiration_;//the expiration date of the ENUM domain validation, have to be set for enum domain, otherwise unused
        Optional<bool> enum_publish_flag_;//flag for publishing ENUM number and associated contact in public directory
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request

    public:
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_admin_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_admin_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_admin_contact_handle, std::string);
        DECLARE_EXCEPTION_DATA(invalid_expiration_date, boost::gregorian::date);/**< exception members for invalid expiration date of the domain */

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_domain_fqdn<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_unknown_nsset_handle<Exception>
        , ExceptionData_unknown_keyset_handle<Exception>
        , ExceptionData_unknown_registrant_handle<Exception>
        , ExceptionData_vector_of_unknown_admin_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_admin_contact_handle<Exception>
        , ExceptionData_vector_of_unassigned_admin_contact_handle<Exception>
        , ExceptionData_invalid_expiration_date<Exception>
        {};

        UpdateDomain(const std::string& fqdn
                , const std::string& registrar);
        UpdateDomain(const std::string& fqdn
            , const std::string& registrar
            , const Optional<std::string>& registrant
            , const Optional<std::string>& authinfo
            , const Optional<Nullable<std::string> >& nsset
            , const Optional<Nullable<std::string> >& keyset
            , const std::vector<std::string>& add_admin_contact
            , const std::vector<std::string>& rem_admin_contact
            , const Optional<boost::gregorian::date>& enum_validation_expiration
            , const Optional<bool>& enum_publish_flag
            , const Optional<unsigned long long> logd_request_id
            );
        UpdateDomain& set_registrant(const std::string& registrant);
        UpdateDomain& set_authinfo(const std::string& authinfo);
        UpdateDomain& set_nsset(const Nullable<std::string>& nsset);
        UpdateDomain& set_nsset(const std::string& nsset);
        UpdateDomain& unset_nsset();
        UpdateDomain& set_keyset(const Nullable<std::string>& keyset);
        UpdateDomain& set_keyset(const std::string& keyset);
        UpdateDomain& unset_keyset();
        UpdateDomain& add_admin_contact(const std::string& admin_contact);
        UpdateDomain& rem_admin_contact(const std::string& admin_contact);

        /**
        * Set domain expiration date.
        * @param exdate sets domain expiration date into @ref expiration_date_ attribute
        */
        UpdateDomain& set_domain_expiration(const boost::gregorian::date& exdate);

        UpdateDomain& set_enum_validation_expiration(const boost::gregorian::date& valexdate);
        UpdateDomain& set_enum_publish_flag(bool enum_publish_flag);
        UpdateDomain& set_logd_request_id(unsigned long long logd_request_id);
        unsigned long long exec(OperationContext& ctx);//return new history_id
        friend std::ostream& operator<<(std::ostream& os, const UpdateDomain& i);
        std::string to_string();
    };//class UpdateDomain

}//namespace Fred

#endif//UPDATE_DOMAIN_H_
