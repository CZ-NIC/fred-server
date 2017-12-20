/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  declaration of PublicRequestAuthTypeIface class
 */

#ifndef PUBLIC_REQUEST_AUTH_TYPE_IFACE_H_F7F25DCF5675DE12A1BC3F7F86DE6750//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_AUTH_TYPE_IFACE_H_F7F25DCF5675DE12A1BC3F7F86DE6750

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/public_request_object_lock_guard.hh"

namespace LibFred {

/**
 * Common class for type of particular public request with authentication.
 */
class PublicRequestAuthTypeIface:public PublicRequestTypeIface
{
public:
    /**
     * Generate unique password for new public request authentication.
     * @param _locked_contact contact joined with public request (password can be derived from contact's authinfopw)
     * @return password
     */
    virtual std::string generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const = 0;
    /**
     * Instance pointer is publicly deletable.
     */
    virtual ~PublicRequestAuthTypeIface() { }
};

} // namespace LibFred

#endif//PUBLIC_REQUEST_AUTH_TYPE_IFACE_H_F7F25DCF5675DE12A1BC3F7F86DE6750
