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
 *  @file update_keyset.h
 *  keyset update
 */

#ifndef UPDATE_KEYSET_H_
#define UPDATE_KEYSET_H_

#include <string>
#include <vector>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"


namespace Fred
{


    class DnsKey
    {
        unsigned short flags_;
        unsigned short protocol_;
        unsigned short alg_;
        std::string key_;//base64 encoding
    public:
        virtual ~DnsKey(){}
        DnsKey(unsigned short _flags
                , unsigned short _protocol
                , unsigned short _alg
                , const std::string& _key)
        : flags_(_flags)
        , protocol_(_protocol)
        , alg_(_alg)
        , key_(_key)
        {}

        unsigned short get_flags()
        {
            return flags_;
        }

        unsigned short get_protocol()
        {
            return protocol_;
        }

        unsigned short get_alg()
        {
            return alg_;
        }

        std::string get_key()
        {
            return key_;
        }
    };

    class UpdateKeyset
    {
        const std::string handle_;//nsset identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> authinfo_;//set authinfo
        std::vector<std::string> add_tech_contact_; //tech contacts to be added
        std::vector<std::string> rem_tech_contact_; //tech contacts to be removed
        std::vector<DnsKey> add_dns_key_; //dns keys to be added
        std::vector<DnsKey> rem_dns_key_; //dns keys to be removed
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request

    public:
        UpdateKeyset(const std::string& handle
                , const std::string& registrar);
        UpdateKeyset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& authinfo
                , const std::vector<std::string>& add_tech_contact
                , const std::vector<std::string>& rem_tech_contact
                , const std::vector<DnsKey>& add_dns_key
                , const std::vector<DnsKey>& rem_dns_key
                , const Optional<unsigned long long> logd_request_id
                );
        UpdateKeyset& set_authinfo(const std::string& authinfo);
        UpdateKeyset& add_tech_contact(const std::string& tech_contact);
        UpdateKeyset& rem_tech_contact(const std::string& tech_contact);
        UpdateKeyset& add_dns_key(const DnsKey& dns_key);
        UpdateKeyset& rem_dns_key(const DnsKey& dns_key);
        UpdateKeyset& set_logd_request_id(unsigned long long logd_request_id);
        unsigned long long exec(OperationContext& ctx);//return new history_id
    };//class UpdateKeyset

    //exception impl
    class UpdateKeysetException
    : public OperationExceptionImpl<UpdateKeysetException, 8192>
    {
    public:
        UpdateKeysetException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<UpdateKeysetException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"not found:handle", "not found:registrar", "not found:tech contact"};
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class UpdateKeysetException

    typedef UpdateKeysetException::OperationErrorType UpdateKeysetError;
#define UKEX(DATA) UpdateKeysetException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define UKERR(DATA) UpdateKeysetError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))


}//namespace Fred

#endif//UPDATE_KEYSET_H_
