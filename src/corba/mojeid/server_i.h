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
*  @server_i.h
*  header of mojeid corba wrapper
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/MojeID.idl
*/
#ifndef SERVER_I_H_
#define SERVER_I_H_

#include <MojeID.hh>
#include <memory>
#include <string>

namespace Registry
{
    namespace MojeID
    {
        class MojeIDImpl;//pimpl class

        ///mojeid corba interface
        class Server_i: public POA_Registry::MojeID::Server
        {
        private:
            // do not copy
            const std::auto_ptr<MojeIDImpl> pimpl_;
            Server_i(const Server_i&);//no body
            Server_i& operator= (const Server_i&);//no body

        public:
          // standard constructor
          Server_i(const std::string &_server_name);
          virtual ~Server_i();
          // methods corresponding to defined IDL attributes and operations
          ::CORBA::ULongLong contactCreatePrepare(
                  const Registry::MojeID::Contact& c
                  , const char* trans_id
                  , ::CORBA::ULongLong request_id
                  , ::CORBA::String_out ident);

          ::CORBA::ULongLong contactTransferPrepare(
                  const char* handle
                  , const char* trans_id
                  , ::CORBA::ULongLong request_id
                  , ::CORBA::String_out ident);

          void contactUpdatePrepare(
                  const Registry::MojeID::Contact& c
                  , const char* trans_id
                  , ::CORBA::ULongLong request_id);

          Registry::MojeID::Contact* contactInfo(
                  ::CORBA::ULongLong contact_id);

          ::CORBA::ULongLong processIdentification(
                  const char* ident_request_id
                  , const char* password
                  , ::CORBA::ULongLong request_id);

          char* getIdentificationInfo(
                  ::CORBA::ULongLong contact_id);

          void commitPreparedTransaction(
                  const char* trans_id);

          void rollbackPreparedTransaction(
                  const char* trans_id);

          Registry::MojeID::Buffer* getValidationPdf(
                  ::CORBA::ULongLong contact_id);

          void createValidationRequest(
                  ::CORBA::ULongLong contact_id
                   , ::CORBA::ULongLong request_id);

          Registry::MojeID::ContactStateInfoList* getContactsStates(
                  ::CORBA::ULong last_hours);

          Registry::MojeID::ContactStateInfo getContactState(
                  ::CORBA::ULongLong contact_id);

          ::CORBA::ULongLong getContactId(
                  const char* handle);

          void contactCancelAccountPrepare(
              ::CORBA::ULongLong contact_id
               , const char* trans_id
               , ::CORBA::ULongLong request_id);

          Registry::MojeID::ContactHandleList* getUnregistrableHandles();

          char* contactAuthInfo(::CORBA::ULongLong contact_id);

        };//class Server_i
    }//namespace MojeID
}//namespace Registry

#endif //SERVER_I_H_
