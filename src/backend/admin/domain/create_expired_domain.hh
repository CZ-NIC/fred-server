/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_EXPIRED_DOMAIN_HH_04C50993A8B54BE18AD42AFC55B6985C
#define CREATE_EXPIRED_DOMAIN_HH_04C50993A8B54BE18AD42AFC55B6985C

#include "src/libfred/logger_client.hh"

namespace Admin {
namespace Domain {

void
create_expired_domain(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _fqdn,
        const std::string& _registrant,
        const std::string& _cltrid,
        bool _delete_existing,
        const std::string& _registrar
        );

void
logger_create_expired_domain_close(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _result,
        const unsigned long long _req_id,
        const unsigned long long _deleted_domain_id,
        const unsigned long long _new_domain_id
        );

} // namespace Domain;
} // namespace Admin;

#endif
