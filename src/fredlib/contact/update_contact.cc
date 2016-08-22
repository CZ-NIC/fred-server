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
#include "src/fredlib/contact/copy_history_impl.h"
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
    ContactAddressToUpdate& ContactAddressToUpdate::update(const ContactAddress &_address)
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
    update< ContactAddressType::MAILING >(const ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    update< ContactAddressType::BILLING >(const ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    update< ContactAddressType::SHIPPING >(const ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    update< ContactAddressType::SHIPPING_2 >(const ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    update< ContactAddressType::SHIPPING_3 >(const ContactAddress &_address);

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::MAILING >();

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::BILLING >();

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::SHIPPING >();

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::SHIPPING_2 >();

    template ContactAddressToUpdate& ContactAddressToUpdate::
    remove< ContactAddressType::SHIPPING_3 >();

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
        , const Optional<std::string>& authinfo
        , const Optional< Nullable< std::string > > &name
        , const Optional< Nullable< std::string > > &organization
        , const Optional< Nullable< Fred::Contact::PlaceAddress > > &place
        , const Optional< Nullable< std::string > > &telephone
        , const Optional< Nullable< std::string > > &fax
        , const Optional< Nullable< std::string > > &email
        , const Optional< Nullable< std::string > > &notifyemail
        , const Optional< Nullable< std::string > > &vat
        , const Optional< Nullable< PersonalIdUnion > > &personal_id
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
        , const Optional< Nullable< bool > > &domain_expiration_letter_flag
        , const Optional< unsigned long long > &logd_request_id)
    :   registrar_(registrar)
    ,   authinfo_(authinfo)
    ,   name_(name)
    ,   organization_(organization)
    ,   place_(place)
    ,   telephone_(telephone)
    ,   fax_(fax)
    ,   email_(email)
    ,   notifyemail_(notifyemail)
    ,   vat_(vat)
    ,   personal_id_(personal_id)
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
    ,   domain_expiration_letter_flag_(domain_expiration_letter_flag)
    ,   logd_request_id_(logd_request_id)
    {}

    std::string update_value(
        const std::string &value,
        const std::string &value_name,
        Database::query_param_list &params)
    {
        return value_name + "=$" + params.add(value) + "::text";
    }

    std::string update_value(
        const Nullable< std::string > &null_value,
        const std::string &value_name,
        Database::query_param_list &params)
    {
        std::string sql = value_name + "=";
        if (null_value.isnull()) {
            sql += "NULL";
        }
        else {
            sql += "$" + params.add(null_value.get_value());
        }
        sql += "::text ";
        return sql;
    }

    std::string update_value(
        const Optional< std::string > &opt_value,
        const std::string &value_name,
        Database::query_param_list &params)
    {
        std::string sql = value_name + "=";
        if (opt_value.isset()) {
            sql += "$" + params.add(opt_value.get_value());
        }
        else {
            sql += "NULL";
        }
        sql += "::text ";
        return sql;
    }

    std::string update_value(
        const Optional< Nullable< std::string > > &opt_null_value,
        const std::string &value_name,
        Util::HeadSeparator &set_separator,
        Database::query_param_list &params)
    {
        return opt_null_value.isset()
               ? (set_separator.get() + update_value(opt_null_value.get_value(), value_name, params))
               : std::string();
    }

    std::string update_value(
        const Optional< bool > &opt_value,
        const std::string &value_name,
        Util::HeadSeparator &set_separator,
        Database::query_param_list &params)
    {
        return opt_value.isset()
               ? (set_separator.get() + value_name + "=$" + params.add(opt_value.get_value()) + "::boolean ")
               : std::string();
    }

    template < class DERIVED >
    unsigned long long UpdateContact< DERIVED >::exec(OperationContext& ctx
        , const InfoContactOutput& contact)
    {
        unsigned long long history_id = 0;

        Exception update_contact_exception;
        //update object
        try {
            history_id = UpdateObject(contact.info_contact_data.handle,
                "contact", registrar_, authinfo_,
                logd_request_id_.isset() ? Nullable< ::size_t >(logd_request_id_.get_value())
                                         : Nullable< ::size_t >()).exec(ctx);
        }
        catch(const UpdateObject::Exception& ex) {
            if(ex.is_set_unknown_registrar_handle()) {
                //fatal good path, need valid registrar performing update
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(ex.get_unknown_registrar_handle()));
            }

            bool caught_exception_has_been_handled = false;

            //non-fatal good paths, update can continue to check input
            if(ex.is_set_unknown_object_handle()) {
                update_contact_exception.set_unknown_contact_handle(contact.info_contact_data.handle);
                caught_exception_has_been_handled = true;
            }

            if( ! caught_exception_has_been_handled ) {
                throw; //rethrow unexpected
            }
        }

        //update contact
        {
            Database::query_param_list params;
            std::ostringstream update_set;
            Util::HeadSeparator set_separator("SET ",",");

            update_set << update_value(name_,         "name",         set_separator, params);
            update_set << update_value(organization_, "organization", set_separator, params);
            if (place_.isset()) {
                if (place_.get_value().isnull()) {
                    update_set << set_separator.get() << "street1=NULL::text,"
                                                  "street2=NULL::text,"
                                                  "street3=NULL::text,"
                                                  "city=NULL::text,"
                                                  "stateorprovince=NULL::text,"
                                                  "postalcode=NULL::text,"
                                                  "country=NULL::text";
                }
                else {
                    const Fred::Contact::PlaceAddress place = place_.get_value().get_value();
                    const std::string country = Contact::get_country_code(
                        place.country, ctx, &update_contact_exception, &Exception::set_unknown_country);
                    update_set << set_separator.get();
                    update_set << update_value(place.street1,         "street1",         params) << ",";
                    update_set << update_value(place.street2,         "street2",         params) << ",";
                    update_set << update_value(place.street3,         "street3",         params) << ",";
                    update_set << update_value(place.city,            "city",            params) << ",";
                    update_set << update_value(place.stateorprovince, "stateorprovince", params) << ",";
                    update_set << update_value(place.postalcode,      "postalcode",      params) << ",";
                    update_set << update_value(country,               "country",         params);
                }
            }

            update_set << update_value(telephone_,   "telephone",   set_separator, params);
            update_set << update_value(fax_,         "fax",         set_separator, params);
            update_set << update_value(email_,       "email",       set_separator, params);
            update_set << update_value(notifyemail_, "notifyemail", set_separator, params);
            update_set << update_value(vat_,         "vat",         set_separator, params);

            if (personal_id_.isset()) {
                const Nullable< PersonalIdUnion > nullable_personal_id = personal_id_.get_value();
                if (nullable_personal_id.isnull()) {
                    update_set << set_separator.get() << "ssntype=NULL::integer,"
                                                  "ssn=NULL::text";
                }
                else {
                    const PersonalIdUnion personal_id = nullable_personal_id.get_value();
                    const ::size_t ssn_type_id(
                        Contact::get_ssntype_id(personal_id.get_type(),
                                                ctx, &update_contact_exception, &Exception::set_unknown_ssntype));
                    update_set << set_separator.get() << "ssntype=$" << params.add(ssn_type_id) << "::integer,"
                                                  "ssn=$" << params.add(personal_id.get()) << "::text";
                }
            }

            update_set << update_value(disclosename_,         "disclosename",         set_separator, params);
            update_set << update_value(discloseorganization_, "discloseorganization", set_separator, params);
            update_set << update_value(discloseaddress_,      "discloseaddress",      set_separator, params);
            update_set << update_value(disclosetelephone_,    "disclosetelephone",    set_separator, params);
            update_set << update_value(disclosefax_,          "disclosefax",          set_separator, params);
            update_set << update_value(discloseemail_,        "discloseemail",        set_separator, params);
            update_set << update_value(disclosevat_,          "disclosevat",          set_separator, params);
            update_set << update_value(discloseident_,        "discloseident",        set_separator, params);
            update_set << update_value(disclosenotifyemail_,  "disclosenotifyemail",  set_separator, params);

            if (domain_expiration_letter_flag_.isset()) {
                const Nullable< bool > flag = domain_expiration_letter_flag_.get_value();
                update_set << set_separator.get() << "warning_letter=";
                if (flag.isnull()) {
                    update_set << "NULL";
                }
                else {
                    update_set << "$" << params.add(flag.get_value());
                }
                update_set << "::boolean";
            }

            //check exception
            if (update_contact_exception.throw_me()) {
                BOOST_THROW_EXCEPTION(update_contact_exception);
            }

            //execute update of the contact
            if (!update_set.str().empty()) {
                std::ostringstream sql;
                params.push_back(contact.info_contact_data.id);
                sql << "UPDATE contact " << update_set.str() << " "
                       "WHERE id=$" << params.size() << "::integer RETURNING id";
                Database::Result update_contact_res = ctx.get_conn().exec_params(sql.str(), params);
                if (update_contact_res.size() != 1) {
                    BOOST_THROW_EXCEPTION(InternalError("failed to update contact"));
                }
                Admin::AdminContactVerificationObjectStates::conditionally_cancel_final_states(
                    ctx,
                    contact.info_contact_data.id,
                    name_.isset(),
                    organization_.isset(),
                    place_.isset(),
                    place_.isset(),
                    place_.isset(),
                    place_.isset(),
                    place_.isset(),
                    place_.isset(),
                    place_.isset(),
                    telephone_.isset(),
                    fax_.isset(),
                    email_.isset(),
                    notifyemail_.isset(),
                    vat_.isset(),
                    personal_id_.isset(),
                    personal_id_.isset());
            }

            //UPDATE or INSERT contact_address
            if (!addresses_.to_update().empty()) {
                const ContactAddressToUpdate::ToUpdate &to_update = addresses_.to_update();
                params.clear();
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
                        std::ostringstream sql;
                        sql << "UPDATE contact_address SET ";
                        params.clear();
                        params.push_back(contact.info_contact_data.id);//$1 = contactid
                        params.push_back(type); //$2 = type
                        //company_name optional
                        if (address.company_name.isset()) {
                            if (type != Fred::ContactAddressType::to_string(Fred::ContactAddressType::SHIPPING)
                                    && type != Fred::ContactAddressType::to_string(Fred::ContactAddressType::SHIPPING_2)
                                    && type != Fred::ContactAddressType::to_string(Fred::ContactAddressType::SHIPPING_3))
                            {
                                update_contact_exception.set_forbidden_company_name_setting(type);
                                continue;
                            }
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
                            if (type != Fred::ContactAddressType::to_string(Fred::ContactAddressType::SHIPPING)
                                    && type != Fred::ContactAddressType::to_string(Fred::ContactAddressType::SHIPPING_2)
                                    && type != Fred::ContactAddressType::to_string(Fred::ContactAddressType::SHIPPING_3))
                            {
                                update_contact_exception.set_forbidden_company_name_setting(type);
                                continue;
                            }
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
                //check exception
                if (update_contact_exception.throw_me()) {
                    BOOST_THROW_EXCEPTION(update_contact_exception);
                }

                if (!sql_insert.str().empty()) {
                    ctx.get_conn().exec_params(sql_insert.str(), params_insert);
                }
            }

            //DELETE contact_address
            if (!addresses_.to_remove().empty()) {
                const ContactAddressToUpdate::ToRemove &to_remove = addresses_.to_remove();
                params.clear();
                params.push_back(contact.info_contact_data.id);//$1 = contactid
                std::ostringstream sql;
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

        copy_contact_data_to_contact_history_impl(ctx, contact.info_contact_data.id, history_id);

        return history_id;
    }

    template < class DERIVED >
    std::string UpdateContact< DERIVED >::to_string() const
    {
        return Util::format_operation_state("UpdateContact",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("registrar",registrar_))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("name",name_.print_quoted()))
        (std::make_pair("organization",organization_.print_quoted()))
        (std::make_pair("place",place_.print_quoted()))
        (std::make_pair("telephone",telephone_.print_quoted()))
        (std::make_pair("fax",fax_.print_quoted()))
        (std::make_pair("email",email_.print_quoted()))
        (std::make_pair("notifyemail_",notifyemail_.print_quoted()))
        (std::make_pair("vat",vat_.print_quoted()))
        (std::make_pair("personal_id",personal_id_.print_quoted()))
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
        (std::make_pair("domain_expiration_letter_flag",domain_expiration_letter_flag_.isset() ? domain_expiration_letter_flag_.get_value().print_quoted() : domain_expiration_letter_flag_.print_quoted()))
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
            , const Optional<std::string>& authinfo
            , const Optional< Nullable< std::string > > &name
            , const Optional< Nullable< std::string > > &organization
            , const Optional< Nullable< Fred::Contact::PlaceAddress > > &place
            , const Optional< Nullable< std::string > > &telephone
            , const Optional< Nullable< std::string > > &fax
            , const Optional< Nullable< std::string > > &email
            , const Optional< Nullable< std::string > > &notifyemail
            , const Optional< Nullable< std::string > > &vat
            , const Optional< Nullable< PersonalIdUnion > > &personal_id
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
            , const Optional< Nullable< bool > > &domain_expiration_letter_flag
            , const Optional< unsigned long long > &logd_request_id)
    : UpdateContact<UpdateContactById>(registrar
              , authinfo
              , name
              , organization
              , place
              , telephone
              , fax
              , email
              , notifyemail
              , vat
              , personal_id
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
              , domain_expiration_letter_flag
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
                else {
                    throw;//rethrow unexpected
                }
            }

            try
            {
                history_id = UpdateContact<UpdateContactById>::exec(ctx,contact);
            }
            catch(const UpdateContact<UpdateContactById>::Exception& update_contact_exception)
            {
                if( update_contact_exception.is_set_unknown_registrar_handle() ) {
                    //fatal good path, need valid registrar performing update
                    BOOST_THROW_EXCEPTION( ExceptionType().set_unknown_registrar_handle( update_contact_exception.get_unknown_registrar_handle()) );
                }

                bool caught_exception_has_been_handled = false;

                //non-fatal good paths, update can continue to check input
                if(update_contact_exception.is_set_unknown_contact_handle()) {
                    update_exception.set_unknown_contact_id(id_);
                    caught_exception_has_been_handled = true;
                }
                if( update_contact_exception.is_set_unknown_ssntype() ) {
                    update_exception.set_unknown_ssntype( update_contact_exception.get_unknown_ssntype() );
                    caught_exception_has_been_handled = true;
                }
                if( update_contact_exception.is_set_unknown_country() ) {
                    update_exception.set_unknown_country( update_contact_exception.get_unknown_country() );
                    caught_exception_has_been_handled = true;
                }
                if( update_contact_exception.is_set_forbidden_company_name_setting() ) {
                    update_exception.set_forbidden_company_name_setting( update_contact_exception.get_forbidden_company_name_setting() );
                    caught_exception_has_been_handled = true;
                }

                if( ! caught_exception_has_been_handled ) {
                    throw; //rethrow unexpected
                }
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
            , const Optional<std::string>& authinfo
            , const Optional< Nullable< std::string > > &name
            , const Optional< Nullable< std::string > > &organization
            , const Optional< Nullable< Fred::Contact::PlaceAddress > > &place
            , const Optional< Nullable< std::string > > &telephone
            , const Optional< Nullable< std::string > > &fax
            , const Optional< Nullable< std::string > > &email
            , const Optional< Nullable< std::string > > &notifyemail
            , const Optional< Nullable< std::string > > &vat
            , const Optional< Nullable< PersonalIdUnion > > &personal_id
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
            , const Optional< Nullable< bool > > &domain_expiration_letter_flag
            , const Optional< unsigned long long > &logd_request_id)
    : UpdateContact<UpdateContactByHandle>(registrar
            , authinfo
            , name
            , organization
            , place
            , telephone
            , fax
            , email
            , notifyemail
            , vat
            , personal_id
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
            , domain_expiration_letter_flag
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
                else {
                    throw;//rethrow unexpected
                }
            }

            try
            {
                history_id = UpdateContact<UpdateContactByHandle>::exec(ctx,contact);
            }
            catch(const UpdateContact<UpdateContactByHandle>::Exception& update_contact_exception)
            {
                if( update_contact_exception.is_set_unknown_registrar_handle() ) {
                    //fatal good path, need valid registrar performing update
                    BOOST_THROW_EXCEPTION( ExceptionType().set_unknown_registrar_handle( update_contact_exception.get_unknown_registrar_handle()) );
                }

                bool caught_exception_has_been_handled = false;

                //non-fatal good paths, update can continue to check input
                if( update_contact_exception.is_set_unknown_contact_handle() ) {
                    update_exception.set_unknown_contact_handle(handle_);
                    caught_exception_has_been_handled = true;
                }
                if( update_contact_exception.is_set_unknown_ssntype() ) {
                    update_exception.set_unknown_ssntype( update_contact_exception.get_unknown_ssntype() );
                    caught_exception_has_been_handled = true;
                }
                if( update_contact_exception.is_set_unknown_country() ) {
                    update_exception.set_unknown_country( update_contact_exception.get_unknown_country() );
                    caught_exception_has_been_handled = true;
                }
                if( update_contact_exception.is_set_forbidden_company_name_setting() ) {
                    update_exception.set_forbidden_company_name_setting( update_contact_exception.get_forbidden_company_name_setting() );
                    caught_exception_has_been_handled = true;
                }

                if( ! caught_exception_has_been_handled ) {
                    throw; //rethrow unexpected
                }
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
