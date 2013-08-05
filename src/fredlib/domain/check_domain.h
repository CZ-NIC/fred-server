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
 *  @file check_domain.h
 *  domain check
 */

#ifndef CHECK_DOMAIN_H
#define CHECK_DOMAIN_H

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "fredlib/zone/zone.h"

namespace Fred
{

    class CheckDomain
    {
        const std::string fqdn_;//domain identifier
    public:

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_domain_fqdn<Exception>
        {};

        CheckDomain(const std::string& fqdn);
        //check domain name syntax and zone
        bool is_invalid_handle(OperationContext& ctx);
        //check if domain have existing zone
        bool is_bad_zone(OperationContext& ctx);
        //check number of domain name labels
        bool is_bad_length(OperationContext& ctx);
        //check if domain name is on blacklist
        bool is_blacklisted(OperationContext& ctx);
        //check if domain name is registered, if true then set conflicting_fqdn
        bool is_registered(OperationContext& ctx, std::string& conflicting_fqdn_out);

        friend std::ostream& operator<<(std::ostream& os, const CheckDomain& i);
        std::string to_string();
    };//class CheckDomain

}//namespace Fred

#endif//CHECK_DOMAIN_H
