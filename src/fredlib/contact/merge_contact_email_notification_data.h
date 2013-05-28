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
 *  @file merge_contact_email_notification_data.h
 *  contact merge email notification data
 */

#ifndef MERGE_CONTACT_EMAIL_NOTIFICATION_DATA_H
#define MERGE_CONTACT_EMAIL_NOTIFICATION_DATA_H

#include <string>
#include <vector>


#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "fredlib/contact/merge_contact.h"


namespace Fred
{
    struct MergeContactEmailNotificationInput
    {
        std::string src_contact_handle;//source contact identifier
        std::string dst_contact_handle;//destination contact identifier
        MergeContactOutput merge_output;//result of merge operation
        MergeContactEmailNotificationInput(){}
        MergeContactEmailNotificationInput(const std::string& _src_contact_handle
                , const std::string& _dst_contact_handle
                , const MergeContactOutput& _merge_output)
        : src_contact_handle(_src_contact_handle)
        , dst_contact_handle(_dst_contact_handle)
        , merge_output(_merge_output)
        {}
        friend std::ostream& operator<<(std::ostream& os, const MergeContactEmailNotificationInput& i)
        {
            return os << "MergeContactEmailNotificationInput"
                    " src_contact_handle: " << i.src_contact_handle
                << " dst_contact_handle: " << i.dst_contact_handle
                << " merge_output: " << i.merge_output
            ;
        }
    };

    struct MergeContactNotificationEmail
    {
        std::string dst_contact_handle;
        std::string dst_contact_roid;
        std::vector<std::string> domain_registrant_list;
        std::vector<std::string> domain_admin_list;
        std::vector<std::string> nsset_tech_list;
        std::vector<std::string> keyset_tech_list;
        std::vector<std::string> removed_list;
        std::vector<std::string> removed_roid_list;
        friend std::ostream& operator<<(std::ostream& os, const MergeContactNotificationEmail& i)
        {
            os << "MergeContactNotificationEmail dst_contact_handle: " << i.dst_contact_handle
                    << " dst_contact_roid: " << i.dst_contact_roid;
            if(!i.domain_registrant_list.empty()) os << " ";
            for(std::vector<std::string>::const_iterator ci = i.domain_registrant_list.begin()
                    ; ci != i.domain_registrant_list.end() ;  ++ci) os << *ci;
            if(!i.domain_admin_list.empty()) os << " ";
            for(std::vector<std::string>::const_iterator ci = i.domain_admin_list.begin()
                    ; ci != i.domain_admin_list.end() ;  ++ci) os << *ci;
            if(!i.nsset_tech_list.empty()) os << " ";
            for(std::vector<std::string>::const_iterator ci = i.nsset_tech_list.begin()
                    ; ci != i.nsset_tech_list.end() ;  ++ci) os << *ci;
            if(!i.keyset_tech_list.empty()) os << " ";
            for(std::vector<std::string>::const_iterator ci = i.keyset_tech_list.begin()
                    ; ci != i.keyset_tech_list.end() ;  ++ci) os << *ci;
            if(!i.removed_list.empty()) os << " ";
            for(std::vector<std::string>::const_iterator ci = i.removed_list.begin()
                    ; ci != i.removed_list.end() ;  ++ci) os << *ci;
            if(!i.removed_roid_list.empty()) os << " ";
            for(std::vector<std::string>::const_iterator ci = i.removed_roid_list.begin()
                    ; ci != i.removed_roid_list.end() ;  ++ci) os << *ci;
            return os;
        }
    };

    struct SortedContactNotificationEmail
    {
        std::string dst_contact_handle;
        std::set<std::string> domain_registrant_list;
        std::set<std::string> domain_admin_list;
        std::set<std::string> nsset_tech_list;
        std::set<std::string> keyset_tech_list;
        std::set<std::string> removed_list;
        std::set<std::string> removed_roid_list;
    };

    class MergeContactEmailNotificationData
    {
        std::vector<MergeContactEmailNotificationInput> merge_contact_data_;

        void update_email(std::vector<MergeContactEmailNotificationInput>::iterator i
                    , SortedContactNotificationEmail& email);

    public:
        DECLARE_EXCEPTION_DATA(invalid_contact_handle, std::string);
        DECLARE_EXCEPTION_DATA(invalid_registry_object_identifier, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_invalid_contact_handle<Exception>
        , ExceptionData_invalid_registry_object_identifier<Exception>
        {};

        MergeContactEmailNotificationData(const std::vector<MergeContactEmailNotificationInput>& merge_contact_data_);
        std::vector<MergeContactNotificationEmail> exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const MergeContactEmailNotificationData& i);
        std::string to_string();

    };//class MergeContactEmailNotificationData

    struct MergeContactNotificationEmailWithAddr
    {
        std::string notification_email_addr;
        MergeContactNotificationEmail email_data;
    };

    class MergeContactNotificationEmailAddr
    {
        const std::vector<MergeContactNotificationEmail> email_data_;
    public:
        DECLARE_EXCEPTION_DATA(invalid_registry_object_identifier, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_invalid_registry_object_identifier<Exception>
        {};

        MergeContactNotificationEmailAddr(const std::vector<MergeContactNotificationEmail>& email_data);
        std::vector<MergeContactNotificationEmailWithAddr> exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const MergeContactNotificationEmailAddr& ich);
        std::string to_string();
    };

}//namespace Fred

#endif//MERGE_CONTACT_EMAIL_NOTIFICATION_DATA_H
