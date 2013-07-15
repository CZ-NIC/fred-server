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
 *  @file info_domain.h
 *  domain info
 */

#ifndef INFO_DOMAIN_H_
#define INFO_DOMAIN_H_

#include <string>
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/domain/info_domain_data.h"

namespace Fred
{
    struct InfoDomainOutput
    {
        InfoDomainData info_domain_data;//common info domain data
        boost::posix_time::ptime utc_timestamp;// utc timestamp
        boost::posix_time::ptime local_timestamp;//local zone timestamp

        InfoDomainOutput()
        {}

        bool operator==(const InfoDomainOutput& rhs) const
        {
            return info_domain_data == rhs.info_domain_data;
        }

        bool operator!=(const InfoDomainOutput& rhs) const
        {
            return !this->operator ==(rhs);
        }

    };

    class InfoDomain
    {
        const std::string fqdn_;//domain identifier
        const std::string registrar_;//registrar identifier
        bool lock_;//lock object_registry row for domain

    public:
        InfoDomain(const std::string& fqdn
                , const std::string& registrar);
        InfoDomain& set_lock(bool lock = true);//set lock object_registry row for domain
        InfoDomainOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data
    };//class InfoDomain

//exception impl
    class InfoDomainException
    : public OperationExceptionImpl<InfoDomainException, 8192>
    {
    public:
        InfoDomainException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<InfoDomainException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={
                    "not found:fqdn"
                    , "not found:registrar"
            };
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class InfoDomainException

    typedef InfoDomainException::OperationErrorType InfoDomainError;
#define IDEX(DATA) InfoDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define IDERR(DATA) InfoDomainError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred

#endif//INFO_DOMAIN_H_
