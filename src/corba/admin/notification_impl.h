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
*  @file
*  header of notification wrapper over corba
*/
#ifndef NOTIFICATION_IMPL_H_
#define NOTIFICATION_IMPL_H_

#include "src/corba/Notification.hh"

#include <string>

namespace Registry {

    namespace Notification {

        /**
         * Wrapper class for CORBA \c NotificationIface methods implementation
         * Implements all virtual methods of POA_Registry::Notification::NotificationIface
         */
        class Notification_i : public POA_Registry::Notification::NotificationIface {

            public:

                virtual ~Notification_i() {};


                /**
                 * Wrapper for IDL NotificationIface method of the same name
                 */
                DomainEmailSeq *notify_outzoneunguarded_domain_email_list(const DomainEmailSeq &domain_email_seq);

        };

    }

}
#endif

/* vim: set et sw=4 : */
