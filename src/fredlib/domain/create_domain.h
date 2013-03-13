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
 *  @file create_domain.h
 *  create domain
 */

#ifndef CREATE_DOMAIN_H_
#define CREATE_DOMAIN_H_

#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"


namespace Fred
{

    class CreateDomain
    {
        const std::string fqdn_;//domain identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> authinfo_;//set authinfo
        const std::string registrant_;//set registrant
        Optional<Nullable<std::string> > nsset_;//set nsset to NULL or value
        Optional<Nullable<std::string> > keyset_;//set keyset
        std::vector<std::string> admin_contacts_; //set admin contacts
        Optional<unsigned> expiration_period_;//for exdate in months
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request

    public:
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
                , const Optional<unsigned>& expiration_period
                , const Optional<unsigned long long> logd_request_id);

        CreateDomain& set_authinfo(const std::string& authinfo);
        CreateDomain& set_nsset(const Nullable<std::string>& nsset);
        CreateDomain& set_nsset(const std::string& nsset);
        CreateDomain& set_keyset(const Nullable<std::string>& keyset);
        CreateDomain& set_keyset(const std::string& keyset);
        CreateDomain& set_admin_contacts(const std::vector<std::string>& admin_contacts);
        CreateDomain& set_expiration_period(unsigned expiration_period);
        CreateDomain& set_logd_request_id(unsigned long long logd_request_id);
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

    };//CreateDomain

    //exception impl
    class CreateDomainException
    : public OperationExceptionImpl<CreateDomainException, 8192>
    {
    public:
        CreateDomainException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<CreateDomainException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={
                "not found crdate:fqdn"
                , "not found zone:fqdn"
                , "not found:registrant"
                , "not found:nsset"
                , "not found:keyset"
                , "not found:admin contact"
                , "already set:admin contact"
            };
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }

    };//class CreateDomainException

    typedef CreateDomainException::OperationErrorType CreateDomainError;
#define CDEX(DATA) CreateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define CDERR(DATA) CreateDomainError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}
#endif // CREATE_DOMAIN_H_
