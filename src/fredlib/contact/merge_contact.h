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
        std::string sponsoring_registrar;
        std::string set_registrant;
        Optional<unsigned long long> history_id;
    };

    struct MergeContactUpdateDomainAdminContact
    {
        std::string fqdn;
        std::string sponsoring_registrar;
        std::string rem_admin_contact;
        std::string add_admin_contact;
        Optional<unsigned long long> history_id;
    };

    struct MergeContactUpdateNssetTechContact
    {
        std::string handle;
        std::string sponsoring_registrar;
        std::string rem_tech_contact;
        std::string add_tech_contact;
        Optional<unsigned long long> history_id;
    };

    struct MergeContactUpdateKeysetTechContact
    {
        std::string handle;
        std::string sponsoring_registrar;
        std::string rem_tech_contact;
        std::string add_tech_contact;
        Optional<unsigned long long> history_id;
    };

    struct MergeContactOutput
    {
        std::vector<MergeContactUpdateDomainRegistrant> update_domain_registrant;
        std::vector<MergeContactUpdateDomainAdminContact> update_domain_admin_contact;
        std::vector<MergeContactUpdateNssetTechContact> update_nsset_tech_contact;
        std::vector<MergeContactUpdateKeysetTechContact> update_keyset_tech_contact;
    };


    class MergeContact
    {
        const std::string src_contact_handle_;//source contact identifier
        const std::string dst_contact_handle_;//destination contact identifier
        const std::string registrar_;//registrar used for object updates
        Optional<unsigned long long> logd_request_id_; //id of the new entry in log_entry

        void lock_object_registry_row_for_update(OperationContext& ctx, bool dry_run);
        void diff_contacts(OperationContext& ctx);
        MergeContactOutput merge_contact_impl(OperationContext& ctx, bool dry_run);

    public:
        MergeContact(const std::string& from_contact_handle, const std::string& to_contact_handle, const std::string& registrar);
        MergeContact& set_logd_request_id(unsigned long long logd_request_id);
        MergeContactOutput exec_dry_run(OperationContext& ctx);//history_id not set in output
        MergeContactOutput exec(OperationContext& ctx);
    };//class MergeContact

    //exception impl
    class MergeContactException
    : public OperationExceptionImpl<MergeContactException, 8192>
    {
    public:
        MergeContactException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<MergeContactException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"not found:src_contact_handle", "not found:dst_contact_handle", "not found:registrar"
                , "invalid:src_contact_handle", "invalid:dst_contact_handle"};
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class MergeContactException

    typedef MergeContactException::OperationErrorType MergeContactError;
#define MCEX(DATA) MergeContactException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MCERR(DATA) MergeContactError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred

#endif//MERGE_CONTACT_H
