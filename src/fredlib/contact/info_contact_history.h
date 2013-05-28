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
 *  @file info_contact_history.h
 *  contact history info
 */

#ifndef INFO_CONTACT_HISTORY_H_
#define INFO_CONTACT_HISTORY_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "fredlib/contact/info_contact_data.h"

namespace Fred
{

    struct InfoContactHistoryOutput
    {
        InfoContactData info_contact_data;//common info conatct data

        Nullable<unsigned long long> next_historyid; //next historyid
        boost::posix_time::ptime history_valid_from;//history valid from time
        Nullable<boost::posix_time::ptime> history_valid_to;//history valid to time, null means open end
        Nullable<unsigned long long> logd_request_id; //logd.request_id

        InfoContactHistoryOutput()
        {}
    };

    class InfoContactHistory
    {
        const std::string roid_;//contact identifier
        Optional<boost::posix_time::ptime> history_timestamp_;//history timestamp
        const std::string registrar_;//registrar identifier
        bool lock_;//lock object_registry row for contact

    public:
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_registry_object_identifier<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        InfoContactHistory(const std::string& roid, const std::string& registrar);
        InfoContactHistory(const std::string& roid, const Optional<boost::posix_time::ptime>& history_timestamp, const std::string& registrar);

        InfoContactHistory& set_history_timestamp(boost::posix_time::ptime history_timestamp);//set history timestamp
        InfoContactHistory& set_lock(bool lock = true);//set lock object_registry row for contact
        std::vector<InfoContactHistoryOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data
        friend std::ostream& operator<<(std::ostream& os, const InfoContactHistory& ich);
        std::string to_string();

    };//class InfoContactHistory

//exception impl
    class InfoContactHistoryException
    : public OperationExceptionImpl<InfoContactHistoryException, 8192>
    {
    public:
        InfoContactHistoryException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<InfoContactHistoryException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={
                    "not found:roid"
                    , "not found:registrar"
            };
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class InfoContactHistoryException

    typedef InfoContactHistoryException::OperationErrorType InfoContactHistoryError;

}//namespace Fred

#endif//INFO_CONTACT_HISTORY_H_
