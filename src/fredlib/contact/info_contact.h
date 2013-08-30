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
 *  @file info_contact.h
 *  contact info
 */

#ifndef INFO_CONTACT_H_
#define INFO_CONTACT_H_

#include <string>
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/contact/info_contact_data.h"


namespace Fred
{
    struct InfoContactOutput
    {
        InfoContactData info_contact_data;//common info keyset data
        boost::posix_time::ptime utc_timestamp;//utc timestamp
        boost::posix_time::ptime local_timestamp;//local zone timestamp

        InfoContactOutput()
        {}

        bool operator==(const InfoContactOutput& rhs) const
        {
            return info_contact_data == rhs.info_contact_data;
        }

        bool operator!=(const InfoContactOutput& rhs) const
        {
            return !this->operator ==(rhs);
        }

    };

    class InfoContact
    {
        const std::string handle_;//keyset identifier
        bool lock_;//lock object_registry row

    public:
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_contact_handle<Exception>
        {};

        InfoContact(const std::string& handle);
        InfoContact& set_lock(bool lock = true);//set lock object_registry row
        InfoContactOutput exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data

        friend std::ostream& operator<<(std::ostream& os, const InfoContact& ic);
        std::string to_string();
    };//class InfoContact
}//namespace Fred

#endif//INFO_CONTACT_H_
