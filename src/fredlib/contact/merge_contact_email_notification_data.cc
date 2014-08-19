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

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"


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
            typedef std::map<std::string , SortedContactNotificationEmail> EmailMap;//key is dst_contact_roid
            EmailMap email_by_dst_contact;

            for( std::vector<MergeContactEmailNotificationInput>::iterator i = merge_contact_data_.begin()
                    ; i != merge_contact_data_.end(); ++i )
            {
                //check contacts are different
                if(i->merge_output.contactid.dst_contact_roid.compare(i->merge_output.contactid.src_contact_roid) == 0)
                {//error if equal
                    BOOST_THROW_EXCEPTION(Exception().set_invalid_registry_object_identifier(
                            i->merge_output.contactid.dst_contact_roid));
                }

                //look for notification email by contact roid
                EmailMap::iterator email_by_dst_contact_it = email_by_dst_contact.find(i->merge_output.contactid.dst_contact_roid);
                EmailMap::iterator email_by_src_contact_it = email_by_dst_contact.find(i->merge_output.contactid.src_contact_roid);
                if(email_by_dst_contact_it == email_by_dst_contact.end())
                {//email not found -> create new
                    SortedContactNotificationEmail email;

                    email.dst_contact_handle = i->dst_contact_handle;
                    email.removed_list.insert(i->src_contact_handle);
                    email.removed_roid_list.insert(i->merge_output.contactid.src_contact_roid);

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
                        email.removed_roid_list.insert(src_email.removed_roid_list.begin(), src_email.removed_roid_list.end());
                        email_by_dst_contact.erase(email_by_src_contact_it);
                    }

                    //insert new email
                    email_by_dst_contact.insert(EmailMap::value_type(i->merge_output.contactid.dst_contact_roid, email));
                }
                else
                {//email found -> update
                    SortedContactNotificationEmail email(email_by_dst_contact_it->second);

                    if(email.dst_contact_handle.compare(i->dst_contact_handle) != 0)
                    {//error if equal
                        BOOST_THROW_EXCEPTION(Exception().set_invalid_contact_handle(
                                i->dst_contact_handle));
                    }

                    email.removed_list.insert(i->src_contact_handle);
                    email.removed_roid_list.insert(i->merge_output.contactid.src_contact_roid);

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
                        email.removed_roid_list.insert(src_email.removed_roid_list.begin(), src_email.removed_roid_list.end());
                        email_by_dst_contact.erase(email_by_src_contact_it);
                    }

                    //update email
                    email_by_dst_contact_it->second = email;
                    //email_by_dst_contact[i->dst_contact_handle] = email;
                }
            }//for i

            result.reserve(email_by_dst_contact.size());
            for(EmailMap::iterator it = email_by_dst_contact.begin(); it != email_by_dst_contact.end(); ++it)
            {
                MergeContactNotificationEmail notifemail;
                notifemail.dst_contact_handle = it->second.dst_contact_handle;
                notifemail.dst_contact_roid = it->first;

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

                for(std::set<std::string>::iterator si = it->second.removed_roid_list.begin()
                        ; si != it->second.removed_roid_list.end(); ++si)
                {
                    notifemail.removed_roid_list.push_back(*si);
                }

                result.push_back(notifemail);
            }//for it
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return result;
    }

    std::string MergeContactEmailNotificationData::to_string() const
    {
        return Util::format_operation_state("MergeContactEmailNotificationData",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("merge_contact_data",Util::format_container(merge_contact_data_)))
        );
    }

    MergeContactNotificationEmailAddr::MergeContactNotificationEmailAddr(
            const std::vector<MergeContactNotificationEmail>& email_data)
    : email_data_(email_data)
    {}

    std::vector<MergeContactNotificationEmailWithAddr> MergeContactNotificationEmailAddr::exec(OperationContext& ctx)
    {
        std::vector<MergeContactNotificationEmailWithAddr> result;
        try
        {
            result.reserve(email_data_.size());
            for(std::vector<MergeContactNotificationEmail>::const_iterator ci = email_data_.begin()
                ; ci != email_data_.end(); ++ci)
            {
                Database::Result  email_result = ctx.get_conn().exec_params(
                    "SELECT trim(both ' ' from  COALESCE(c.notifyemail,'')), trim(both ' ' from COALESCE(c.email, '')), oreg.name "
                    " FROM object_registry oreg "
                    " JOIN contact c ON  oreg.id = c.id "
                    " WHERE oreg.roid = $1::text"
                , Database::query_param_list(ci->dst_contact_roid));

                if(email_result.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_invalid_registry_object_identifier(
                            ci->dst_contact_roid));
                }
                if(email_result.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get destination contact email"));
                }

                MergeContactNotificationEmailWithAddr email_with_addr;
                std::string tmp_not_email = static_cast<std::string>(email_result[0][0]);
                std::string tmp_email = static_cast<std::string>(email_result[0][1]);
                email_with_addr.notification_email_addr = tmp_not_email.empty() ? tmp_email : tmp_not_email;
                email_with_addr.email_data = *ci;
                result.push_back(email_with_addr);
            }//for ci

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return result;
    }

    std::string MergeContactNotificationEmailAddr::to_string() const
    {
        return Util::format_operation_state("MergeContactNotificationEmailAddr",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("email_data",Util::format_container(email_data_)))
        );
    }

}//namespace Fred

