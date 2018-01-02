/* * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
*  @server_i.h
*  header of public request corba wrapper
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace/idl/idl/PublicRequest.idl
*/

#ifndef SERVER_I_HH_4C0DD51687BF47B19E2078C31BF6AA93
#define SERVER_I_HH_4C0DD51687BF47B19E2078C31BF6AA93

#include "src/bin/corba/PublicRequest.hh"

#include <memory>
#include <string>

namespace Registry {

class PublicRequestImpl;

namespace PublicRequest {

class Server_i : public POA_Registry::PublicRequest::PublicRequestIntf
{
public:
    Server_i(const std::string& _server_name);

    virtual ~Server_i();

    ::CORBA::ULongLong create_authinfo_request_registry_email(
        ObjectType_PR::Type object_type,
        const char* object_handle,
        NullableULongLong* log_request_id);

    ::CORBA::ULongLong create_authinfo_request_non_registry_email(
        ObjectType_PR::Type object_type,
        const char* object_handle,
        NullableULongLong* log_request_id,
        ConfirmedBy::Type confirmation_method,
        const char* specified_email);

    ::CORBA::ULongLong create_block_unblock_request(
        ObjectType_PR::Type object_type,
        const char* object_handle,
        NullableULongLong* log_request_id,
        ConfirmedBy::Type confirmation_method,
        LockRequestType::Type lock_request_type);

    Buffer* create_public_request_pdf(CORBA::ULongLong public_request_id, Language::Type lang);

private:
    const std::unique_ptr<PublicRequestImpl> pimpl_;

    Server_i(const Server_i&); // no body
    Server_i& operator= (const Server_i&); // no body
};

} // namespace Registry::PublicRequest
} // namespace Registry

#endif
