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
 *  @file merge_contact_email_notification_data.cc
 *  contact merge email notification data
 */

#include <string>
#include <vector>
#include <map>
#include <set>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "fredlib/contact/merge_contact_email_notification_data.h"


namespace Fred
{
    MergeContactEmailNotificationData::MergeContactEmailNotificationData(
            const std::vector<MergeContactEmailNotificationInput>& merge_contact_data)
    : merge_contact_data_(merge_contact_data)
    {}

    void MergeContactEmailNotificationData::update_email(
            std::vector<MergeContactEmailNotificationInput>::iterator i
            , SortedContactNotificationEmail& email)
    {
        for(std::vector<MergeContactUpdateDomainRegistrant>::iterator it
            = i->merge_output.update_domain_registrant.begin()
                ; it != i->merge_output.update_domain_registrant.end() ; ++it)
        {
            email.domain_registrant_list.insert(it->fqdn);
        }

        for(std::vector<MergeContactUpdateDomainAdminContact>::iterator it
            = i->merge_output.update_domain_admin_contact.begin()
                ; it != i->merge_output.update_domain_admin_contact.end() ; ++it)
        {
            email.domain_admin_list.insert(it->fqdn);
        }

        for(std::vector<MergeContactUpdateNssetTechContact>::iterator it
            = i->merge_output.update_nsset_tech_contact.begin()
                ; it != i->merge_output.update_nsset_tech_contact.end() ; ++it)
        {
            email.nsset_tech_list.insert(it->handle);
        }

        for(std::vector<MergeContactUpdateKeysetTechContact>::iterator it
            = i->merge_output.update_keyset_tech_contact.begin()
                ; it != i->merge_output.update_keyset_tech_contact.end() ; ++it)
        {
            email.keyset_tech_list.insert(it->handle);
        }

    }

    std::vector<MergeContactNotificationEmail> MergeContactEmailNotificationData::exec(OperationContext& //ctx //possibly for logging
            )
    {
        std::vector<MergeContactNotificationEmail> result;

        try
        {

        typedef std::map<std::string , SortedContactNotificationEmail> EmailMap;//key is dst_contact_handle
        EmailMap email_by_dst_contact;

        for( std::vector<MergeContactEmailNotificationInput>::iterator i = merge_contact_data_.begin()
                ; i != merge_contact_data_.end(); ++i )
        {
            //look for notification email by contact handle
            EmailMap::iterator email_by_dst_contact_it = email_by_dst_contact.find(i->dst_contact_handle);
            EmailMap::iterator email_by_src_contact_it = email_by_dst_contact.find(i->src_contact_handle);
            if(email_by_dst_contact_it == email_by_dst_contact.end())
            {//email not found -> create new
                SortedContactNotificationEmail email;
                update_email(i,email);

                //add and erase previous merge record
                if(email_by_src_contact_it != email_by_dst_contact.end())
                {
                    SortedContactNotificationEmail src_email(email_by_src_contact_it->second);
                    email.domain_registrant_list.insert(src_email.domain_registrant_list.begin(), src_email.domain_registrant_list.end());
                    email.domain_admin_list.insert(src_email.domain_admin_list.begin(), src_email.domain_admin_list.end());
                    email.nsset_tech_list.insert(src_email.nsset_tech_list.begin(), src_email.nsset_tech_list.end());
                    email.keyset_tech_list.insert(src_email.keyset_tech_list.begin(), src_email.keyset_tech_list.end());
                    email.removed_list.insert(src_email.removed_list.begin(), src_email.removed_list.end());
                    email_by_dst_contact.erase(email_by_src_contact_it);
                }

                //insert new email
                email_by_dst_contact.insert(EmailMap::value_type(i->dst_contact_handle, email));
            }
            else
            {//email found -> update
                SortedContactNotificationEmail email(email_by_dst_contact_it->second);
                update_email(i,email);

                //add and erase previous merge record
                if(email_by_src_contact_it != email_by_dst_contact.end())
                {
                    SortedContactNotificationEmail src_email(email_by_src_contact_it->second);
                    email.domain_registrant_list.insert(src_email.domain_registrant_list.begin(), src_email.domain_registrant_list.end());
                    email.domain_admin_list.insert(src_email.domain_admin_list.begin(), src_email.domain_admin_list.end());
                    email.nsset_tech_list.insert(src_email.nsset_tech_list.begin(), src_email.nsset_tech_list.end());
                    email.keyset_tech_list.insert(src_email.keyset_tech_list.begin(), src_email.keyset_tech_list.end());
                    email.removed_list.insert(src_email.removed_list.begin(), src_email.removed_list.end());
                    email_by_dst_contact.erase(email_by_src_contact_it);
                }

                //update email
                email_by_dst_contact[i->dst_contact_handle] = email;
            }
        }//for i

        for(EmailMap::iterator it = email_by_dst_contact.begin(); it != email_by_dst_contact.end(); ++it)
        {
            MergeContactNotificationEmail notifemail;
            notifemail.dst_contact_handle = it->first;

            for(std::set<std::string>::iterator si = it->second.domain_registrant_list.begin()
                    ; si != it->second.domain_registrant_list.end(); ++si)
            {
                notifemail.domain_registrant_list.push_back(*si);
            }

            for(std::set<std::string>::iterator si = it->second.domain_admin_list.begin()
                    ; si != it->second.domain_admin_list.end(); ++si)
            {
                notifemail.domain_admin_list.push_back(*si);
            }

            for(std::set<std::string>::iterator si = it->second.nsset_tech_list.begin()
                    ; si != it->second.nsset_tech_list.end(); ++si)
            {
                notifemail.nsset_tech_list.push_back(*si);
            }

            for(std::set<std::string>::iterator si = it->second.keyset_tech_list.begin()
                    ; si != it->second.keyset_tech_list.end(); ++si)
            {
                notifemail.keyset_tech_list.push_back(*si);
            }

            for(std::set<std::string>::iterator si = it->second.removed_list.begin()
                    ; si != it->second.removed_list.end(); ++si)
            {
                notifemail.removed_list.push_back(*si);
            }

            result.push_back(notifemail);
        }//for it

        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<MergeContactEmailNotificationDataException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }

        return result;
    }
}//namespace Fred

