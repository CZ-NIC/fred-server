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
 *  @file create_domain_name_blacklist_id.h
 *  create domain name blacklist
 */

#ifndef CREATE_DOMAIN_NAME_BLACKLIST_ID_H_
#define CREATE_DOMAIN_NAME_BLACKLIST_ID_H_

#include "create_object_state_request.h"

namespace Fred
{

    typedef unsigned long long UserId;

    class CreateDomainNameBlacklistId
    {
    public:
        typedef boost::posix_time::ptime Time;
        CreateDomainNameBlacklistId(ObjectId _object_id,
            const std::string &_reason);
        CreateDomainNameBlacklistId(ObjectId _object_id,
            const std::string &_reason,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to,
            const Optional< UserId > &_creator);
        CreateDomainNameBlacklistId& set_valid_from(const Time &_valid_from);
        CreateDomainNameBlacklistId& set_valid_to(const Time &_valid_to);
        CreateDomainNameBlacklistId& set_creator(UserId _creator);
        void exec(OperationContext &_ctx);

    //exception impl
        class Exception
        : public OperationExceptionImpl< Exception, 2048 >
        {
        public:
            Exception(const char* file,
                const int line,
                const char* function,
                const char* data)
            :   OperationExceptionImpl< Exception, 2048 >(file, line, function, data)
            {}

            ConstArr get_fail_param_impl() throw()
            {
                static const char* list[] = {"out of turn:valid_from-to", "domain:already blacklisted",
                                             "creator:not found"};
                return ConstArr(list, sizeof(list) / sizeof(char*));
            }
        };//class CreateDomainNameBlacklistIdException

        typedef Exception::OperationErrorType Error;
    private:
        ObjectId object_id_;
        const std::string reason_;
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
        Optional< UserId > creator_;
    };//class CreateDomainNameBlacklistId


}//namespace Fred

#endif//CREATE_DOMAIN_NAME_BLACKLIST_H_
