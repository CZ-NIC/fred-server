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
 *  @file create_domain_name_blacklist.h
 *  create domain name blacklist
 */

#ifndef CREATE_DOMAIN_NAME_BLACKLIST_H_
#define CREATE_DOMAIN_NAME_BLACKLIST_H_

#include "fredlib/object_state/create_object_state_request.h"

namespace Fred
{

    class CreateDomainNameBlacklist
    {
    public:
        typedef boost::posix_time::ptime Time;
        CreateDomainNameBlacklist(const std::string &_domain,
            const std::string &_reason);
        CreateDomainNameBlacklist(const std::string &_domain,
            const std::string &_reason,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to);
        CreateDomainNameBlacklist& set_valid_from(const Time &_valid_from);
        CreateDomainNameBlacklist& set_valid_to(const Time &_valid_to);
        void exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(already_blacklisted_domain, std::string);
        DECLARE_EXCEPTION_DATA(out_of_turn, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_already_blacklisted_domain<Exception>,
            ExceptionData_out_of_turn<Exception>
        {};
    private:
        const std::string domain_;
        const std::string reason_;
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
    };//class CreateDomainNameBlacklist


}//namespace Fred

#endif//CREATE_DOMAIN_NAME_BLACKLIST_H_
