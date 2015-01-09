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

namespace Fred
{

    class CancelDomainNameBlacklist
    {
    public:
        CancelDomainNameBlacklist(const std::string &_domain);
        void exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(domain_not_found, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_domain_not_found<Exception>
        {};
    private:
        const std::string domain_;
    };//class CancelDomainNameBlacklist


}//namespace Fred

#endif//CANCEL_DOMAIN_NAME_BLACKLIST_H_
