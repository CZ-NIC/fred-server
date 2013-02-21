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
 *  @file cancel_domain_name_blacklist.h
 *  cancel domain name blacklist
 */

#ifndef CANCEL_DOMAIN_NAME_BLACKLIST_H_
#define CANCEL_DOMAIN_NAME_BLACKLIST_H_

#include "create_object_state_request.h"

namespace Fred
{

    class CancelDomainNameBlacklist
    {
    public:
        CancelDomainNameBlacklist(const std::string &_domain);
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
                static const char* list[] = {"domain:not found"};
                return ConstArr(list, sizeof(list) / sizeof(char*));
            }
        };//class CancelDomainNameBlacklistException

        typedef Exception::OperationErrorType Error;
    private:
        const std::string domain_;
    };//class CancelDomainNameBlacklist


}//namespace Fred

#endif//CANCEL_DOMAIN_NAME_BLACKLIST_H_
