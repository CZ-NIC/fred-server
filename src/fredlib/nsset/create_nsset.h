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
 *  @file create_nsset.h
 *  create nsset
 */

#ifndef CREATE_NSSET_H_
#define CREATE_NSSET_H_

#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/nsset/nsset_dns_host.h"

namespace Fred
{

    class CreateNsset
    {
        const std::string handle_;//nsset identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> authinfo_;//set authinfo
        Optional<short> tech_check_level_; //nsset tech check level
        std::vector<DnsHost> dns_hosts_;//dns hosts to add
        std::vector<std::string> tech_contacts_; //tech contacts to be added
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request

    public:
        DECLARE_EXCEPTION_DATA(already_set_dns_host, std::string);
        DECLARE_EXCEPTION_DATA(invalid_dns_host_ipaddr, std::string);

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_technical_contact_handle<Exception>
        , ExceptionData_already_set_technical_contact_handle<Exception>
        , ExceptionData_already_set_dns_host<Exception>
        , ExceptionData_invalid_dns_host_ipaddr<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        CreateNsset(const std::string& handle
                , const std::string& registrar);
        CreateNsset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& authinfo
                , const Optional<short>& tech_check_level
                , const std::vector<DnsHost>& dns_hosts
                , const std::vector<std::string>& tech_contacts
                , const Optional<unsigned long long> logd_request_id
                );

        CreateNsset& set_authinfo(const std::string& authinfo);
        CreateNsset& set_tech_check_level(short tech_check_level);
        CreateNsset& set_dns_hosts(const std::vector<DnsHost>& dns_hosts);
        CreateNsset& set_tech_contacts(const std::vector<std::string>& tech_contacts);
        CreateNsset& set_logd_request_id(unsigned long long logd_request_id);
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

        friend std::ostream& operator<<(std::ostream& os, const CreateNsset& i);
        std::string to_string();
    };//CreateNsset

}
#endif // CREATE_NSSET_H_
