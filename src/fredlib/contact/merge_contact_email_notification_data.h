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
    };

    struct MergeContactNotificationEmail
    {
        std::string dst_contact_handle;
        std::vector<std::string> domain_registrant_list;
        std::vector<std::string> domain_admin_list;
        std::vector<std::string> nsset_tech_list;
        std::vector<std::string> keyset_tech_list;
        std::vector<std::string> removed_list;
    };

    struct SortedContactNotificationEmail
    {
        std::set<std::string> domain_registrant_list;
        std::set<std::string> domain_admin_list;
        std::set<std::string> nsset_tech_list;
        std::set<std::string> keyset_tech_list;
        std::set<std::string> removed_list;
    };

    class MergeContactEmailNotificationData
    {
        std::vector<MergeContactEmailNotificationInput> merge_contact_data_;

        void update_email(std::vector<MergeContactEmailNotificationInput>::iterator i
                    , SortedContactNotificationEmail& email);

    public:
        MergeContactEmailNotificationData(const std::vector<MergeContactEmailNotificationInput>& merge_contact_data_);
        std::vector<MergeContactNotificationEmail> exec(OperationContext& ctx);
    };//class MergeContactEmailNotificationData

    //exception impl
    class MergeContactEmailNotificationDataException
    : public OperationExceptionImpl<MergeContactEmailNotificationDataException, 8192>
    {
    public:
        MergeContactEmailNotificationDataException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<MergeContactEmailNotificationDataException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"invalid"};
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class MergeContactEmailNotificationDataException

    typedef MergeContactEmailNotificationDataException::OperationErrorType MergeContactEmailNotificationDataError;
#define MCENDEX(DATA) MergeContactEmailNotificationDataException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MCENDERR(DATA) MergeContactEmailNotificationDataError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred

#endif//MERGE_CONTACT_EMAIL_NOTIFICATION_DATA_H
