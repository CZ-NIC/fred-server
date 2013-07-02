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
 *  @file update_nsset.h
 *  nsset update
 */

#ifndef UPDATE_NSSET_H_
#define UPDATE_NSSET_H_

#include <string>
#include <vector>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "fredlib/nsset/nsset_dns_host.h"

namespace Fred
{

    class UpdateNsset
    {
        const std::string handle_;//nsset identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> authinfo_;//set authinfo
        std::vector<DnsHost> add_dns_;//dns hosts to add
        std::vector<std::string> rem_dns_;//dns hosts to remove
        std::vector<std::string> add_tech_contact_; //tech contacts to be added
        std::vector<std::string> rem_tech_contact_; //tech contacts to be removed
        Optional<short> tech_check_level_; //nsset tech check level
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request

    public:
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_dns_host, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(invalid_dns_host_ipaddr, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_technical_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unassigned_dns_host, std::string);

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_nsset_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_vector_of_unknown_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_technical_contact_handle<Exception>
        , ExceptionData_vector_of_unassigned_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_dns_host<Exception>
        , ExceptionData_vector_of_unassigned_dns_host<Exception>
        , ExceptionData_vector_of_invalid_dns_host_ipaddr<Exception>
        {};

        UpdateNsset(const std::string& handle
                , const std::string& registrar);
        UpdateNsset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& authinfo
                , const std::vector<DnsHost>& add_dns
                , const std::vector<std::string>& rem_dns
                , const std::vector<std::string>& add_tech_contact
                , const std::vector<std::string>& rem_tech_contact
                , const Optional<short>& tech_check_level
                , const Optional<unsigned long long> logd_request_id
                );
        UpdateNsset& set_authinfo(const std::string& authinfo);
        UpdateNsset& add_dns(const DnsHost& dns);
        UpdateNsset& rem_dns(const std::string& fqdn);
        UpdateNsset& add_tech_contact(const std::string& tech_contact);
        UpdateNsset& rem_tech_contact(const std::string& tech_contact);
        UpdateNsset& set_tech_check_level(short tech_check_level);
        UpdateNsset& set_logd_request_id(unsigned long long logd_request_id);
        unsigned long long exec(OperationContext& ctx);//return new history_id

        friend std::ostream& operator<<(std::ostream& os, const UpdateNsset& i);
        std::string to_string();
    };//class UpdateNsset

}//namespace Fred

#endif//UPDATE_NSSET_H_
