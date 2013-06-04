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
 *  @file info_keyset.h
 *  keyset info
 */

#ifndef INFO_KEYSET_H_
#define INFO_KEYSET_H_

#include <string>
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/keyset/info_keyset_data.h"


namespace Fred
{
    struct InfoKeysetOutput
    {
        InfoKeysetData info_keyset_data;//common info keyset data
        boost::posix_time::ptime utc_timestamp;//utc timestamp
        boost::posix_time::ptime local_timestamp;//local zone timestamp

        InfoKeysetOutput()
        {}

        bool operator==(const InfoKeysetOutput& rhs) const
        {
            return info_keyset_data == rhs.info_keyset_data;
        }

        bool operator!=(const InfoKeysetOutput& rhs) const
        {
            return !this->operator ==(rhs);
        }

    };

    class InfoKeyset
    {
        const std::string handle_;//keyset identifier
        const std::string registrar_;//registrar identifier
        bool lock_;//lock object_registry row

    public:

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_keyset_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};


        InfoKeyset(const std::string& handle
                , const std::string& registrar);
        InfoKeyset& set_lock(bool lock = true);//set lock object_registry row
        InfoKeysetOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        friend std::ostream& operator<<(std::ostream& os, const InfoKeyset& i);
        std::string to_string();

    };//class InfoKeyset

}//namespace Fred

#endif//INFO_KEYSET_H_
