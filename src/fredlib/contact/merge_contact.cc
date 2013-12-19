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
#include "util/random.h"

/*
 * XXX: #9877 - temoporary (and partial implementation of update contact - authinfo change only)
 * this class should be deleted where full UpdateContact...(...) operation is present
 */
namespace Fred
{
    class UpdateContactByHandle
    {
    private:
        const std::string handle_;
        const std::string registrar_;
        Optional<std::string> authinfo_;
        Nullable<unsigned long long> logd_request_id_;


    public:
        UpdateContactByHandle(const std::string &_handle, const std::string _registrar)
            : handle_(_handle),
              registrar_(_registrar)
        {
        }


        UpdateContactByHandle& set_authinfo(const std::string &_authinfo)
        {
            authinfo_ = _authinfo;
            return *this;
        }


        UpdateContactByHandle& set_logd_request_id(unsigned long long _logd_request_id)
        {
            logd_request_id_ = _logd_request_id;
            return *this;
        }


        void exec(OperationContext &ctx)
        {
            Database::Result r_dst_contact_id = ctx.get_conn().exec_params(
                    "SELECT id FROM object_registry oreg WHERE type = 1"
                    " AND erdate is null AND name = $1::text",
                    Database::query_param_list(handle_));
            unsigned long long dst_contact_id = 0;
            if (r_dst_contact_id.size() == 1) {
                dst_contact_id = static_cast<unsigned long long>(r_dst_contact_id[0][0]);
            }
            if (dst_contact_id == 0) {
                throw std::runtime_error("destination contact id look up failed");
            }

            /* update object table */
            {
                Database::QueryParams p_update_object;
                std::stringstream s_update_object;

                p_update_object.push_back(registrar_);
                s_update_object << "UPDATE object SET upid = "
                    "(SELECT id FROM registrar WHERE handle = $" <<  p_update_object.size() << "::text), update = now()";
                if (authinfo_.isset())
                {
                    p_update_object.push_back(authinfo_.get_value());
                    s_update_object << ", authinfopw = $" << p_update_object.size() << "::text";
                }
                p_update_object.push_back(dst_contact_id);
                s_update_object << " WHERE id = $" << p_update_object.size() << "::integer";

                ctx.get_conn().exec_params(s_update_object.str(), p_update_object);
            }

            /* insert into history tables */
            {
                Database::Result rhistory = ctx.get_conn().exec_params(
                        "INSERT INTO history (id, request_id)"
                        " VALUES (DEFAULT, $1::bigint) RETURNING id",
                        Database::query_param_list(logd_request_id_));
                unsigned long long history_id = 0;
                if (rhistory.size() == 1) {
                    history_id = static_cast<unsigned long long>(rhistory[0][0]);
                }
                if (history_id == 0) {
                    throw std::runtime_error("cannot save new history");
                }

                ctx.get_conn().exec_params("UPDATE object_registry SET historyid = $1::integer"
                        " WHERE id = $2::integer",
                        Database::query_param_list(history_id)(dst_contact_id));

                Database::Result robject_history = ctx.get_conn().exec_params(
                        "INSERT INTO object_history (historyid, id, clid, upid, trdate, update, authinfopw)"
                        " SELECT $1::integer, o.id, o.clid, o.upid, o.trdate, o.update, o.authinfopw"
                        " FROM object o"
                        " WHERE o.id = $2::integer",
                        Database::query_param_list(history_id)(dst_contact_id));

                Database::Result rcontact_history = ctx.get_conn().exec_params(
                        "INSERT INTO contact_history (historyid, id, name, organization, street1, street2, street3,"
                        " city, stateorprovince, postalcode, country, telephone, fax, email, disclosename,"
                        " discloseorganization, discloseaddress, disclosetelephone, disclosefax, discloseemail,"
                        " notifyemail, vat, ssn, ssntype, disclosevat, discloseident, disclosenotifyemail)"
                        " SELECT $1::integer, c.id, c.name, c.organization, c.street1, c.street2, c.street3,"
                        " c.city, c.stateorprovince, c.postalcode, c.country, c.telephone, c.fax, c.email,"
                        " c.disclosename, c.discloseorganization, c.discloseaddress, c.disclosetelephone, c.disclosefax,"
                        " c.discloseemail, c.notifyemail, c.vat, c.ssn, c.ssntype, c.disclosevat, c.discloseident,"
                        " c.disclosenotifyemail FROM contact c WHERE c.id = $2::integer",
                        Database::query_param_list(history_id)(dst_contact_id));
            }
        }
    };
}


