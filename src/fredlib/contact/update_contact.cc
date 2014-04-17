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
 *  contact update
 */

#include <string>
#include <vector>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/contact/contact_enum.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    ContactAddressToUpdate::ContactAddressToUpdate(const ContactAddressToUpdate &_src)
    :   to_update_(_src.to_update_),
        to_remove_(_src.to_remove_)
    {
    }

    template < ContactAddressType::Value type >
    ContactAddressToUpdate& ContactAddressToUpdate::update(const struct ContactAddress &_address)
    {
        to_update_[type] = _address;
        to_remove_.erase(type);
        return *this;
    }

    template < ContactAddressType::Value type >
    ContactAddressToUpdate& ContactAddressToUpdate::remove()
    {
        to_remove_.insert(type);
        to_update_.erase(type);
        return *this;
    }

    template ContactAddressToUpdate& ContactAddressToUpdate::
    update< ContactAddressType::MAILING >(const struct ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    update< ContactAddressType::BILLING >(const struct ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    update< ContactAddressType::SHIPPING >(const struct ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::MAILING >();

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::BILLING >();

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::SHIPPING >();

    std::string ContactAddressToUpdate::to_string()const
    {
        std::ostringstream out;
        if (!to_update_.empty()) {
            out << "to_update:{";
            for (ToUpdate::const_iterator addr_ptr = to_update_.begin();
                 addr_ptr != to_update_.end(); ++addr_ptr) {
                if (addr_ptr != to_update_.begin()) {
                    out << ",";
                }
                out << addr_ptr->first << ":" << addr_ptr->second;
            }
            out << "}";
        }
        if (!to_remove_.empty()) {
            if (!out.str().empty()) {
                out << " ";
            }
            out << "to_remove:{";
            for (ToRemove::const_iterator type_ptr = to_remove_.begin();
                 type_ptr != to_remove_.end(); ++type_ptr) {
                if (type_ptr != to_remove_.begin()) {
                    out << ",";
                }
                out << (*type_ptr);
            }
            out << "}";
        }
        return out.str();
    }

    template < class DERIVED >
    UpdateContact< DERIVED >::UpdateContact(const std::string& registrar
        , const Optional<std::string>& sponsoring_registrar
        , const Optional<std::string>& authinfo
        , const Optional<std::string>& name
        , const Optional<std::string>& organization
        , const Optional< Fred::Contact::PlaceAddress > &place
        , const Optional<std::string>& telephone
        , const Optional<std::string>& fax
        , const Optional<std::string>& email
        , const Optional<std::string>& notifyemail
        , const Optional<std::string>& vat
        , const Optional<std::string>& ssntype
        , const Optional<std::string>& ssn
        , const ContactAddressToUpdate &addresses
        , const Optional<bool>& disclosename
        , const Optional<bool>& discloseorganization
        , const Optional<bool>& discloseaddress
        , const Optional<bool>& disclosetelephone
        , const Optional<bool>& disclosefax
        , const Optional<bool>& discloseemail
        , const Optional<bool>& disclosevat
        , const Optional<bool>& discloseident
        , const Optional<bool>& disclosenotifyemail
        , const Optional<unsigned long long>& logd_request_id
        )
    :   registrar_(registrar)
    ,   sponsoring_registrar_(sponsoring_registrar)
    ,   authinfo_(authinfo)
    ,   name_(name)
    ,   organization_(organization)
    ,   place_(place)
    ,   telephone_(telephone)
    ,   fax_(fax)
    ,   email_(email)
    ,   notifyemail_(notifyemail)
    ,   vat_(vat)
    ,   ssntype_(ssntype)
    ,   ssn_(ssn)
    ,   addresses_(addresses)
    ,   disclosename_(disclosename)
    ,   discloseorganization_(discloseorganization)
    ,   discloseaddress_(discloseaddress)
    ,   disclosetelephone_(disclosetelephone)
    ,   disclosefax_(disclosefax)
    ,   discloseemail_(discloseemail)
    ,   disclosevat_(disclosevat)
    ,   discloseident_(discloseident)
    ,   disclosenotifyemail_(disclosenotifyemail)
    ,   logd_request_id_(logd_request_id.isset()
            ? Nullable<unsigned long long>(logd_request_id.get_value())
            : Nullable<unsigned long long>())//is NULL if not set
    {}

    template < class DERIVED >
    unsigned long long UpdateContact< DERIVED >::exec(OperationContext& ctx
        , const InfoContactOutput& contact)
    {
        unsigned long long history_id = 0;

        Exception update_contact_exception;
        //update object
        try {
            history_id = UpdateObject(contact.info_contact_data.handle,
                "contact",registrar_,sponsoring_registrar_, authinfo_,
                logd_request_id_).exec(ctx);
        }
        catch(const UpdateObject::Exception& ex) {
            if(ex.is_set_unknown_registrar_handle()) {
                //fatal good path, need valid registrar performing update
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(ex.get_unknown_registrar_handle()));
            }
            else if(ex.is_set_unknown_object_handle()
                 || ex.is_set_unknown_sponsoring_registrar_handle()) {//non-fatal good path, update can continue to check input
                if(ex.is_set_unknown_object_handle()) {
                    update_contact_exception.set_unknown_contact_handle(contact.info_contact_data.handle);
                }
                if(ex.is_set_unknown_sponsoring_registrar_handle()) {
                    update_contact_exception.set_unknown_sponsoring_registrar_handle(
                        ex.get_unknown_sponsoring_registrar_handle());
                }
            }
            else {
                throw;//rethrow unexpected
            }
        }

        //update contact
        {
            Database::QueryParams params;//query params
            std::ostringstream sql;
            Util::HeadSeparator set_separator("SET ",", ");
            sql << "UPDATE contact ";

            if (name_.isset()) {
                params.push_back(name_.get_value());
                sql << set_separator.get() << "name = $" << params.size() << "::text ";
            }

            if (organization_.isset()) {
                params.push_back(organization_.get_value());
                sql << set_separator.get() << "organization = $" << params.size() << "::text ";
            }

            if (place_.isset()) {
                const Fred::Contact::PlaceAddress &place = place_.get_value();
                params.push_back(place.street1);
                sql << set_separator.get() << "street1 = $" << params.size() << "::text ";

                if (place.street2.isset()) {
                    params.push_back(place.street2.get_value());
                    sql << set_separator.get() << "street2 = $" << params.size() << "::text ";
                }

                if (place.street3.isset()) {
                    params.push_back(place.street3.get_value());
                    sql << set_separator.get() << "street3 = $" << params.size() << "::text ";
                }

                params.push_back(place.city);
                sql << set_separator.get() << "city = $" << params.size() << "::text ";

                if (place.stateorprovince.isset()) {
                    params.push_back(place.stateorprovince.get_value());
                    sql << set_separator.get() << "stateorprovince = $" << params.size() << "::text ";
                }

                params.push_back(place.postalcode);
                sql << set_separator.get() << "postalcode = $" << params.size() << "::text ";

                params.push_back(Contact::get_country_code(place.country, ctx, &update_contact_exception, &Exception::set_unknown_country));
                sql << set_separator.get() << "country = $" << params.size() << "::text ";
            }

            if (telephone_.isset()) {
                params.push_back(telephone_.get_value());
                sql << set_separator.get() << "telephone = $" << params.size() << "::text ";
            }

            if (fax_.isset()) {
                params.push_back(fax_.get_value());
                sql << set_separator.get() << "fax = $" << params.size() << "::text ";
            }

            if (email_.isset()) {
                params.push_back(email_.get_value());
                sql << set_separator.get() << "email = $" << params.size() << "::text ";
            }

            if (notifyemail_.isset()) {
                params.push_back(notifyemail_.get_value());
                sql << set_separator.get() << "notifyemail = $" << params.size() << "::text ";
            }

            if (vat_.isset()) {
                params.push_back(vat_.get_value());
                sql << set_separator.get() << "vat = $" << params.size() << "::text ";
            }

            if (ssntype_.isset()) {
                params.push_back(Contact::get_ssntype_id(ssntype_,ctx, &update_contact_exception, &Exception::set_unknown_ssntype));
                sql << set_separator.get() << "ssntype = $" << params.size() << "::integer ";
            }

            if (ssn_.isset()) {
                params.push_back(ssn_.get_value());
                sql << set_separator.get() << "ssn = $" << params.size() << "::text ";
            }

            if (disclosename_.isset()) {
                params.push_back(disclosename_.get_value());
                sql << set_separator.get() << "disclosename = $" << params.size() << "::boolean ";
            }

            if (discloseorganization_.isset()) {
                params.push_back(discloseorganization_.get_value());
                sql << set_separator.get() << "discloseorganization = $" << params.size() << "::boolean ";
            }

            if (discloseaddress_.isset()) {
                params.push_back(discloseaddress_.get_value());
                sql << set_separator.get() << "discloseaddress = $" << params.size() << "::boolean ";
            }

            if (disclosetelephone_.isset()) {
                params.push_back(disclosetelephone_.get_value());
                sql << set_separator.get() << "disclosetelephone = $" << params.size() << "::boolean ";
            }

            if (disclosefax_.isset()) {
                params.push_back(disclosefax_.get_value());
                sql << set_separator.get() << "disclosefax = $" << params.size() << "::boolean ";
            }

            if (discloseemail_.isset()) {
                params.push_back(discloseemail_.get_value());
                sql << set_separator.get() << "discloseemail = $" << params.size() << "::boolean ";
            }

            if (disclosevat_.isset()) {
                params.push_back(disclosevat_.get_value());
                sql << set_separator.get() << "disclosevat = $" << params.size() << "::boolean ";
            }

            if (discloseident_.isset()) {
                params.push_back(discloseident_.get_value());
                sql << set_separator.get() << "discloseident = $" << params.size() << "::boolean ";
            }

            if (disclosenotifyemail_.isset()) {
                params.push_back(disclosenotifyemail_.get_value());
                sql << set_separator.get() << "disclosenotifyemail = $" << params.size() << "::boolean ";
            }

            params.push_back(contact.info_contact_data.id);
            sql << " WHERE id = $" << params.size() << "::integer RETURNING id";

            //check exception
            if (update_contact_exception.throw_me()) {
                BOOST_THROW_EXCEPTION(update_contact_exception);
            }

            //execute update of the contact
            if (1 < params.size()) {
                Database::Result update_contact_res = ctx.get_conn().exec_params(sql.str(), params);
                if (update_contact_res.size() != 1) {
                    BOOST_THROW_EXCEPTION(InternalError("failed to update contact"));
                }
            }

            //UPDATE or INSERT contact_address
            if (!addresses_.to_update().empty()) {
                const ContactAddressToUpdate::ToUpdate &to_update = addresses_.to_update();
                params = Database::QueryParams();
                params.push_back(contact.info_contact_data.id);
                Database::Result address_types_res = ctx.get_conn().exec_params(
                    "SELECT type FROM contact_address WHERE contactid=$1::bigint", params);
                ContactAddressToUpdate::ToRemove updatable;//exists
                for (::size_t idx = 0; idx < address_types_res.size(); ++idx) {
                    updatable.insert(ContactAddressType::from_string(
                                     static_cast< std::string >(address_types_res[idx][0])));
                }
                std::ostringstream sql_insert;
                Database::QueryParams params_insert;
                for (ContactAddressToUpdate::ToUpdate::const_iterator addr_ptr = to_update.begin();
                     addr_ptr != to_update.end(); ++addr_ptr) {
                    const std::string type = addr_ptr->first.to_string();
                    const ContactAddress &address = addr_ptr->second;
                    if (updatable.find(addr_ptr->first) != updatable.end()) { //UPDATE
                        sql.str("");
                        sql << "UPDATE contact_address SET ";
                        params = Database::QueryParams();
                        params.push_back(contact.info_contact_data.id);//$1 = contactid
                        params.push_back(type); //$2 = type
                        //company_name optional
                        if (address.company_name.isset()) {
                            params.push_back(address.company_name.get_value());
                            sql << "company_name=$" << params.size() << "::text,";
                        }
                        else {
                            sql << "company_name=NULL,";
                        }
                        //street1 required
                        params.push_back(address.street1);
                        sql << "street1=$" << params.size() << "::text,";
                        //street2 optional
                        if (address.street2.isset()) {
                            params.push_back(address.street2.get_value());
                            sql << "street2=$" << params.size() << "::text,";
                        }
                        else {
                            sql << "street2=NULL,";
                        }
                        //street3 optional
                        if (address.street3.isset()) {
                            params.push_back(address.street3.get_value());
                            sql << "street3=$" << params.size() << "::text,";
                        }
                        else {
                            sql << "street3=NULL,";
                        }
                        //city required
                        params.push_back(address.city);
                        sql << "city=$" << params.size() << "::text,";
                        //stateorprovince optional
                        if (address.stateorprovince.isset()) {
                            params.push_back(address.stateorprovince.get_value());
                            sql << "stateorprovince=$" << params.size() << "::text,";
                        }
                        else {
                            sql << "stateorprovince=NULL,";
                        }
                        //postalcode required
                        params.push_back(address.postalcode);
                        sql << "postalcode=$" << params.size() << "::text,";
                        //country required
                        params.push_back(address.country);
                        sql << "country=$" << params.size() << "::text "
                        "WHERE contactid=$1::bigint AND type=$2::contact_address_type";
                        ctx.get_conn().exec_params(sql.str(), params);
                    }
                    else { //INSERT
                        if (sql_insert.str().empty()) {
                            params_insert.push_back(contact.info_contact_data.id);//$1 = contactid
                            sql_insert <<
                            "INSERT INTO contact_address (contactid,type,company_name,street1,"
                                                         "street2,street3,city,stateorprovince,"
                                                         "postalcode,country) "
                            "VALUES(";
                        }
                        else {
                            sql_insert << ",(";
                        }
                        params_insert.push_back(type);
                        sql_insert <<
                        //contactid
                        "$1::bigint,"
                        //type
                        "$" << params_insert.size() << "::contact_address_type,";
                        //company_name optional
                        if (address.company_name.isset()) {
                            params_insert.push_back(address.company_name.get_value());
                            sql_insert << "$" << params_insert.size() << "::text,";
                        }
                        else {
                            sql_insert << "NULL,";
                        }
                        //street1 required
                        params_insert.push_back(address.street1);
                        sql_insert << "$" << params_insert.size() << "::text,";
                        //street2 optional
                        if (address.street2.isset()) {
                            params_insert.push_back(address.street2.get_value());
                            sql_insert << "$" << params_insert.size() << "::text,";
                        }
                        else {
                            sql_insert << "NULL,";
                        }
                        //street3 optional
                        if (address.street3.isset()) {
                            params_insert.push_back(address.street3.get_value());
                            sql_insert << "$" << params_insert.size() << "::text,";
                        }
                        else {
                            sql_insert << "NULL,";
                        }
                        //city required
                        params_insert.push_back(address.city);
                        sql_insert << "$" << params_insert.size() << "::text,";
                        //stateorprovince optional
                        if (address.stateorprovince.isset()) {
                            params_insert.push_back(address.stateorprovince.get_value());
                            sql_insert << "$" << params_insert.size() << "::text,";
                        }
                        else {
                            sql_insert << "NULL,";
                        }
                        //postalcode required
                        params_insert.push_back(address.postalcode);
                        sql_insert << "$" << params_insert.size() << "::text,";
                        //country required
                        params_insert.push_back(address.country);
                        sql_insert << "$" << params_insert.size() << "::text)";
                    }
                }
                if (!sql_insert.str().empty()) {
                    ctx.get_conn().exec_params(sql_insert.str(), params_insert);
                }
            }

            //DELETE contact_address
            if (!addresses_.to_remove().empty()) {
                const ContactAddressToUpdate::ToRemove &to_remove = addresses_.to_remove();
                params = Database::QueryParams();
                params.push_back(contact.info_contact_data.id);//$1 = contactid
                sql.str("");
                sql << "DELETE FROM contact_address WHERE contactid=$1::bigint AND "
                                                         "type IN (";
                for (ContactAddressToUpdate::ToRemove::const_iterator type_ptr = to_remove.begin();
                     type_ptr != to_remove.end(); ++type_ptr) {
                    params.push_back(type_ptr->to_string());//type
                    if (type_ptr != to_remove.begin()) {
                        sql << ",";
                    }
                    sql << "$" << params.size() << "::contact_address_type";
                }
                sql << ")";
                ctx.get_conn().exec_params(sql.str(), params);
            }
        }//update contact

        //save history
        {
            //contact_history
            ctx.get_conn().exec_params(
                "INSERT INTO contact_history(historyid,id"
                " , name, organization, street1, street2, street3, city, stateorprovince, postalcode"
                " , country, telephone, fax, email, notifyemail, vat, ssntype, ssn"
                " , disclosename, discloseorganization, discloseaddress, disclosetelephone"
                " , disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail"
                ") "
                 "SELECT $1::bigint, id "
                " , name, organization, street1, street2, street3, city, stateorprovince, postalcode "
                " , country, telephone, fax, email, notifyemail, vat, ssntype, ssn "
                " , disclosename, discloseorganization, discloseaddress, disclosetelephone "
                " , disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail "
                " FROM contact "
                " WHERE id = $2::integer"
                , Database::query_param_list(history_id)(contact.info_contact_data.id));

            ctx.get_conn().exec_params(
                "INSERT INTO contact_address_history (historyid, id, contactid, type, company_name,"
                " street1, street2, street3, city, stateorprovince, postalcode, country)"
                " SELECT $1::bigint, id, contactid, type, company_name,"
                " street1, street2, street3, city, stateorprovince, postalcode, country"
                " FROM contact_address WHERE contactid=$2::bigint"
                , Database::query_param_list(history_id)(contact.info_contact_data.id));
        }//save history

        return history_id;
    }

    template < class DERIVED >
    std::string UpdateContact< DERIVED >::to_string() const
    {
        return Util::format_operation_state("UpdateContact",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("registrar",registrar_))
        (std::make_pair("sponsoring_registrar",sponsoring_registrar_.print_quoted()))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("name",name_.print_quoted()))
        (std::make_pair("organization",organization_.print_quoted()))
        (std::make_pair("place",place_.print_quoted()))
        (std::make_pair("telephone",telephone_.print_quoted()))
        (std::make_pair("fax",fax_.print_quoted()))
        (std::make_pair("email",email_.print_quoted()))
        (std::make_pair("notifyemail_",notifyemail_.print_quoted()))
        (std::make_pair("vat",vat_.print_quoted()))
        (std::make_pair("ssntype",ssntype_.print_quoted()))
        (std::make_pair("ssn",ssn_.print_quoted()))
        (std::make_pair("addresses",addresses_.to_string()))
        (std::make_pair("disclosename",disclosename_.print_quoted()))
        (std::make_pair("discloseorganization",discloseorganization_.print_quoted()))
        (std::make_pair("discloseaddress",discloseaddress_.print_quoted()))
        (std::make_pair("disclosetelephone",disclosetelephone_.print_quoted()))
        (std::make_pair("disclosefax",disclosefax_.print_quoted()))
        (std::make_pair("discloseemail",discloseemail_.print_quoted()))
        (std::make_pair("disclosevat",disclosevat_.print_quoted()))
        (std::make_pair("discloseident",discloseident_.print_quoted()))
        (std::make_pair("disclosenotifyemail",disclosenotifyemail_.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }

    UpdateContactById::UpdateContactById(unsigned long long id, const std::string& registrar)
        : UpdateContact<UpdateContactById>(registrar)
        , id_(id)
        , select_contact_by_id_(Fred::InfoContactById(id_).set_lock())
        {}

    UpdateContactById::UpdateContactById(unsigned long long id
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& authinfo
            , const Optional<std::string>& name
            , const Optional<std::string>& organization
            , const Optional< Fred::Contact::PlaceAddress > &place
            , const Optional<std::string>& telephone
            , const Optional<std::string>& fax
            , const Optional<std::string>& email
            , const Optional<std::string>& notifyemail
            , const Optional<std::string>& vat
            , const Optional<std::string>& ssntype
            , const Optional<std::string>& ssn
            , const ContactAddressToUpdate &addresses
            , const Optional<bool>& disclosename
            , const Optional<bool>& discloseorganization
            , const Optional<bool>& discloseaddress
            , const Optional<bool>& disclosetelephone
            , const Optional<bool>& disclosefax
            , const Optional<bool>& discloseemail
            , const Optional<bool>& disclosevat
            , const Optional<bool>& discloseident
            , const Optional<bool>& disclosenotifyemail
            , const Optional<unsigned long long>& logd_request_id
            )
    : UpdateContact<UpdateContactById>(registrar
              , sponsoring_registrar
              , authinfo
              , name
              , organization
              , place
              , telephone
              , fax
              , email
              , notifyemail
              , vat
              , ssntype
              , ssn
              , addresses
              , disclosename
              , discloseorganization
              , discloseaddress
              , disclosetelephone
              , disclosefax
              , discloseemail
              , disclosevat
              , discloseident
              , disclosenotifyemail
              , logd_request_id
              )
    , id_(id)
    , select_contact_by_id_(Fred::InfoContactById(id_).set_lock())
    {}

    unsigned long long UpdateContactById::exec(Fred::OperationContext& ctx)
    {
        unsigned long long history_id = 0;//to return

        try
        {
            ExceptionType update_exception;
            Fred::InfoContactOutput contact;//contact selected for update
            try
            {
                contact = select_contact_by_id_.exec(ctx);
            }
            catch(const Fred::InfoContactById::Exception& info_exception)
            {
                if(info_exception.is_set_unknown_object_id())
                {
                    update_exception.set_unknown_contact_id(
                        info_exception.get_unknown_object_id());
                }
                else throw;//rethrow unexpected
            }

            try
            {
                history_id = UpdateContact<UpdateContactById>::exec(ctx,contact);
            }
            catch(const UpdateContact<UpdateContactById>::Exception& update_contact_exception)
            {
                if(update_contact_exception.is_set_unknown_registrar_handle())
                {
                    //fatal good path, need valid registrar performing update
                    BOOST_THROW_EXCEPTION(ExceptionType().set_unknown_registrar_handle(
                            update_contact_exception.get_unknown_registrar_handle()));
                }
                else if(update_contact_exception.is_set_unknown_contact_handle()
                    ||  update_contact_exception.is_set_unknown_sponsoring_registrar_handle()
                    ||  update_contact_exception.is_set_unknown_ssntype()
                    ||  update_contact_exception.is_set_unknown_country()
                    )//non-fatal good path, update can continue to check input
                {
                    if(update_contact_exception.is_set_unknown_contact_handle())
                    {
                        update_exception.set_unknown_contact_id(id_);
                    }

                    if(update_contact_exception.is_set_unknown_sponsoring_registrar_handle())
                    {
                        update_exception.set_unknown_sponsoring_registrar_handle(
                                update_contact_exception.get_unknown_sponsoring_registrar_handle());
                    }

                    if(update_contact_exception.is_set_unknown_ssntype())
                    {
                        update_exception.set_unknown_ssntype(
                                update_contact_exception.get_unknown_ssntype());
                    }

                    if(update_contact_exception.is_set_unknown_country())
                    {
                        update_exception.set_unknown_country(
                                update_contact_exception.get_unknown_country());
                    }
                }
                else throw;//rethrow unexpected
            }

            if(update_exception.throw_me())
            {
                BOOST_THROW_EXCEPTION(update_exception);
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }

    std::string UpdateContactById::to_string() const
    {
        return Util::format_operation_state("UpdateContactById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("select_contact_by_id", select_contact_by_id_.to_string()))//member operation
        (std::make_pair("UpdateContact<UpdateContactById>",UpdateContact<UpdateContactById>::to_string()))//parent operation
        );
    }

    UpdateContactByHandle::UpdateContactByHandle(const std::string& handle, const std::string& registrar)
        : UpdateContact<UpdateContactByHandle>(registrar)
        , handle_(handle)
        , select_contact_by_handle_(Fred::InfoContactByHandle(handle_).set_lock())
        {}

    UpdateContactByHandle::UpdateContactByHandle(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& authinfo
            , const Optional<std::string>& name
            , const Optional<std::string>& organization
            , const Optional< Fred::Contact::PlaceAddress > &place
            , const Optional<std::string>& telephone
            , const Optional<std::string>& fax
            , const Optional<std::string>& email
            , const Optional<std::string>& notifyemail
            , const Optional<std::string>& vat
            , const Optional<std::string>& ssntype
            , const Optional<std::string>& ssn
            , const ContactAddressToUpdate &addresses
            , const Optional<bool>& disclosename
            , const Optional<bool>& discloseorganization
            , const Optional<bool>& discloseaddress
            , const Optional<bool>& disclosetelephone
            , const Optional<bool>& disclosefax
            , const Optional<bool>& discloseemail
            , const Optional<bool>& disclosevat
            , const Optional<bool>& discloseident
            , const Optional<bool>& disclosenotifyemail
            , const Optional<unsigned long long> logd_request_id
            )
    : UpdateContact<UpdateContactByHandle>(registrar
            , sponsoring_registrar
            , authinfo
            , name
            , organization
            , place
            , telephone
            , fax
            , email
            , notifyemail
            , vat
            , ssntype
            , ssn
            , addresses
            , disclosename
            , discloseorganization
            , discloseaddress
            , disclosetelephone
            , disclosefax
            , discloseemail
            , disclosevat
            , discloseident
            , disclosenotifyemail
            , logd_request_id
            )
    , handle_(handle)
    , select_contact_by_handle_(Fred::InfoContactByHandle(handle_).set_lock())
    {}

    unsigned long long UpdateContactByHandle::exec(Fred::OperationContext& ctx)
    {
        unsigned long long history_id = 0;//to return

        try
        {
            ExceptionType update_exception;
            Fred::InfoContactOutput contact;
            try
            {
                contact = select_contact_by_handle_.exec(ctx);
            }
            catch(const Fred::InfoContactByHandle::Exception& info_exception)
            {
                if(info_exception.is_set_unknown_contact_handle())
                {
                    update_exception.set_unknown_contact_handle(
                        info_exception.get_unknown_contact_handle());
                }
                else throw;//rethrow unexpected
            }

            try
            {
                history_id = UpdateContact<UpdateContactByHandle>::exec(ctx,contact);
            }
            catch(const UpdateContact<UpdateContactByHandle>::Exception& update_contact_exception)
            {
                if(update_contact_exception.is_set_unknown_registrar_handle())
                {
                    //fatal good path, need valid registrar performing update
                    BOOST_THROW_EXCEPTION(ExceptionType().set_unknown_registrar_handle(
                            update_contact_exception.get_unknown_registrar_handle()));
                }
                else if(update_contact_exception.is_set_unknown_contact_handle()
                    ||  update_contact_exception.is_set_unknown_sponsoring_registrar_handle()
                    ||  update_contact_exception.is_set_unknown_ssntype()
                    ||  update_contact_exception.is_set_unknown_country()
                        )//non-fatal good path, update can continue to check input
                {
                    if(update_contact_exception.is_set_unknown_contact_handle())
                    {
                        update_exception.set_unknown_contact_handle(handle_);
                    }

                    if(update_contact_exception.is_set_unknown_sponsoring_registrar_handle())
                    {   //non-fatal good path, update can continue to check input
                        update_exception.set_unknown_sponsoring_registrar_handle(
                                update_contact_exception.get_unknown_sponsoring_registrar_handle());
                    }

                    if(update_contact_exception.is_set_unknown_ssntype())
                    {   //non-fatal good path, update can continue to check input
                        update_exception.set_unknown_ssntype(
                                update_contact_exception.get_unknown_ssntype());
                    }

                    if(update_contact_exception.is_set_unknown_country())
                    {   //non-fatal good path, update can continue to check input
                        update_exception.set_unknown_country(
                                update_contact_exception.get_unknown_country());
                    }
                }
                else throw;//rethrow unexpected
            }

            if(update_exception.throw_me())
            {
                BOOST_THROW_EXCEPTION(update_exception);
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }

    std::string UpdateContactByHandle::to_string() const
    {
        return Util::format_operation_state("UpdateContactByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("select_contact_by_handle", select_contact_by_handle_.to_string()))//member operation
        (std::make_pair("UpdateContact<UpdateContactByHandle>",UpdateContact<UpdateContactByHandle>::to_string()))//parent operation
        );
    }

}//namespace Fred

