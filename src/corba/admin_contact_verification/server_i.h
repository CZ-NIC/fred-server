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
#include <AdminContactVerification.hh>

namespace Registry
{
    namespace AdminContactVerification
    {
        /// corba interface
        class Server_i : public POA_Registry::AdminContactVerification::Server {
            private:
                // do not copy
                Server_i(const Server_i&); // no definition
                Server_i& operator= (const Server_i&); // no definition

            public:
                Server_i() {};
                virtual ~Server_i(){};

                //   Methods corresponding to IDL attributes and operations
                virtual InfoContactCheck* getInfoContactCheck(const char* check_handle);

                virtual void updateContactCheckTests(const char* check_handle, const TestResultUpdateSeq& changes);

                virtual void resolveContactCheckStatus(const char* check_handle, const char* status);

                virtual void enqueueContactCheck(::CORBA::ULongLong contact_id, const char* testsuite_handle);

                virtual ContactTestDefSeq* listTestSuiteDefs(const char* lang);

                virtual ContactCheckStatusDefSeq* listCheckStatusDefs(const char* lang);

                virtual ContactTestResultStatusDefSeq* listTestResultStatusDefs(const char* lang);
        };
    }
}

#endif // #include guard
