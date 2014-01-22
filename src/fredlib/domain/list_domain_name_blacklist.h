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
 *  @file list_domain_name_blacklist.h
 *  list domain name blacklist
 */

#ifndef LIST_DOMAIN_NAME_BLACKLIST_H_
#define LIST_DOMAIN_NAME_BLACKLIST_H_

#include "create_domain_name_blacklist.h"
#include <vector>

namespace Fred
{

    class DomainNameBlacklistItem
    {
    public:
        typedef boost::posix_time::ptime Time;
        typedef unsigned long long Id;
        DomainNameBlacklistItem() {}
        DomainNameBlacklistItem(Id _id, const std::string &_domain, const Time &_valid_from,
            const Optional< Time > &_valid_to, const std::string &_reason);
        DomainNameBlacklistItem(const DomainNameBlacklistItem &_src);
        ~DomainNameBlacklistItem() {}
        DomainNameBlacklistItem& operator=(const DomainNameBlacklistItem &_src);
        Id get_id() const { return id_; }
        const std::string& get_domain() const { return domain_; }
        const Time& get_valid_from() const { return valid_from_; }
        const Optional< Time >& get_valid_to() const { return valid_to_; }
        const std::string& get_reason() const { return reason_; }
    private:
        Id id_;
        std::string domain_;
        Time valid_from_;
        Optional< Time > valid_to_;
        std::string reason_;
    };

    typedef std::vector< DomainNameBlacklistItem > DomainNameBlacklist;

    class ListDomainNameBlacklist
    {
    public:
        ListDomainNameBlacklist();
        DomainNameBlacklist& exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(domain_not_found, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_domain_not_found<Exception>
        {};
    private:
        DomainNameBlacklist blacklist_;
    };//class ListDomainNameBlacklist


}//namespace Fred

#endif//LIST_DOMAIN_NAME_BLACKLIST_H_
