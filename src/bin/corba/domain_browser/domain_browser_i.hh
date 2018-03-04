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
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace/enum/idl/idl/DomainBrowser.idl
*/
#ifndef DOMAIN_BROWSER_I_HH_FC4AD335535141AD98C4B5643D47AFD5
#define DOMAIN_BROWSER_I_HH_FC4AD335535141AD98C4B5643D47AFD5


#include <memory>
#include "src/bin/corba/DomainBrowser.hh"
#include "src/backend/domain_browser/domain_browser.hh"

namespace Fred {
namespace Backend {
namespace DomainBrowser {

class DomainBrowser;

} // namespace Fred::Backend::DomainBrowser
} // namespace Fred::Backend
} // namespace Fred

namespace CorbaConversion
{
    namespace DomainBrowser
    {

        ///domain browser corba interface
        class Server_i: public POA_Registry::DomainBrowser::Server
        {
        private:
            // do not copy
            const std::unique_ptr<Fred::Backend::DomainBrowser::DomainBrowser> pimpl_;

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
            CORBA::ULongLong getContactId(const char* handle);

            Registry::DomainBrowser::DomainList* getDomainList(
                CORBA::ULongLong user_contact_id,
                 Registry::DomainBrowser::NullableULongLong* contact_id_ptr,
                CORBA::ULong offset,
                CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::NssetList* getNssetList(
                CORBA::ULongLong user_contact_id,
                 Registry::DomainBrowser::NullableULongLong* contact_id_ptr,
                CORBA::ULong offset,
                CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::KeysetList* getKeysetList(
                CORBA::ULongLong user_contact_id,
                 Registry::DomainBrowser::NullableULongLong* contact_id_ptr,
                CORBA::ULong offset,
                CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::DomainList* getDomainsForKeyset(
                CORBA::ULongLong user_contact_id,
                CORBA::ULongLong keyset_id,
                CORBA::ULong offset,
                CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::DomainList* getDomainsForNsset(
                CORBA::ULongLong contact_id,
                CORBA::ULongLong nsset_id,
                CORBA::ULong offset,
                CORBA::Boolean& limit_exceeded);

            Registry::DomainBrowser::ContactDetail* getContactDetail(
                CORBA::ULongLong user_contact_id,
                 CORBA::ULongLong detail_id,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::NSSetDetail* getNssetDetail(
                CORBA::ULongLong user_contact_id,
                 CORBA::ULongLong nsset_id,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::DomainDetail* getDomainDetail(
                CORBA::ULongLong user_contact_id,
                 CORBA::ULongLong domain_id,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::KeysetDetail* getKeysetDetail(
                CORBA::ULongLong user_contact_id,
                 CORBA::ULongLong keyset_id,
                Registry::DomainBrowser::DataAccessLevel& auth_result);

            Registry::DomainBrowser::RegistrarDetail* getRegistrarDetail(
                CORBA::ULongLong user_contact_id,
                const char* handle);

            CORBA::Boolean setContactDiscloseFlags(
                CORBA::ULongLong user_contact_id,
                const Registry::DomainBrowser::UpdateContactDiscloseFlags& flags,
                CORBA::ULongLong request_id);

            CORBA::Boolean setContactAuthInfo(
                CORBA::ULongLong user_contact_id,
                const char* auth_info,
                CORBA::ULongLong request_id);

            CORBA::Boolean setObjectBlockStatus(CORBA::ULongLong user_contact_id,
                const char* objtype,
                const Registry::DomainBrowser::ObjectIdSeq& objects,
                Registry::DomainBrowser::ObjectBlockType block,
                Registry::DomainBrowser::RefusedObjectHandleSequence_out change_prohibited);

            Registry::DomainBrowser::StatusDescList* getPublicStatusDesc(const char* lang);

            Registry::DomainBrowser::MergeContactCandidateList* getMergeContactCandidateList(
                CORBA::ULongLong contact_id,
                CORBA::ULong offset,
                CORBA::Boolean& limit_exceeded);

            void mergeContacts(CORBA::ULongLong dst_contact_id,
                const Registry::DomainBrowser::ObjectIdSeq& src_contact_id_list,
                CORBA::ULongLong request_id);

            void setContactPreferenceForDomainExpirationLetters(
                CORBA::ULongLong user_contact_id,
                CORBA::Boolean send_expiration_letters,
                CORBA::ULongLong request_id);

        };

    } // namespace CorbaConversion::DomainBrowser
} // namespace CorbaConversion

#endif
