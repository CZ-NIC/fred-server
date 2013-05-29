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
 *  @file merge_contact.h
 *  contact merge
 */

#ifndef MERGE_CONTACT_H
#define MERGE_CONTACT_H

#include <string>
#include <vector>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Fred
{

    struct MergeContactUpdateDomainRegistrant
    {
        std::string fqdn;
        unsigned long long domain_id;
        std::string sponsoring_registrar;
        std::string set_registrant;
        Optional<unsigned long long> history_id;
        MergeContactUpdateDomainRegistrant()
        : domain_id(0)
        {}
        MergeContactUpdateDomainRegistrant(
            const std::string& _fqdn
            , unsigned long long _domain_id
            , const std::string& _sponsoring_registrar
            , const std::string& _set_registrant
            , const Optional<unsigned long long>& _history_id
        )
            : fqdn(_fqdn)
            , domain_id(_domain_id)
            , sponsoring_registrar(_sponsoring_registrar)
            , set_registrant(_set_registrant)
            , history_id(_history_id)
        {}
        friend std::ostream& operator<<(std::ostream& os, const MergeContactUpdateDomainRegistrant& i)
        {
            return os << "MergeContactUpdateDomainRegistrant fqdn: " << i.fqdn
                << " domain_id: " << i.domain_id
                << " sponsoring_registrar: " << i.sponsoring_registrar
                << " set_registrant: " << i.set_registrant
                << " history_id: " << i.history_id.print_quoted()
                    ;
        }
    };

    struct MergeContactUpdateDomainAdminContact
    {
        std::string fqdn;
        unsigned long long domain_id;
        std::string sponsoring_registrar;
        std::string rem_admin_contact;
        std::string add_admin_contact;
        Optional<unsigned long long> history_id;
        MergeContactUpdateDomainAdminContact()
        : domain_id(0)
        {}
        MergeContactUpdateDomainAdminContact(
            const std::string& _fqdn
            , unsigned long long _domain_id
            , const std::string& _sponsoring_registrar
            , const std::string& _rem_admin_contact
            , const std::string& _add_admin_contact
            , const Optional<unsigned long long>& _history_id
        )
            : fqdn(_fqdn)
            , domain_id(_domain_id)
            , sponsoring_registrar(_sponsoring_registrar)
            , rem_admin_contact(_rem_admin_contact)
            , add_admin_contact(_add_admin_contact)
            , history_id(_history_id)
        {}
        friend std::ostream& operator<<(std::ostream& os, const MergeContactUpdateDomainAdminContact& i)
        {
            return os << "MergeContactUpdateDomainRegistrant fqdn: " << i.fqdn
                    << " domain_id: " << i.domain_id
                    << " sponsoring_registrar: " << i.sponsoring_registrar
                    << " rem_admin_contact: " << i.rem_admin_contact
                    << " add_admin_contact: " << i.add_admin_contact
                    << " history_id: " << i.history_id.print_quoted()
                    ;
        }
    };

    struct MergeContactUpdateNssetTechContact
    {
        std::string handle;
        unsigned long long nsset_id;
        std::string sponsoring_registrar;
        std::string rem_tech_contact;
        std::string add_tech_contact;
        Optional<unsigned long long> history_id;
        MergeContactUpdateNssetTechContact()
        : nsset_id(0)
        {}
        MergeContactUpdateNssetTechContact(
            const std::string& _handle
            , unsigned long long _nsset_id
            , const std::string& _sponsoring_registrar
            , const std::string& _rem_tech_contact
            , const std::string& _add_tech_contact
            , const Optional<unsigned long long>& _history_id
        )
            : handle(_handle)
            , nsset_id(_nsset_id)
            , sponsoring_registrar(_sponsoring_registrar)
            , rem_tech_contact(_rem_tech_contact)
            , add_tech_contact(_add_tech_contact)
            , history_id(_history_id)
            {}
        friend std::ostream& operator<<(std::ostream& os, const MergeContactUpdateNssetTechContact& i)
        {
            return os << "MergeContactUpdateDomainRegistrant handle: " << i.handle
                    << " nsset_id: " << i.nsset_id
                    << " sponsoring_registrar: " << i.sponsoring_registrar
                    << " rem_tech_contact: " << i.rem_tech_contact
                    << " add_tech_contact: " << i.add_tech_contact
                    << " history_id: " << i.history_id.print_quoted()
                    ;
        }
    };

    struct MergeContactUpdateKeysetTechContact
    {
        std::string handle;
        unsigned long long keyset_id;
        std::string sponsoring_registrar;
        std::string rem_tech_contact;
        std::string add_tech_contact;
        Optional<unsigned long long> history_id;
        MergeContactUpdateKeysetTechContact()
        : keyset_id(0)
        {}
        MergeContactUpdateKeysetTechContact(
            const std::string& _handle
            , unsigned long long _keyset_id
            , const std::string& _sponsoring_registrar
            , const std::string& _rem_tech_contact
            , const std::string& _add_tech_contact
            , const Optional<unsigned long long>& _history_id
        )
            : handle(_handle)
            , keyset_id(_keyset_id)
            , sponsoring_registrar(_sponsoring_registrar)
            , rem_tech_contact(_rem_tech_contact)
            , add_tech_contact(_add_tech_contact)
            , history_id(_history_id)
        {}
        friend std::ostream& operator<<(std::ostream& os, const MergeContactUpdateKeysetTechContact& i)
        {
            return os << "MergeContactUpdateDomainRegistrant handle: " << i.handle
                    << " keyset_id: " << i.keyset_id
                    << " sponsoring_registrar: " << i.sponsoring_registrar
                    << " rem_tech_contact: " << i.rem_tech_contact
                    << " add_tech_contact: " << i.add_tech_contact
                    << " history_id: " << i.history_id.print_quoted()
                    ;
        }
    };

    struct MergeContactLockedContactId
    {
        unsigned long long src_contact_id;
        unsigned long long src_contact_historyid;
        std::string src_contact_roid;
        std::string src_contact_sponsoring_registrar;
        unsigned long long dst_contact_id;
        unsigned long long dst_contact_historyid;
        std::string dst_contact_roid;
        std::string dst_contact_sponsoring_registrar;
        MergeContactLockedContactId()
        : src_contact_id(0)
        , src_contact_historyid(0)
        , src_contact_sponsoring_registrar()
        , dst_contact_id(0)
        , dst_contact_historyid(0)
        , dst_contact_sponsoring_registrar()
        {}
        MergeContactLockedContactId(
                unsigned long long _src_contact_id
                , unsigned long long _src_contact_historyid
                , const std::string& _src_contact_roid
                , const std::string& _src_contact_sponsoring_registrar
                , unsigned long long _dst_contact_id
                , unsigned long long _dst_contact_historyid
                , const std::string& _dst_contact_roid
                , const std::string& _dst_contact_sponsoring_registrar
                )
        : src_contact_id(_src_contact_id)
        , src_contact_historyid(_src_contact_historyid)
        , src_contact_roid(_src_contact_roid)
        , src_contact_sponsoring_registrar(_src_contact_sponsoring_registrar)
        , dst_contact_id(_dst_contact_id)
        , dst_contact_historyid(_dst_contact_historyid)
        , dst_contact_roid(_dst_contact_roid)
        , dst_contact_sponsoring_registrar(_dst_contact_sponsoring_registrar)
        {}
        friend std::ostream& operator<<(std::ostream& os, const MergeContactLockedContactId& i)
        {
            return os << "MergeContactLockedContactId"
                    " src_contact_id: " << i.src_contact_id
                << " src_contact_historyid: " << i.src_contact_historyid
                << " src_contact_roid: " << i.src_contact_roid
                << " src_contact_sponsoring_registrar: " << i.src_contact_sponsoring_registrar
                << " dst_contact_id: " << i.dst_contact_id
                << " dst_contact_historyid: " << i.dst_contact_historyid
                << " dst_contact_roid: " << i.dst_contact_roid
                << "dst_contact_sponsoring_registrar: " << i.dst_contact_sponsoring_registrar
            ;
        }
    };

    struct MergeContactOutput
    {
        MergeContactLockedContactId contactid;
        std::vector<MergeContactUpdateDomainRegistrant> update_domain_registrant;
        std::vector<MergeContactUpdateDomainAdminContact> update_domain_admin_contact;
        std::vector<MergeContactUpdateNssetTechContact> update_nsset_tech_contact;
        std::vector<MergeContactUpdateKeysetTechContact> update_keyset_tech_contact;
        MergeContactOutput(){}
        MergeContactOutput(
                const MergeContactLockedContactId& _contactid
                , const std::vector<MergeContactUpdateDomainRegistrant>& _update_domain_registrant
                , const std::vector<MergeContactUpdateDomainAdminContact>& _update_domain_admin_contact
                , const std::vector<MergeContactUpdateNssetTechContact>& _update_nsset_tech_contact
                , const std::vector<MergeContactUpdateKeysetTechContact>& _update_keyset_tech_contact
                )
        : contactid(_contactid)
        , update_domain_registrant(_update_domain_registrant)
        , update_domain_admin_contact(_update_domain_admin_contact)
        , update_nsset_tech_contact(_update_nsset_tech_contact)
        , update_keyset_tech_contact(_update_keyset_tech_contact)
        {}
        friend std::ostream& operator<<(std::ostream& os, const MergeContactOutput& i)
        {
            os << "MergeContactOutput contactid: " << i.contactid;
            if(!i.update_domain_registrant.empty()) os << " ";
            for(std::vector<MergeContactUpdateDomainRegistrant>::const_iterator ci = i.update_domain_registrant.begin()
                    ; ci != i.update_domain_registrant.end() ;  ++ci) os << *ci;
            if(!i.update_domain_admin_contact.empty()) os << " ";
            for(std::vector<MergeContactUpdateDomainAdminContact>::const_iterator ci = i.update_domain_admin_contact.begin()
                    ; ci != i.update_domain_admin_contact.end() ;  ++ci) os << *ci;
            if(!i.update_nsset_tech_contact.empty()) os << " ";
            for(std::vector<MergeContactUpdateNssetTechContact>::const_iterator ci = i.update_nsset_tech_contact.begin()
                    ; ci != i.update_nsset_tech_contact.end() ;  ++ci) os << *ci;
            if(!i.update_keyset_tech_contact.empty()) os << " ";
            for(std::vector<MergeContactUpdateKeysetTechContact>::const_iterator ci = i.update_keyset_tech_contact.begin()
                    ; ci != i.update_keyset_tech_contact.end() ;  ++ci) os << *ci;
            return os;
        }
    };

    class MergeContact
    {
        const std::string src_contact_handle_;//source contact identifier
        const std::string dst_contact_handle_;//destination contact identifier
        const std::string registrar_;//registrar used for object updates
        Optional<unsigned long long> logd_request_id_; //id of the new entry in log_entry

        MergeContactLockedContactId lock_object_registry_row_for_update(OperationContext& ctx, bool dry_run);
        void diff_contacts(OperationContext& ctx);
        MergeContactOutput merge_contact_impl(OperationContext& ctx, bool dry_run);

    public:
        DECLARE_EXCEPTION_DATA(unknown_source_contact_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_destination_contact_handle, std::string);
        struct InvalidContacts{std::string source_handle; std::string destination_handle;
        InvalidContacts(const std::string& _source_handle, const std::string& _destination_handle)
        : source_handle(_source_handle), destination_handle(_destination_handle){}};
        DECLARE_EXCEPTION_DATA(unable_to_get_difference_of_contacts, InvalidContacts);
        DECLARE_EXCEPTION_DATA(contacts_differ, InvalidContacts);
        DECLARE_EXCEPTION_DATA(identical_contacts_handle, std::string);
        DECLARE_EXCEPTION_DATA(identical_contacts_roid, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_source_contact_handle<Exception>
        , ExceptionData_unknown_destination_contact_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_unable_to_get_difference_of_contacts<Exception>
        , ExceptionData_contacts_differ<Exception>
        , ExceptionData_identical_contacts_handle<Exception>
        , ExceptionData_identical_contacts_roid<Exception>
        {};

        MergeContact(const std::string& from_contact_handle, const std::string& to_contact_handle, const std::string& registrar);
        MergeContact& set_logd_request_id(unsigned long long logd_request_id);
        MergeContactOutput exec_dry_run(OperationContext& ctx);//history_id not set in output
        MergeContactOutput exec(OperationContext& ctx);
        friend std::ostream& operator<<(std::ostream& os, const MergeContact& i);
        std::string to_string();
    };//class MergeContact

}//namespace Fred

#endif//MERGE_CONTACT_H
