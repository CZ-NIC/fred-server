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
 *  @file create_keyset.h
 *  create keyset
 */

#ifndef CREATE_KEYSET_H_
#define CREATE_KEYSET_H_

#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/keyset/keyset_dns_key.h"

namespace Fred
{

    class CreateKeyset
    {
        const std::string handle_;//keyset identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> authinfo_;//set authinfo
        std::vector<DnsKey> dns_keys_; //dns keys to be set
        std::vector<std::string> tech_contacts_; //tech contacts to be set
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request

    public:
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_dns_key, DnsKey);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);
        DECLARE_VECTOR_OF_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);

        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_vector_of_unknown_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_technical_contact_handle<Exception>
        , ExceptionData_vector_of_already_set_dns_key<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        CreateKeyset(const std::string& handle
                , const std::string& registrar);
        CreateKeyset(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& authinfo
                , const std::vector<DnsKey>& dns_keys
                , const std::vector<std::string>& tech_contacts
                , const Optional<unsigned long long> logd_request_id
                );

        CreateKeyset& set_authinfo(const std::string& authinfo);
        CreateKeyset& set_dns_keys(const std::vector<DnsKey>& dns_keys);
        CreateKeyset& set_tech_contacts(const std::vector<std::string>& tech_contacts);
        CreateKeyset& set_logd_request_id(unsigned long long logd_request_id);
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

        friend std::ostream& operator<<(std::ostream& os, const CreateKeyset& i);
        std::string to_string();
    };//CreateKeyset
}
#endif // CREATE_KEYSET_H_
