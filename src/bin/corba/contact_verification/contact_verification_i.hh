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
*  @contact_verification_i.h
*  header of contact verification corba wrapper
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace/enum/idl/idl/ContactVerification.idl
*/

#ifndef CONTACT_VERIFICATION_I_HH_634B136D452B45E7B6E9AFD756BB28BD
#define CONTACT_VERIFICATION_I_HH_634B136D452B45E7B6E9AFD756BB28BD

#include "src/bin/corba/ContactVerification.hh"
#include <memory>
#include <string>

namespace Fred {
namespace Backend {
namespace ContactVerification {

class ContactVerificationImpl;

} // namespace Fred::Backend::ContactVerification
} // namespace Fred::Backend
} // namespace Fred

namespace CorbaConversion {
namespace Contact {
namespace Verification {

class ContactVerification_i : public POA_Registry::ContactVerification
{
private:
    // do not copy
    const std::unique_ptr<Fred::Backend::ContactVerification::ContactVerificationImpl> pimpl_;
    ContactVerification_i(const ContactVerification_i&); //no body
    ContactVerification_i& operator=(const ContactVerification_i&); //no body

public:
    // standard constructor
    ContactVerification_i(const std::string& _server_name);
    virtual ~ContactVerification_i();

    // methods corresponding to defined IDL attributes and operations
    ::CORBA::ULongLong createConditionalIdentification(
            const char* contact_handle,
            const char* registrar_handle,
            ::CORBA::ULongLong log_id,
            ::CORBA::String_out request_id);

    ::CORBA::ULongLong processConditionalIdentification(
            const char* request_id,
            const char* password,
            ::CORBA::ULongLong log_id);

    ::CORBA::ULongLong processIdentification(
            const char* contact_handle,
            const char* password,
            ::CORBA::ULongLong log_id);

    char* getRegistrarName(const char* registrar_handle);

}; //class ContactVerification_i
} // namespace CorbaConversion::Contact::Verification
} // namespace CorbaConversion::Contact
} // namespace CorbaConversion

#endif //CONTACT_VERIFICATION_I_H__

