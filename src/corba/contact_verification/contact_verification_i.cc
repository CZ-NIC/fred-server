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
*  @contact_verification_i.cc
*  implementation of contact verification corba interface
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/ContactVerification.idl
*/

#include "contact_verification_i.h"
#include "corba_wrapper_decl.h"
#include "contact_verification/contact_verification_impl.h"
#include "mailer_manager.h"
#include <ContactVerification.hh>
#include <string>
#include "contact_verification/corba_conversion.h"


namespace Registry
{
    namespace Contact
    {
        namespace Verification
        {

            ContactVerification_i::ContactVerification_i(const std::string &_server_name)
            : pimpl_(new ContactVerificationImpl(_server_name
                    , boost::shared_ptr<Fred::Mailer::Manager>(
                        new MailerManager(CorbaContainer::get_instance()
                        ->getNS()))))
            {}

            ContactVerification_i::~ContactVerification_i()
            {}


            // methods corresponding to defined IDL attributes and operations
            ::CORBA::ULongLong ContactVerification_i::createConditionalIdentification(
                    const char* contact_handle
                    , const char* registrar_handle
                    , ::CORBA::ULongLong log_id
                    , ::CORBA::String_out request_id)
            {
                try
                {
                    return 0;

                }//try
                catch (Registry::Contact::Verification::OBJECT_NOT_EXISTS&)
                {
                    throw Registry::ContactVerification::OBJECT_NOT_EXISTS();
                }
                catch (Fred::Contact::Verification::DataValidationError &_ex)
                {
                    throw Registry::ContactVerification::DATA_VALIDATION_ERROR(
                        corba_wrap_validation_error_list(_ex.errors));
                }
                catch (std::exception &_ex)
                {
                    throw Registry::ContactVerification::INTERNAL_SERVER_ERROR(_ex.what());
                }
                catch (...)
                {
                    throw Registry::ContactVerification::INTERNAL_SERVER_ERROR();
                }
            }

            ::CORBA::ULongLong ContactVerification_i::processConditionalIdentification(
                    const char* request_id
                    , const char* password
                    , ::CORBA::ULongLong log_id)
            {
                return 0;
            }

            ::CORBA::ULongLong ContactVerification_i::processIdentification(
                    const char* contact_handle
                    , const char* password
                    , ::CORBA::ULongLong log_id)
            {
                return 0;
            }

            char* ContactVerification_i::getRegistrarName(
                    const char* registrar_handle)
            {
                return CORBA::string_dup("registarr name");
            }
        }
    }
}

