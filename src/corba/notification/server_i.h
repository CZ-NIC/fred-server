/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
*  @file
*  header of notification wrapper over corba
*/
#ifndef CORBA_NOTIFICATION_SERVER_I_H_
#define CORBA_NOTIFICATION_SERVER_I_H_

#include <string>

#include "src/corba/Notification.hh"

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
