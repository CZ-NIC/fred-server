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
 *  @file merge_contact.cc
 *  contact merge
 */

#include <string>

#include <boost/algorithm/string.hpp>

#include "fredlib/contact/merge_contact.h"

#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"

namespace Fred
{
    MergeContact::MergeContact(const std::string& from_contact_handle, const std::string& to_contact_handle, const std::string& registrar)
    : src_contact_handle_(from_contact_handle)
    , dst_contact_handle_(to_contact_handle)
    , registrar_(registrar)
    {
        if(boost::algorithm::to_upper_copy(src_contact_handle_).compare(boost::algorithm::to_upper_copy(dst_contact_handle_)) == 0)
        {
            std::string errmsg("unable to merge the same contacts || identical:dst_contact_handle: ");
            errmsg += boost::replace_all_copy(dst_contact_handle_,"|", "[pipe]");//quote pipes
            errmsg += " |";
            throw MCEX(errmsg.c_str());
        }
    }

    MergeContact& MergeContact::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    MergeContactLockedContactId MergeContact::lock_object_registry_row_for_update(OperationContext& ctx, bool dry_run)
    {
        MergeContactLockedContactId ret;
        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                std::string("SELECT oreg.id, oreg.historyid, oreg.roid, r.handle"
                " FROM enum_object_type eot "
                " JOIN object_registry oreg ON oreg.type = eot.id AND oreg.erdate IS NULL "
                " AND oreg.name = UPPER($1::text) "
                " JOIN object o ON o.id = oreg.id JOIN registrar r ON r.id = o.clid"
                " WHERE eot.name = 'contact' ") + (dry_run ? " " : " FOR UPDATE OF oreg")
                , Database::query_param_list(src_contact_handle_));

