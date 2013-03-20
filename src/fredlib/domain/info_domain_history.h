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
 *  @file info_domain_history.h
 *  domain history info
 */

#ifndef INFO_DOMAIN_HISTORY_H_
#define INFO_DOMAIN_HISTORY_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "fredlib/domain/enum_validation_extension.h"

namespace Fred
{

    struct InfoDomainHistoryData
    {
        std::string roid;//domain identifier
        std::string fqdn;//domain name
        Nullable<boost::posix_time::ptime> delete_time; //domain delete time
        unsigned long long historyid;//historyid
        Nullable<unsigned long long> next_historyid; //next historyid
        boost::posix_time::ptime history_valid_from;//history valid from time
        Nullable<boost::posix_time::ptime> history_valid_to;//history valid to time, null means open end
        std::string registrant_handle;//domain owner
        Nullable<std::string> nsset_handle;//nssset might not be set
        Nullable<std::string> keyset_handle;//keyset might not be set
        std::string sponsoring_registrar_handle;//registrar which have right for change
        std::string create_registrar_handle;//registrar which created domain
        Nullable<std::string> update_registrar_handle;//registrar which last time changed domain
        boost::posix_time::ptime creation_time;//time of domain creation
        Nullable<boost::posix_time::ptime> update_time; //last update time
        Nullable<boost::posix_time::ptime> transfer_time; //last transfer time
        boost::gregorian::date expiration_date; //domain expiration date
        std::string authinfopw;//password for domain transfer
        Nullable<ENUMValidationExtension > enum_domain_validation;//enum domain validation info
        boost::posix_time::ptime outzone_time; //domain outzone time
        boost::posix_time::ptime cancel_time; //domain cancel time
        std::vector<std::string> admin_contacts;//list of administrative contacts

        InfoDomainHistoryData()
        : historyid(0)
        {}
    };

    class InfoDomainHistory
    {
        const std::string roid_;//domain identifier
        Optional<boost::posix_time::ptime> history_timestamp_;//history timestamp
        const std::string registrar_;//registrar identifier
        bool lock_;//lock object_registry row for domain

    public:
        InfoDomainHistory(const std::string& roid, const std::string& registrar);
        InfoDomainHistory(const std::string& roid, const Optional<boost::posix_time::ptime>& history_timestamp, const std::string& registrar);

        InfoDomainHistory& set_history_timestamp(boost::posix_time::ptime history_timestamp);//set history timestamp
        InfoDomainHistory& set_lock(bool lock = true);//set lock object_registry row for domain
        std::vector<InfoDomainHistoryData> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return data
    };//class InfoDomainHistory

//exception impl
    class InfoDomainHistoryException
    : public OperationExceptionImpl<InfoDomainHistoryException, 8192>
    {
    public:
        InfoDomainHistoryException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<InfoDomainHistoryException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={
                    "not found:roid"
                    , "not found:registrar"
            };
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class InfoDomainHistoryException

    typedef InfoDomainHistoryException::OperationErrorType InfoDomainHistoryError;
#define IDHEX(DATA) InfoDomainHistoryException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define IDHERR(DATA) InfoDomainHistoryError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred

#endif//INFO_DOMAIN_HISTORY_H_
