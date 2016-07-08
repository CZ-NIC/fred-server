/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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
 *  <++>
 */

#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include <boost/format.hpp>

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"

// CORBA stub for Notification.idl
#include "src/corba/Notification.hh"

namespace Admin {

    namespace Notification {

        /// Exception for internal error
        struct INTERNAL_ERROR : std::exception {
            const char* what() const throw() {
                return "INTERNAL ERROR";
            }
        };

        /// Exception for invalid input OBSOLETE
        //struct VALUE_ERROR : std::exception {
        //private:
        //    long int lineno;
        //public:
        //    VALUE_ERROR(long int lineno) : lineno(lineno) {};
        //    const char* what() const {
        //        boost::format retval("Error: Importing record #%1% failed.");
        //        retval % lineno;
        //        return retval.str().c_str();
        //    }
        //};

        //typedef std::vector<std::pair<unsigned long long, std::string> > DomainEmailList;

        std::vector<std::pair<unsigned long long, std::string> >
        notify_outzoneunguarded_domain_email_list(
            const std::vector<std::pair<unsigned long long, std::string> > &domain_email_list
        );

    }

}

#endif

/* vim: set et sw=4 : */

