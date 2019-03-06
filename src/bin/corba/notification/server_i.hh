/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
*  @file
*  header of notification wrapper over corba
*/
#ifndef SERVER_I_HH_D94322D1300B46A8B37726F5DFCE45AF
#define SERVER_I_HH_D94322D1300B46A8B37726F5DFCE45AF

#include <string>

#include "src/bin/corba/Notification.hh"

namespace Registry {
namespace Notification {

/**
 * Wrapper class for CORBA \c Registry::Notification methods implementation
 * Implements all virtual methods of POA_Registry::Notification::Server
 */
class Server_i : public POA_Registry::Notification::Server {

    public:

        Server_i(const std::string &server_name);

        virtual ~Server_i() {}

        /**
         * Get server name
         * @return name for logging context
         */
        std::string get_server_name();

        /**
         * Wrapper for IDL Notification::Server interface method of the same name
         */
        void set_domain_outzone_unguarded_warning_emails(const DomainEmailSeq &domain_email_seq);

    private:

        std::string server_name;

};

} // namespace Notification
} // namespace Registry

#endif
