/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
*  header of domain browser corba wrapper
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/DomainBrowser.idl
*/
#ifndef DOMAIN_BROWSER_I_H_
#define DOMAIN_BROWSER_I_H_


#include <memory>
#include "src/corba/DomainBrowser.hh"
#include "src/domain_browser/domain_browser.h"

namespace Registry
{
    namespace DomainBrowser
    {

        class DomainBrowserImpl;//pimpl class

        ///domain browser corba interface
        class Server_i: public POA_Registry::DomainBrowser::Server
        {
        private:
            // do not copy
            const std::auto_ptr<Registry::DomainBrowserImpl::DomainBrowser> pimpl_;

            Server_i(const Server_i&);//no body
            Server_i& operator= (const Server_i&);//no body

        public:
            // standard constructor
            Server_i(const std::string &_server_name,
                    const std::string& _update_registrar_handle,
                    unsigned int domain_list_limit,
                    unsigned int nsset_list_limit,
                    unsigned int keyset_list_limit,
                    unsigned int contact_list_limit);

            virtual ~Server_i();
            // methods corresponding to defined IDL attributes and operations
            ::CORBA::ULongLong getObjectRegistryId(
                const char* objtype,
                const char* handle);

            Registry::DomainBrowser::DomainList* getDomainList(
                ::CORBA::ULongLong user_contact_id,
                ::CORBA::ULongLong contact_id,
                ::CORBA::ULong offset,
                ::CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::NssetList* getNssetList(
                ::CORBA::ULongLong user_contact_id,
                ::CORBA::ULongLong contact_id,
                ::CORBA::ULong offset,
                ::CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::KeysetList* getKeysetList(
                ::CORBA::ULongLong user_contact_id,
                ::CORBA::ULongLong contact_id,
                ::CORBA::ULong offset,
                ::CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::DomainList* getDomainsForKeyset(
                ::CORBA::ULongLong contact_id,
                ::CORBA::ULongLong keyset_id,
                ::CORBA::ULong offset,
                ::CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::DomainList* getDomainsForNsset(
                ::CORBA::ULongLong contact_id,
                ::CORBA::ULongLong nsset_id,
                ::CORBA::ULong offset,
                ::CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::ContactDetail* getContactDetail(
                ::CORBA::ULongLong contact_id,
                 ::CORBA::ULongLong detail_id,
                const char* lang,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::NSSetDetail* getNssetDetail(
                ::CORBA::ULongLong contact_id,
                 ::CORBA::ULongLong nsset_id,
                const char* lang,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::DomainDetail* getDomainDetail(
                ::CORBA::ULongLong contact_id,
                 ::CORBA::ULongLong domain_id,
                const char* lang,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::KeysetDetail* getKeysetDetail(
                ::CORBA::ULongLong contact_id,
                 ::CORBA::ULongLong keyset_id,
                const char* lang,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::RegistrarDetail* getRegistrarDetail(
                ::CORBA::ULongLong contact_id,
                const char* handle);

            ::CORBA::Boolean setContactDiscloseFlags(
                ::CORBA::ULongLong contact_id,
                const Registry::DomainBrowser::UpdateContactDiscloseFlags& flags,
                ::CORBA::ULongLong request_id);

            ::CORBA::Boolean setAuthInfo(
                ::CORBA::ULongLong contact_id,
                const char* objtype,
                ::CORBA::ULongLong objref_id,
                const char* auth_info,
                ::CORBA::ULongLong request_id);

            ::CORBA::Boolean setObjectBlockStatus(::CORBA::ULongLong contact_id,
                const char* objtype,
                const Registry::DomainBrowser::ObjectIdSeq& objects,
                Registry::DomainBrowser::ObjectBlockType block,
                Registry::DomainBrowser::RecordSequence_out blocked);

            Registry::DomainBrowser::RecordSequence* getPublicStatusDesc(const char* lang);

            Registry::DomainBrowser::RecordSet* getMergeContactCandidateList(
                ::CORBA::ULongLong contact_id,
                ::CORBA::ULong offset,
                ::CORBA::Boolean& limit_exceeded);

            void mergeContacts(::CORBA::ULongLong dst_contact_id,
                const Registry::DomainBrowser::ObjectIdSeq& src_contact_id_list,
                ::CORBA::ULongLong request_id);

        };//class Server_i
    }//namespace DomainBrowser
}//namespace Registry

#endif //DOMAIN_BROWSER_I_H_
