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
 *  implementation of admin contact verification wrapper over corba
 */


#include "server_i.h"

namespace Registry
{
    namespace AdminContactVerification
    {
        //   Methods corresponding to IDL attributes and operations
        InfoContactCheck* Server_i::getInfoContactCheck(const char* check_handle){
          // insert code here and remove the warning
          // warning "Code missing in function <InfoContactCheck* Server_i::getInfoContactCheck(const char* check_handle)>"
        }

        void Server_i::updateContactCheckTests(const char* check_handle, const TestResultUpdateSeq& changes){
          // insert code here and remove the warning
          // warning "Code missing in function <void Server_i::updateContactCheckTests(const char* check_handle, const TestResultUpdateSeq& changes)>"
        }

        void Server_i::resolveContactCheckStatus(const char* check_handle, const char* status){
          // insert code here and remove the warning
          // warning "Code missing in function <void Server_i::resolveContactCheckStatus(const char* check_handle, const char* status)>"
        }

        void Server_i::enqueueContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle){
          // insert code here and remove the warning
          // warning "Code missing in function <void Server_i::enqueueContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle)>"
        }

        ContactTestDefSeq* Server_i::listTestSuiteDefs(const char* lang){
          // insert code here and remove the warning
          // warning "Code missing in function <ContactTestDefSeq* Server_i::listTestSuiteDefs(const char* lang)>"
            return new ContactTestDefSeq;
        }

        ContactCheckStatusDefSeq* Server_i::listCheckStatusDefs(const char* lang){
          // insert code here and remove the warning
          // warning "Code missing in function <ContactCheckStatusDefSeq* Server_i::listCheckStatusDefs(const char* lang)>"
            return new ContactCheckStatusDefSeq;
        }

        ContactTestResultStatusDefSeq* Server_i::listTestResultStatusDefs(const char* lang){
          // insert code here and remove the warning
          // warning "Code missing in function <ContactTestResultStatusDefSeq* Server_i::listTestResultStatusDefs(const char* lang)>"
            return new ContactTestResultStatusDefSeq;
        }
    }
}
