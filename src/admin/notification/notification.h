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

/// Exception for case when some emails are not valid
struct DomainEmailValidationError : std::exception {
    std::map<unsigned long long, std::set<std::string> > domain_invalid_emails_map;
    DomainEmailValidationError(std::map<unsigned long long, std::set<std::string> > domain_invalid_emails_map) : domain_invalid_emails_map(domain_invalid_emails_map) {}
    ~DomainEmailValidationError() throw () {}
    const char* what() const throw () {
        return "invalid notification email or emails";
    }
};

/**
 * Import list of additional emails used to notify \c outboundUnguardedWarning domain state
 *
 * \param domain_emails_map  emails by domain_id
 *
 * \throw InternalError               in case of unexpected failure
 * \throw DomainEmailValidationError  in case of invalid input
 */
void set_domain_outzone_unguarded_warning_emails(
    const std::map<unsigned long long, std::set<std::string> > &domain_emails_map
);

} // namespace Notification
} // namespace Admin

#endif
