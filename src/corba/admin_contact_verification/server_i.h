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
*  header of admin contact verification wrapper over corba
*/
#ifndef CORBA_ADMIN_CONTACT_VERIFICATION_SERVER_I_H_85445454532330
#define CORBA_ADMIN_CONTACT_VERIFICATION_SERVER_I_H_85445454532330

// generated from idl
#include "src/corba/AdminContactVerification.hh"

#include <string>

namespace Registry
{
    namespace AdminContactVerification
    {
        /// corba interface
        class Server_i : public POA_Registry::AdminContactVerification::Server  {
            private:
                const std::string server_name_;
                // do not copy
                Server_i(const Server_i&); // no definition
                Server_i& operator= (const Server_i&); // no definition

            public:
                Server_i()
                    : server_name_("fred-adifd")
                { }
                virtual ~Server_i(){ }

                // Methods corresponding to IDL attributes and operations
                virtual ContactCheckDetail* getContactCheckDetail(const char* check_handle);

                virtual ContactCheckList* getChecksOfContact(::CORBA::ULongLong contact_id, NullableString* testsuite, ::CORBA::ULong max_item_count);
                virtual ContactCheckList* getActiveChecks(NullableString* testsuite);

                virtual void updateContactCheckTests(const char* check_handle, const TestUpdateSeq& changes, ::CORBA::ULongLong logd_request_id);

                virtual void resolveContactCheckStatus(const char* check_handle, const char* status, ::CORBA::ULongLong logd_request_id);

                virtual void deleteDomainsAfterFailedManualCheck(const char* check_handle);

                virtual char* requestEnqueueingContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle, ::CORBA::ULongLong logd_request_id);
                virtual void confirmEnqueueingContactCheck(const char* check_handle, ::CORBA::ULongLong logd_request_id);
                virtual char* enqueueContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle, ::CORBA::ULongLong logd_request_id);

                virtual MessageSeq* getContactCheckMessages(::CORBA::ULongLong contact_id);

                virtual ContactTestStatusDefSeq* listTestStatusDefs(const char* lang);
                virtual ContactCheckStatusDefSeq* listCheckStatusDefs(const char* lang);
                virtual ContactTestDefSeq* listTestDefs(const char* lang, NullableString* testsuite_handle);
                virtual ContactTestSuiteDefSeq* listTestSuiteDefs(const char* lang);
        };
    }
}

#endif // #include guard
