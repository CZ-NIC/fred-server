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
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request

    public:
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
        UpdateDomain& set_logd_request_id(unsigned long long logd_request_id);
        void exec(OperationContext& ctx);
    };//class UpdateDomain

    struct UpdateDomainFailParam
    {
        ConstArr get_fail_param()
        {
            static const char* list[]={"param1", "param2", "param3", "param4", "param5"};
            ConstArr ca;
            ca.arr = list;
            ca.size = sizeof(list)/sizeof(char*);//number of elements in the list
            return ca;
        }
    protected:
        ~UpdateDomainFailParam()throw(){}
    };

    struct UpdateDomainFailReason
    {
        ConstArr get_fail_reason()
        {
            static const char* list[]={"reason1", "reason2", "reason3", "reason4", "reason5"};
            ConstArr ca;
            ca.arr = list;
            ca.size = sizeof(list)/sizeof(char*);//number of elements in the list
            return ca;
        }
    protected:
        ~UpdateDomainFailReason()throw(){}
    };

    ///operation exception base class
    struct UpdateDomainOperationException
    : virtual public std::exception  //common base
    {
        virtual const char* what() const throw() = 0;
        virtual ~UpdateDomainOperationException() throw() {};
    };

    typedef OperationException<2048,UpdateDomainOperationException
        ,UpdateDomainFailParam,UpdateDomainFailReason> UpdateDomainException;
    #define UDEX(DATA) UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred

#endif//UPDATE_DOMAIN_H_