            if (lock_res.size() != 1)
            {
                std::string errmsg("|| not found:src_contact_handle: ");
                errmsg += boost::replace_all_copy(src_contact_handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw MCEX(errmsg.c_str());
            }

            ret.src_contact_id = static_cast<unsigned long long>(lock_res[0][0]);
            ret.src_contact_historyid = static_cast<unsigned long long>(lock_res[0][1]);
            ret.src_contact_roid = static_cast<std::string>(lock_res[0][2]);
            ret.src_contact_sponsoring_registrar = static_cast<std::string>(lock_res[0][3]);
        }

        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                std::string("SELECT oreg.id, oreg.historyid, oreg.roid, r.handle "
                        " FROM enum_object_type eot "
                        " JOIN object_registry oreg ON oreg.type = eot.id AND oreg.erdate IS NULL "
                        " AND oreg.name = UPPER($1::text) "
                        " JOIN object o ON o.id = oreg.id JOIN registrar r ON r.id = o.clid"
                        " WHERE eot.name = 'contact' ") + (dry_run ? " " : " FOR UPDATE OF oreg")
                , Database::query_param_list(dst_contact_handle_));

            if (lock_res.size() != 1)
            {
                std::string errmsg("|| not found:dst_contact_handle: ");
                errmsg += boost::replace_all_copy(dst_contact_handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw MCEX(errmsg.c_str());
            }

            ret.dst_contact_id = static_cast<unsigned long long>(lock_res[0][0]);
            ret.dst_contact_historyid = static_cast<unsigned long long>(lock_res[0][1]);
            ret.dst_contact_roid = static_cast<std::string>(lock_res[0][2]);
            ret.dst_contact_sponsoring_registrar = static_cast<std::string>(lock_res[0][3]);
        }

        return ret;
    }//lock_object_registry_row_for_update

    void MergeContact::diff_contacts(OperationContext& ctx)
    {
        if(boost::algorithm::to_upper_copy(src_contact_handle_).compare(boost::algorithm::to_upper_copy(dst_contact_handle_)) == 0)
        {
            std::string errmsg("unable to merge the same contacts || identical:dst_contact_handle: ");
            errmsg += boost::replace_all_copy(dst_contact_handle_,"|", "[pipe]");//quote pipes
            errmsg += " |";
            throw MCEX(errmsg.c_str());
        }

        Database::Result diff_result = ctx.get_conn().exec_params(
        "SELECT "//c1.name, oreg1.name, o1.clid, c2.name, oreg2.name , o2.clid,
        " (trim(both ' ' from COALESCE(c1.name,'')) != trim(both ' ' from COALESCE(c2.name,''))) OR "
        " (trim(both ' ' from COALESCE(c1.organization,'')) != trim(both ' ' from COALESCE(c2.organization,''))) OR "
        " (trim(both ' ' from COALESCE(c1.street1,'')) != trim(both ' ' from COALESCE(c2.street1,''))) OR "
        " (trim(both ' ' from COALESCE(c1.street2,'')) != trim(both ' ' from COALESCE(c2.street2,''))) OR "
        " (trim(both ' ' from COALESCE(c1.street3,'')) != trim(both ' ' from COALESCE(c2.street3,''))) OR "
        " (trim(both ' ' from COALESCE(c1.city,'')) != trim(both ' ' from COALESCE(c2.city,''))) OR "
        " (trim(both ' ' from COALESCE(c1.postalcode,'')) != trim(both ' ' from COALESCE(c2.postalcode,''))) OR "
        " (trim(both ' ' from COALESCE(c1.stateorprovince,'')) != trim(both ' ' from COALESCE(c2.stateorprovince,''))) OR "
        " (trim(both ' ' from COALESCE(c1.country,'')) != trim(both ' ' from COALESCE(c2.country,''))) OR "
        " (trim(both ' ' from COALESCE(c1.telephone,'')) != trim(both ' ' from COALESCE(c2.telephone,''))) OR "
        " (trim(both ' ' from COALESCE(c1.fax,'')) != trim(both ' ' from COALESCE(c2.fax,''))) OR "
        " (trim(both ' ' from COALESCE(c1.email,'')) != trim(both ' ' from COALESCE(c2.email,''))) OR "
        " (trim(both ' ' from COALESCE(c1.notifyemail,'')) != trim(both ' ' from COALESCE(c2.notifyemail,''))) OR "
        " (trim(both ' ' from COALESCE(c1.vat,'')) != trim(both ' ' from COALESCE(c2.vat,''))) OR "
        " (trim(both ' ' from COALESCE(c1.ssn,'')) != trim(both ' ' from COALESCE(c2.ssn,''))) OR "
        " (COALESCE(c1.ssntype,0) != COALESCE(c2.ssntype,0)) OR "
        " (c1.disclosename != c2.disclosename) OR "
        " (c1.discloseorganization != c2.discloseorganization) OR "
        " (c1.discloseaddress != c2.discloseaddress) OR "
        " (c1.disclosetelephone != c2.disclosetelephone) OR "
        " (c1.disclosefax != c2.disclosefax) OR "
        " (c1.discloseemail != c2.discloseemail) OR "
        " (c1.disclosevat != c2.disclosevat) OR "
        " (c1.discloseident != c2.discloseident) OR "
        " (c1.disclosenotifyemail != c2.disclosenotifyemail) OR "
        " o1.clid != o2.clid "// current registrar
        "  as differ "
        " FROM (object_registry oreg1 "
        " JOIN object o1 ON oreg1.id=o1.id "
        " JOIN contact c1 ON c1.id = oreg1.id AND oreg1.name = UPPER($1::text) AND oreg1.erdate IS NULL) "
        " JOIN (object_registry oreg2 "
        " JOIN object o2 ON oreg2.id=o2.id "
        " JOIN contact c2 ON c2.id = oreg2.id AND oreg2.name = UPPER($2::text) AND oreg2.erdate IS NULL"
        ") ON TRUE "
          , Database::query_param_list(src_contact_handle_)(dst_contact_handle_));
        if (diff_result.size() != 1)
        {
            std::string errmsg("unable to get contact difference || invalid:src_contact_handle: ");
            errmsg += boost::replace_all_copy(src_contact_handle_,"|", "[pipe]");//quote pipes
            errmsg += " | invalid:dst_contact_handle: ";
            errmsg += boost::replace_all_copy(dst_contact_handle_,"|", "[pipe]");//quote pipes
            errmsg += " |";
            throw MCEX(errmsg.c_str());
        }
        bool contact_differs = static_cast<bool>(diff_result[0][0]);
        if(contact_differs)
        {
            std::string errmsg("contacts differ || invalid:src_contact_handle: ");
            errmsg += boost::replace_all_copy(src_contact_handle_,"|", "[pipe]");//quote pipes
            errmsg += " | invalid:dst_contact_handle: ";
            errmsg += boost::replace_all_copy(dst_contact_handle_,"|", "[pipe]");//quote pipes
            errmsg += " |";
            throw MCEX(errmsg.c_str());
        }
    }//diff_contacts

    MergeContactOutput MergeContact::merge_contact_impl(OperationContext& ctx, bool dry_run)
    {
        MergeContactOutput output;

        //domain_registrant lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                std::string("SELECT oreg.name, r.handle, d.id "
                " FROM contact src_c "
                " JOIN object_registry src_oreg ON src_c.id = src_oreg.id "
                " AND src_oreg.name = UPPER($1::text) AND src_oreg.erdate IS NULL"
                " JOIN domain d ON d.registrant = src_c.id "
                " JOIN object_registry oreg  ON oreg.id = d.id AND oreg.erdate IS NULL "
                " JOIN object o ON oreg.id = o.id "
                " JOIN registrar r ON o.clid = r.id") + (dry_run ? " " : " FOR UPDATE OF oreg")
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                MergeContactUpdateDomainRegistrant tmp;
                tmp.fqdn = std::string(result[i][0]);
                tmp.domain_id = static_cast<unsigned long long>(result[i][2]);
                tmp.sponsoring_registrar = std::string(result[i][1]);
                tmp.set_registrant = dst_contact_handle_;

                if(!dry_run)
                {
                    std::string fqdn = std::string(result[i][0]);
                    UpdateDomain ud (fqdn, registrar_ );
                    ud.set_registrant(dst_contact_handle_);
                    if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_);
                    tmp.history_id = ud.exec(ctx);
                }
                output.update_domain_registrant.push_back(tmp);
            }//for
        }

        //domain_admin lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                std::string("SELECT oreg.name, r.handle, d.id "
                " FROM contact src_c "
                " JOIN object_registry src_oreg ON src_c.id = src_oreg.id "
                " AND src_oreg.name = UPPER($1::text) AND src_oreg.erdate IS NULL"
                " JOIN domain_contact_map dcm ON dcm.role = 1 "
                " AND dcm.contactid  = src_c.id "
                " JOIN domain d ON dcm.domainid = d.id "
                " JOIN object_registry oreg ON oreg.id = d.id AND oreg.erdate IS NULL "
                " JOIN object o ON oreg.id = o.id "
                " JOIN registrar r ON o.clid = r.id") + (dry_run ? " " : " FOR UPDATE OF oreg")
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                MergeContactUpdateDomainAdminContact tmp;
                tmp.fqdn = std::string(result[i][0]);
                tmp.domain_id = static_cast<unsigned long long>(result[i][2]);
                tmp.sponsoring_registrar = std::string(result[i][1]);
                tmp.rem_admin_contact = src_contact_handle_;
                tmp.add_admin_contact = dst_contact_handle_;

                if(!dry_run)
                {
                    try
                    {
                        UpdateDomain ud (tmp.fqdn, registrar_ );
                        ud.rem_admin_contact(src_contact_handle_)
                        .add_admin_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_);
                        tmp.history_id = ud.exec(ctx);
                    }
                    catch(UpdateDomainException& ex)
                    {
                        GetOperationExceptionParamsDataToBoolCallback cb;
                        //look for already set: admin contact
                        ex.callback_exception_params(boost::ref(cb),"already set:admin contact");
                        //if found ignore exception, if not found rethrow exception
                        if(cb.get())
                        {
                            //only remove source admin contact, dest admin contact is already there
                            UpdateDomain ud (tmp.fqdn, registrar_ );
                            ud.rem_admin_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_);
                            tmp.history_id = ud.exec(ctx);
                        }
                        else
                        {
                            throw;
                        }
                    }
                }
                output.update_domain_admin_contact.push_back(tmp);
            }//for
        }

        //nsset_tech lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                std::string("SELECT oreg.name, r.handle, n.id "
                " FROM contact src_c "
                " JOIN object_registry src_oreg ON src_c.id = src_oreg.id "
                " AND src_oreg.name = UPPER($1::text) AND src_oreg.erdate IS NULL "
                " JOIN nsset_contact_map ncm ON ncm.contactid  = src_c.id "
                " JOIN nsset n ON ncm.nssetid = n.id "
                " JOIN object_registry oreg  ON oreg.id = n.id AND oreg.erdate IS NULL "
                " JOIN object o ON oreg.id = o.id "
                " JOIN registrar r ON o.clid = r.id") + (dry_run ? " " : " FOR UPDATE OF oreg")
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                MergeContactUpdateNssetTechContact tmp;
                tmp.handle = std::string(result[i][0]);
                tmp.nsset_id = static_cast<unsigned long long>(result[i][2]);
                tmp.sponsoring_registrar = std::string(result[i][1]);
                tmp.rem_tech_contact = src_contact_handle_;
                tmp.add_tech_contact = dst_contact_handle_;

                if(!dry_run)
                {
                    try
                    {
                        UpdateNsset un(tmp.handle, registrar_ );
                        un.rem_tech_contact(src_contact_handle_)
                        .add_tech_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) un.set_logd_request_id(logd_request_id_);
                        tmp.history_id = un.exec(ctx);
                    }
                    catch(UpdateNssetException& ex)
                    {
                        GetOperationExceptionParamsDataToBoolCallback cb;
                        //look for already set: tech contact
                        ex.callback_exception_params(boost::ref(cb),"already set:tech contact");
                        //if found ignore exception, if not found rethrow exception
                        if(cb.get())
                        {
                            //only remove source tech contact, dest tech contact is already there
                            UpdateNsset un(tmp.handle, registrar_ );
                            un.rem_tech_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) un.set_logd_request_id(logd_request_id_);
                            tmp.history_id = un.exec(ctx);
                        }
                        else
                        {
                            throw;
                        }
                    }
                }
                output.update_nsset_tech_contact.push_back(tmp);
            }//for
        }

        //keyset_tech lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                std::string("SELECT oreg.name, r.handle, k.id "
                " FROM contact src_c "
                " JOIN object_registry src_oreg ON src_c.id = src_oreg.id "
                " AND src_oreg.name = UPPER($1::text) AND src_oreg.erdate IS NULL"
                " JOIN keyset_contact_map kcm ON kcm.contactid  = src_c.id "
                " JOIN keyset k ON kcm.keysetid = k.id "
                " JOIN object_registry oreg  ON oreg.id = k.id AND oreg.erdate IS NULL"
                " JOIN object o ON oreg.id = o.id "
                " JOIN registrar r ON o.clid = r.id") + (dry_run ? " " : " FOR UPDATE OF oreg")
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                MergeContactUpdateKeysetTechContact tmp;
                tmp.handle = std::string(result[i][0]);
                tmp.keyset_id = static_cast<unsigned long long>(result[i][2]);
                tmp.sponsoring_registrar = std::string(result[i][1]);
                tmp.rem_tech_contact = src_contact_handle_;
                tmp.add_tech_contact = dst_contact_handle_;

                if(!dry_run)
                {
                    try
                    {
                        UpdateKeyset uk(tmp.handle, registrar_);
                        uk.rem_tech_contact(src_contact_handle_)
                        .add_tech_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) uk.set_logd_request_id(logd_request_id_);
                        tmp.history_id = uk.exec(ctx);
                    }
                    catch(UpdateKeysetException& ex)
                    {
                        GetOperationExceptionParamsDataToBoolCallback cb;
                        //look for already set: tech contact
                        ex.callback_exception_params(boost::ref(cb),"already set:tech contact");
                        //if found ignore exception, if not found rethrow exception
                        if(cb.get())
                        {
                            //only remove source tech contact, dest tech contact is already there
                            UpdateKeyset uk(tmp.handle, registrar_);
                            uk.rem_tech_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) uk.set_logd_request_id(logd_request_id_);
                            tmp.history_id = uk.exec(ctx);
                        }
                        else
                        {
                            throw;
                        }
                    }
                }
                output.update_keyset_tech_contact.push_back(tmp);
            }//for
        }

        //delete src contact
        if(!dry_run)
        {
            DeleteContact(src_contact_handle_).exec(ctx);
        }

        return output;
    }//merge_contact_impl

    MergeContactOutput MergeContact::exec_dry_run(OperationContext& ctx)
    {
        const bool dry_run = true;
        try
        {
            //lock object_registry row for update
            MergeContactLockedContactId locked_contact = lock_object_registry_row_for_update(ctx,dry_run);

            //diff contacts
            diff_contacts(ctx);

            MergeContactOutput out = merge_contact_impl(ctx, dry_run);
            out.contactid = locked_contact;
            return out;
        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<MergeContactException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }
        return MergeContactOutput();
    }//MergeContact::exec_dry_run

    MergeContactOutput MergeContact::exec(OperationContext& ctx)
    {
        try
        {
            const bool dry_run = false;

            //lock object_registry row for update
            MergeContactLockedContactId locked_contact = lock_object_registry_row_for_update(ctx,dry_run);

            //diff contacts
            diff_contacts(ctx);

            MergeContactOutput out = merge_contact_impl(ctx, dry_run);
            out.contactid = locked_contact;
            return out;
        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<MergeContactException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }
        return MergeContactOutput();
    }//MergeContact::exec

}//namespace Fred

