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
 *  @file notification.h
 *  corba server implementation of registry notification
 */

#ifndef ADMIN_NOTIFICATION_NOTIFICATION_H_
#define ADMIN_NOTIFICATION_NOTIFICATION_H_

#include <boost/format.hpp>

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"

// CORBA stub for Notification.idl
#include "src/corba/Notification.hh"

namespace Admin {

    namespace Notification {

        /// Exception for internal error
        struct InternalError : std::exception {
            const char* what() const throw () {
                return "internal error";
            }
        };

        /// Exception for case when some data are not valid
        struct DomainEmailValidationError : std::exception {
            std::vector<std::pair<unsigned long long, std::string> > invalid_domain_email_list;
            DomainEmailValidationError(std::vector<std::pair<unsigned long long, std::string> > invalid_domain_email_list) : invalid_domain_email_list(invalid_domain_email_list) {};
            ~DomainEmailValidationError() throw () {};
            const char* what() const throw () {
                return "some data are invalid";
            }
        };

        /**
         * Import list of additional emails used to notify \c outboundUnguardedWarning domain state
         *
         * \param domain_email_list  list of pairs <domain_id, email>
         *
         * \throw InternalError               in case of unexpected failure
         * \throw DomainEmailValidationError  in case of invalid input
         */
        void notify_outzone_unguarded_domain_email_list(
            const std::vector<std::pair<unsigned long long, std::string> > &domain_email_list
        );

    }

}

#endif

/* vim: set et sw=4 : */