namespace Fred
{
    MergeContact::MergeContact(const std::string& from_contact_handle, const std::string& to_contact_handle, const std::string& registrar)
    : src_contact_handle_(from_contact_handle)
    , dst_contact_handle_(to_contact_handle)
    , registrar_(registrar)
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
        if(boost::algorithm::to_upper_copy(src_contact_handle_).compare(boost::algorithm::to_upper_copy(dst_contact_handle_)) == 0)
        {
            BOOST_THROW_EXCEPTION(Exception().set_identical_contacts_handle(dst_contact_handle_));
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
            BOOST_THROW_EXCEPTION(Exception().set_unable_to_get_difference_of_contacts(
                    InvalidContacts(src_contact_handle_,dst_contact_handle_)));
        }
        bool contact_differs = static_cast<bool>(diff_result[0][0]);
        if(contact_differs)
        {
            BOOST_THROW_EXCEPTION(Exception().set_contacts_differ(
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
                        ctx.get_conn().exec("SAVEPOINT merge_contact_update_domain");
                        UpdateDomain ud (tmp.fqdn, registrar_ );
                        ud.rem_admin_contact(src_contact_handle_)
                        .add_admin_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_);
                        tmp.history_id = ud.exec(ctx);
                        ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_domain");
                    }
                    catch(UpdateDomain::Exception& ex)
                    {
                        //look for already set: admin contact
                        //if found ignore exception, if not found rethrow exception
                        if(ex.is_set_vector_of_already_set_admin_contact_handle())
                        {
                            //only remove source admin contact, dest admin contact is already there
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT merge_contact_update_domain");
                            UpdateDomain ud (tmp.fqdn, registrar_ );
                            ud.rem_admin_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) ud.set_logd_request_id(logd_request_id_);
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

                if(!dry_run)
                {
                    try
                    {
                        ctx.get_conn().exec("SAVEPOINT merge_contact_update_nsset");
                        UpdateNsset un(tmp.handle, registrar_ );
                        un.rem_tech_contact(src_contact_handle_)
                        .add_tech_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) un.set_logd_request_id(logd_request_id_);
                        tmp.history_id = un.exec(ctx);
                        ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_nsset");
                    }
                    catch(UpdateNsset::Exception& ex)
                    {
                        //look for already set: tech contact
                        //if found ignore exception, if not found rethrow exception
                        if(ex.is_set_vector_of_already_set_technical_contact_handle())
                        {
                            //only remove source tech contact, dest tech contact is already there
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT merge_contact_update_nsset");
                            UpdateNsset un(tmp.handle, registrar_ );
                            un.rem_tech_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) un.set_logd_request_id(logd_request_id_);
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

                if(!dry_run)
                {
                    try
                    {
                        ctx.get_conn().exec("SAVEPOINT merge_contact_update_keyset");
                        UpdateKeyset uk(tmp.handle, registrar_);
                        uk.rem_tech_contact(src_contact_handle_)
                        .add_tech_contact(dst_contact_handle_);
                        if(logd_request_id_.isset()) uk.set_logd_request_id(logd_request_id_);
                        tmp.history_id = uk.exec(ctx);
                        ctx.get_conn().exec("RELEASE SAVEPOINT merge_contact_update_keyset");
                    }
                    catch(UpdateKeyset::Exception& ex)
                    {
                        if(ex.is_set_vector_of_already_set_technical_contact_handle())
                        {
                            //only remove source tech contact, dest tech contact is already there
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT merge_contact_update_keyset");
                            UpdateKeyset uk(tmp.handle, registrar_);
                            uk.rem_tech_contact(src_contact_handle_);
                            if(logd_request_id_.isset()) uk.set_logd_request_id(logd_request_id_);
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
            DeleteContact(src_contact_handle_).exec(ctx);
            /* #9877 - change authinfo of destination contact */
            std::string new_authinfo =  Random::string_from(8, "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789");
            UpdateContactByHandle(dst_contact_handle_, registrar_)
                .set_logd_request_id(logd_request_id_)
                .set_authinfo(new_authinfo)
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

    std::ostream& operator<<(std::ostream& os, const MergeContact& i)
    {
        return os << "#MergeContact src_contact_handle_: " << i.src_contact_handle_
                << " dst_contact_handle_: " << i.dst_contact_handle_
                << " registrar_: " << i.registrar_
                << " logd_request_id_: " << i.logd_request_id_.print_quoted()
                ;
    }

    std::string MergeContact::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred

