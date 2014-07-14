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
 *  @file
 *  contact merge
 */

#include <string>

#include <boost/algorithm/string.hpp>

#include "src/fredlib/contact/merge_contact.h"

#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/poll/create_update_object_poll_message.h"
#include "src/fredlib/poll/create_delete_contact_poll_message.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/random.h"


namespace Fred
{
    MergeContact::MergeContact(const std::string& from_contact_handle, const std::string& to_contact_handle, const std::string& registrar,
            DiffContacts diff_contacts_impl)
    : src_contact_handle_(from_contact_handle)
    , dst_contact_handle_(to_contact_handle)
    , registrar_(registrar)
    , diff_contacts_impl_(diff_contacts_impl)
    {
        if(boost::algorithm::to_upper_copy(src_contact_handle_).compare(boost::algorithm::to_upper_copy(dst_contact_handle_)) == 0)
        {
            BOOST_THROW_EXCEPTION(Exception().set_identical_contacts_handle(dst_contact_handle_));
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

            if (lock_res.size() == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_source_contact_handle(src_contact_handle_));
            }
            if (lock_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to get source contact"));
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

            if (lock_res.size() == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_destination_contact_handle(dst_contact_handle_));
            }
            if (lock_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to get destination contact"));
            }

            ret.dst_contact_id = static_cast<unsigned long long>(lock_res[0][0]);
            ret.dst_contact_historyid = static_cast<unsigned long long>(lock_res[0][1]);
            ret.dst_contact_roid = static_cast<std::string>(lock_res[0][2]);
            ret.dst_contact_sponsoring_registrar = static_cast<std::string>(lock_res[0][3]);
        }

        if(ret.src_contact_roid.compare(ret.dst_contact_roid) == 0)
        {
            BOOST_THROW_EXCEPTION(Exception().set_identical_contacts_roid(ret.dst_contact_roid));
        }

