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
 *  @file info_nsset.h
 *  nsset info
 */

#ifndef INFO_NSSET_H_
#define INFO_NSSET_H_

#include <string>
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/nsset/info_nsset_data.h"


namespace Fred
{
    struct InfoNssetOutput
    {
        InfoNssetData info_nsset_data;//common info nsset data
        boost::posix_time::ptime utc_timestamp;//utc timestamp
        boost::posix_time::ptime local_timestamp;//local zone timestamp

        InfoNssetOutput()
        {}

        bool operator==(const InfoNssetOutput& rhs) const
        {
            return info_nsset_data == rhs.info_nsset_data;
        }

        bool operator!=(const InfoNssetOutput& rhs) const
        {
            return !this->operator ==(rhs);
        }

    };

    class InfoNsset
    {
        const std::string handle_;//nsset identifier
        const std::string registrar_;//registrar identifier
        bool lock_;//lock object_registry row for domain

    public:
        InfoNsset(const std::string& handle
                , const std::string& registrar);
        InfoNsset& set_lock(bool lock = true);//set lock object_registry row
        InfoNssetOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data
    };//class InfoNsset

//exception impl
    class InfoNssetException
    : public OperationExceptionImpl<InfoNssetException, 8192>
    {
    public:
        InfoNssetException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<InfoNssetException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={
                    "not found:handle"
                    , "not found:registrar"
            };
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class InfoNssetException

    typedef InfoNssetException::OperationErrorType InfoNssetError;

}//namespace Fred

#endif//INFO_NSSET_H_