        return ret;
    }//lock_object_registry_row_for_update

    void MergeContact::diff_contacts(OperationContext& ctx)
    {
        if(diff_contacts_impl_)
        {
            bool contact_differs = diff_contacts_impl_(ctx,src_contact_handle_, dst_contact_handle_);

            if(contact_differs)
            {
                BOOST_THROW_EXCEPTION(Exception().set_contacts_differ(
                    InvalidContacts(src_contact_handle_,dst_contact_handle_)));
            }
        }
        else
        {
            BOOST_THROW_EXCEPTION(Exception().set_unable_to_get_difference_of_contacts(
                InvalidContacts(src_contact_handle_,dst_contact_handle_)));
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

                //check if object blocked
                if(Fred::ObjectHasState(tmp.domain_id,Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx)
                    || Fred::ObjectHasState(tmp.domain_id,Fred::ObjectState::SERVER_BLOCKED).exec(ctx))
                {
                    BOOST_THROW_EXCEPTION(Fred::MergeContact::Exception().set_object_blocked(tmp.fqdn));
                }

                if(!dry_run)
                {
                    std::string fqdn = std::string(result[i][0]);
                    UpdateDomain ud (fqdn, registrar_ );
                    ud.set_registrant(dst_contact_handle_);
                    if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_.get_value());
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

                //check if object blocked
                if(Fred::ObjectHasState(tmp.domain_id,Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx)
                    || Fred::ObjectHasState(tmp.domain_id,Fred::ObjectState::SERVER_BLOCKED).exec(ctx))
                {
                    BOOST_THROW_EXCEPTION(Fred::MergeContact::Exception().set_object_blocked(tmp.fqdn));
                }

                if(!dry_run)
                {
                    try
                    {
                        ctx.get_conn().exec("SAVEPOINT merge_contact_update_domain");
                        UpdateDomain ud (tmp.fqdn, registrar_ );
                        ud.rem_admin_contact(src_contact_handle_)
                        .add_admin_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_.get_value());
                        tmp.history_id = ud.exec(ctx);
                        ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_domain");
                    }
                    catch(UpdateDomain::Exception& ex)
                    {
                        //look for already set: admin contact
                        //if found ignore exception, if not found rethrow exception
                        if(ex.is_set_vector_of_already_set_admin_contact_handle()
                            && (ex.get_vector_of_already_set_admin_contact_handle().at(0) == dst_contact_handle_))//check colliding contact handle
                        {
                            //only remove source admin contact, dest admin contact is already there
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT merge_contact_update_domain");
                            UpdateDomain ud (tmp.fqdn, registrar_ );
                            ud.rem_admin_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_.get_value());
                            tmp.history_id = ud.exec(ctx);
                            ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_domain");
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

                //check if object blocked
                if(Fred::ObjectHasState(tmp.nsset_id,Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx))
                {
                    BOOST_THROW_EXCEPTION(Fred::MergeContact::Exception().set_object_blocked(tmp.handle));
                }

                if(!dry_run)
                {
                    try
                    {
                        ctx.get_conn().exec("SAVEPOINT merge_contact_update_nsset");
                        UpdateNsset un(tmp.handle, registrar_ );
                        un.rem_tech_contact(src_contact_handle_)
                        .add_tech_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) un.set_logd_request_id(logd_request_id_.get_value());
                        tmp.history_id = un.exec(ctx);
                        ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_nsset");
                    }
                    catch(UpdateNsset::Exception& ex)
                    {
                        //look for already set: tech contact
                        //if found ignore exception, if not found rethrow exception
                        if(ex.is_set_vector_of_already_set_technical_contact_handle()
                            && (ex.get_vector_of_already_set_technical_contact_handle().at(0) == dst_contact_handle_))//check colliding contact handle
                        {
                            //only remove source tech contact, dest tech contact is already there
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT merge_contact_update_nsset");
                            UpdateNsset un(tmp.handle, registrar_ );
                            un.rem_tech_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) un.set_logd_request_id(logd_request_id_.get_value());
                            tmp.history_id = un.exec(ctx);
                            ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_nsset");
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

                //check if object blocked
                if(Fred::ObjectHasState(tmp.keyset_id,Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx))
                {
                    BOOST_THROW_EXCEPTION(Fred::MergeContact::Exception().set_object_blocked(tmp.handle));
                }

                if(!dry_run)
                {
                    try
                    {
                        ctx.get_conn().exec("SAVEPOINT merge_contact_update_keyset");
                        UpdateKeyset uk(tmp.handle, registrar_);
                        uk.rem_tech_contact(src_contact_handle_)
                        .add_tech_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) uk.set_logd_request_id(logd_request_id_.get_value());
                        tmp.history_id = uk.exec(ctx);
                        ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_keyset");
                    }
                    catch(UpdateKeyset::Exception& ex)
                    {
                        if(ex.is_set_vector_of_already_set_technical_contact_handle()
                            && (ex.get_vector_of_already_set_technical_contact_handle().at(0) == dst_contact_handle_))//check colliding contact handle
                        {
                            //only remove source tech contact, dest tech contact is already there
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT merge_contact_update_keyset");
                            UpdateKeyset uk(tmp.handle, registrar_);
                            uk.rem_tech_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) uk.set_logd_request_id(logd_request_id_.get_value());
                            tmp.history_id = uk.exec(ctx);
                            ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_keyset");
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
            DeleteContactByHandle(src_contact_handle_).exec(ctx);
            /* #9877 - change authinfo of destination contact */
            std::string new_authinfo =  Random::string_from(8, "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789");
            UpdateContactByHandle ucbh (dst_contact_handle_, registrar_);
                if(logd_request_id_.isset()) ucbh.set_logd_request_id(logd_request_id_.get_value());
                ucbh.set_authinfo(new_authinfo)
                .exec(ctx);
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
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
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
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return MergeContactOutput();
    }//MergeContact::exec

    std::string MergeContact::to_string() const
    {
        return Util::format_operation_state("MergeContact",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("src_contact_handle",src_contact_handle_))
        (std::make_pair("dst_contact_handle",dst_contact_handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        (std::make_pair("diff_contacts_impl",
            (static_cast<bool>(diff_contacts_impl_) == false) ? "not set" : "set"))
        );
    }

    bool MergeContact::DefaultDiffContacts::operator()(OperationContext& ctx,
            const std::string& src_contact_handle, const std::string& dst_contact_handle) const
    {
        if(boost::algorithm::to_upper_copy(src_contact_handle).compare(boost::algorithm::to_upper_copy(dst_contact_handle)) == 0)
        {
            BOOST_THROW_EXCEPTION(MergeContact::Exception().set_identical_contacts_handle(dst_contact_handle));
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
        "  as differ, c1.id AS src_contact_id, c2.id AS dst_contact_id"
        " FROM (object_registry oreg1 "
        " JOIN object o1 ON oreg1.id=o1.id "
        " JOIN contact c1 ON c1.id = oreg1.id AND oreg1.name = UPPER($1::text) AND oreg1.erdate IS NULL) "
        " JOIN (object_registry oreg2 "
        " JOIN object o2 ON oreg2.id=o2.id "
        " JOIN contact c2 ON c2.id = oreg2.id AND oreg2.name = UPPER($2::text) AND oreg2.erdate IS NULL"
        ") ON TRUE "
          , Database::query_param_list(src_contact_handle)(dst_contact_handle));
        if (diff_result.size() != 1)
        {
            BOOST_THROW_EXCEPTION(MergeContact::Exception().set_unable_to_get_difference_of_contacts(
                    MergeContact::InvalidContacts(src_contact_handle,dst_contact_handle)));
        }

        unsigned long long dst_contact_id = static_cast<unsigned long long>(diff_result[0]["dst_contact_id"]);

        if(Fred::ObjectHasState(dst_contact_id,Fred::ObjectState::SERVER_BLOCKED).exec(ctx))
        {
            BOOST_THROW_EXCEPTION(Fred::MergeContact::Exception().set_src_contact_in_mojeid(
                Fred::MergeContact::InvalidContacts(src_contact_handle,dst_contact_handle)));
        }

        unsigned long long src_contact_id = static_cast<unsigned long long>(diff_result[0]["src_contact_id"]);

        if(Fred::ObjectHasState(src_contact_id,Fred::ObjectState::MOJEID_CONTACT).exec(ctx))
        {
            BOOST_THROW_EXCEPTION(Fred::MergeContact::Exception().set_src_contact_in_mojeid(
                Fred::MergeContact::InvalidContacts(src_contact_handle,dst_contact_handle)));
        }

        if(Fred::ObjectHasState(src_contact_id,Fred::ObjectState::SERVER_BLOCKED).exec(ctx)
            || Fred::ObjectHasState(src_contact_id,Fred::ObjectState::SERVER_DELETE_PROHIBITED).exec(ctx))
        {
            BOOST_THROW_EXCEPTION(Fred::MergeContact::Exception().set_src_contact_blocked(
                Fred::MergeContact::InvalidContacts(src_contact_handle,dst_contact_handle)));
        }

        bool contact_differs = static_cast<bool>(diff_result[0]["differ"]);
        return contact_differs;
    }


    void create_poll_messages(const Fred::MergeContactOutput &_merge_data, Fred::OperationContext &_ctx)
    {
        for (std::vector<Fred::MergeContactUpdateDomainRegistrant>::const_iterator i = _merge_data.update_domain_registrant.begin();
                i != _merge_data.update_domain_registrant.end(); ++i)
        {
            Fred::Poll::CreateUpdateObjectPollMessage(i->history_id.get_value()).exec(_ctx);
        }
        for (std::vector<Fred::MergeContactUpdateDomainAdminContact>::const_iterator i = _merge_data.update_domain_admin_contact.begin();
                i != _merge_data.update_domain_admin_contact.end(); ++i)
        {
            Fred::Poll::CreateUpdateObjectPollMessage(i->history_id.get_value()).exec(_ctx);
        }
        for (std::vector<Fred::MergeContactUpdateNssetTechContact>::const_iterator i = _merge_data.update_nsset_tech_contact.begin();
                i != _merge_data.update_nsset_tech_contact.end(); ++i)
        {
            Fred::Poll::CreateUpdateObjectPollMessage(i->history_id.get_value()).exec(_ctx);
        }
        for (std::vector<Fred::MergeContactUpdateKeysetTechContact>::const_iterator i = _merge_data.update_keyset_tech_contact.begin();
                i != _merge_data.update_keyset_tech_contact.end(); ++i)
        {
            Fred::Poll::CreateUpdateObjectPollMessage(i->history_id.get_value()).exec(_ctx);
        }
        Fred::Poll::CreateDeleteContactPollMessage(_merge_data.contactid.src_contact_historyid).exec(_ctx);
    }




}//namespace Fred

