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
 *  create contact
 */


#include <string>
#include <vector>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/contact/contact_enum.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    CreateContact::CreateContact(const std::string& handle
                , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    CreateContact::CreateContact(const std::string& handle
            , const std::string& registrar
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
            , const Optional<ContactAddressList>& addresses
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
    : handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , name_(name)
    , organization_(organization)
    , place_(place)
    , telephone_(telephone)
    , fax_(fax)
    , email_(email)
    , notifyemail_(notifyemail)
    , vat_(vat)
    , ssntype_(ssntype)
    , ssn_(ssn)
    , addresses_(addresses)
    , disclosename_(disclosename)
    , discloseorganization_(discloseorganization)
    , discloseaddress_(discloseaddress)
    , disclosetelephone_(disclosetelephone)
    , disclosefax_(disclosefax)
    , discloseemail_(discloseemail)
    , disclosevat_(disclosevat)
    , discloseident_(discloseident)
    , disclosenotifyemail_(disclosenotifyemail)
    , logd_request_id_(logd_request_id.isset()
            ? Nullable<unsigned long long>(logd_request_id.get_value())
            : Nullable<unsigned long long>())//is NULL if not set
    {}

    CreateContact& CreateContact::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    CreateContact& CreateContact::set_name(const std::string& name)
    {
        name_ = name;
        return *this;
    }

    CreateContact& CreateContact::set_organization(const std::string& organization)
    {
        organization_ = organization;
        return *this;
    }

    CreateContact& CreateContact::set_place(const Fred::Contact::PlaceAddress &place)
    {
        place_ = place;
        return *this;
    }

    CreateContact& CreateContact::set_telephone(const std::string& telephone)
    {
        telephone_ = telephone;
        return *this;
    }

    CreateContact& CreateContact::set_fax(const std::string& fax)
    {
        fax_ = fax;
        return *this;
    }

    CreateContact& CreateContact::set_email(const std::string& email)
    {
        email_ = email;
        return *this;
    }

    CreateContact& CreateContact::set_notifyemail(const std::string& notifyemail)
    {
        notifyemail_ = notifyemail;
        return *this;
    }

    CreateContact& CreateContact::set_vat(const std::string& vat)
    {
        vat_ = vat;
        return *this;
    }

    CreateContact& CreateContact::set_ssntype(const std::string& ssntype)
    {
        ssntype_ = ssntype;
        return *this;
    }

    CreateContact& CreateContact::set_ssn(const std::string& ssn)
    {
        ssn_ = ssn;
        return *this;
    }

    CreateContact& CreateContact::set_addresses(const ContactAddressList& addresses)
    {
        addresses_ = addresses;
        return *this;
    }
    
    CreateContact& CreateContact::set_disclosename(const bool disclosename)
    {
        disclosename_ = disclosename;
        return *this;
    }

    CreateContact& CreateContact::set_discloseorganization(const bool discloseorganization)
    {
        discloseorganization_ = discloseorganization;
        return *this;
    }

    CreateContact& CreateContact::set_discloseaddress(const bool discloseaddress)
    {
        discloseaddress_ = discloseaddress;
        return *this;
    }

    CreateContact& CreateContact::set_disclosetelephone(const bool disclosetelephone)
    {
        disclosetelephone_ = disclosetelephone;
        return *this;
    }

    CreateContact& CreateContact::set_disclosefax(const bool disclosefax)
    {
        disclosefax_ = disclosefax;
        return *this;
    }

    CreateContact& CreateContact::set_discloseemail(const bool discloseemail)
    {
        discloseemail_ = discloseemail;
        return *this;
    }

    CreateContact& CreateContact::set_disclosevat(const bool disclosevat)
    {
        disclosevat_ = disclosevat;
        return *this;
    }

    CreateContact& CreateContact::set_discloseident(const bool discloseident)
    {
        discloseident_ = discloseident;
        return *this;
    }

    CreateContact& CreateContact::set_disclosenotifyemail(const bool disclosenotifyemail)
    {
        disclosenotifyemail_ = disclosenotifyemail;
        return *this;
    }

    CreateContact& CreateContact::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    boost::posix_time::ptime CreateContact::exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name)
    {
        boost::posix_time::ptime timestamp;

        try
        {
            Exception create_contact_exception;
            CreateObjectOutput create_object_output;
            try
            {
                create_object_output = CreateObject(
                        "contact", handle_, registrar_, authinfo_, logd_request_id_)
                        .exec(ctx);
            }
            catch(const CreateObject::Exception& create_object_exception)
            {
                //CreateObject implementation sets only one member at once into Exception instance
                if(create_object_exception.is_set_unknown_registrar_handle())
                {
                    //fatal good path, need valid registrar performing create
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(
                            create_object_exception.get_unknown_registrar_handle()));
                }
                else if(create_object_exception.is_set_invalid_object_handle())
                {   //non-fatal good path, create can continue to check input
                    create_contact_exception.set_invalid_contact_handle(
                            create_object_exception.get_invalid_object_handle());
                }
                else throw;//rethrow unexpected
            }

            //create contact
            {
                Database::QueryParams params;//query params
                std::ostringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO contact (";
                val_sql << " VALUES (";

                //id
                params.push_back(create_object_output.object_id);
                col_sql << col_separator.get() << "id";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                if(name_.isset())
                {
                    params.push_back(name_.get_value());
                    col_sql << col_separator.get() << "name";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(organization_.isset())
                {
                    params.push_back(organization_.get_value());
                    col_sql << col_separator.get() << "organization";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(place_.isset())
                {
                    const Fred::Contact::PlaceAddress &place = place_.get_value();
                    params.push_back(place.street1);
                    col_sql << col_separator.get() << "street1";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";

                    if(place.street2.isset())
                    {
                        params.push_back(place.street2.get_value());
                        col_sql << col_separator.get() << "street2";
                        val_sql << val_separator.get() << "$" << params.size() <<"::text";
                    }

                    if(place.street3.isset())
                    {
                        params.push_back(place.street3.get_value());
                        col_sql << col_separator.get() << "street3";
                        val_sql << val_separator.get() << "$" << params.size() <<"::text";
                    }

                    params.push_back(place.city);
                    col_sql << col_separator.get() << "city";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";

                    if(place.stateorprovince.isset())
                    {
                        params.push_back(place.stateorprovince.get_value());
                        col_sql << col_separator.get() << "stateorprovince";
                        val_sql << val_separator.get() << "$" << params.size() <<"::text";
                    }

                    params.push_back(place.postalcode);
                    col_sql << col_separator.get() << "postalcode";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";

                    params.push_back(Contact::get_country_code(place.country, ctx,
                            &create_contact_exception, &Exception::set_unknown_country));
                    col_sql << col_separator.get() << "country";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(telephone_.isset())
                {
                    params.push_back(telephone_.get_value());
                    col_sql << col_separator.get() << "telephone";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(fax_.isset())
                {
                    params.push_back(fax_.get_value());
                    col_sql << col_separator.get() << "fax";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(email_.isset())
                {
                    params.push_back(email_.get_value());
                    col_sql << col_separator.get() << "email";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(notifyemail_.isset())
                {
                    params.push_back(notifyemail_.get_value());
                    col_sql << col_separator.get() << "notifyemail";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(vat_.isset())
                {
                    params.push_back(vat_.get_value());
                    col_sql << col_separator.get() << "vat";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(ssntype_.isset())
                {
                    params.push_back(Contact::get_ssntype_id(ssntype_,ctx,
                            &create_contact_exception, &Exception::set_unknown_ssntype));
                    col_sql << col_separator.get() << "ssntype";
                    val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                }

                if(ssn_.isset())
                {
                    params.push_back(ssn_.get_value());
                    col_sql << col_separator.get() << "ssn";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if(disclosename_.isset())
                {
                    params.push_back(disclosename_.get_value());
                    col_sql << col_separator.get() << "disclosename";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(discloseorganization_.isset())
                {
                    params.push_back(discloseorganization_.get_value());
                    col_sql << col_separator.get() << "discloseorganization";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(discloseaddress_.isset())
                {
                    params.push_back(discloseaddress_.get_value());
                    col_sql << col_separator.get() << "discloseaddress";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(disclosetelephone_.isset())
                {
                    params.push_back(disclosetelephone_.get_value());
                    col_sql << col_separator.get() << "disclosetelephone";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(disclosefax_.isset())
                {
                    params.push_back(disclosefax_.get_value());
                    col_sql << col_separator.get() << "disclosefax";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(discloseemail_.isset())
                {
                    params.push_back(discloseemail_.get_value());
                    col_sql << col_separator.get() << "discloseemail";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(disclosevat_.isset())
                {
                    params.push_back(disclosevat_.get_value());
                    col_sql << col_separator.get() << "disclosevat";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(discloseident_.isset())
                {
                    params.push_back(discloseident_.get_value());
                    col_sql << col_separator.get() << "discloseident";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if(disclosenotifyemail_.isset())
                {
                    params.push_back(disclosenotifyemail_.get_value());
                    col_sql << col_separator.get() << "disclosenotifyemail";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                col_sql <<")";
                val_sql << ")";

                if(create_contact_exception.throw_me())
                {
                    BOOST_THROW_EXCEPTION(create_contact_exception);
                }

                //insert into contact
                ctx.get_conn().exec_params(col_sql.str() + val_sql.str(), params);

                //insert into contact_address
                if (addresses_.isset() && !addresses_.get_value().empty()) {
                    std::ostringstream sql;
                    params = Database::QueryParams();
                    params.push_back(create_object_output.object_id);//$1::bigint=contactid
                    const ContactAddressList &addresses = addresses_.get_value();
                    for (ContactAddressList::const_iterator addr_ptr = addresses.begin();
                         addr_ptr != addresses.end(); ++addr_ptr) {
                        if (sql.str().empty()) {
                            sql << "INSERT INTO contact_address "
                                    "("
                                     "contactid,"
                                     "type,"
                                     "company_name,"
                                     "street1,"
                                     "street2,"
                                     "street3,"
                                     "city,"
                                     "stateorprovince,"
                                     "postalcode,"
                                     "country"
                                    ") VALUES ";
                        }
                        else {
                            sql << ",";
                        }
                        //type
                        params.push_back(addr_ptr->first.to_string());
                        sql << "($1::bigint,$" << params.size() << "::contact_address_type,";
                        //company_name
                        if (addr_ptr->second.company_name.isset()) {
                            params.push_back(addr_ptr->second.company_name.get_value());
                            sql << "$" << params.size() << "::text,";
                        }
                        else {
                            sql << "NULL,";
                        }
                        //street1
                        params.push_back(addr_ptr->second.street1);
                        sql << "$" << params.size() << "::text,";
                        //street2
                        if (addr_ptr->second.street2.isset()) {
                            params.push_back(addr_ptr->second.street2.get_value());
                            sql << "$" << params.size() << "::text,";
                        }
                        else {
                            sql << "NULL,";
                        }
                        //street3
                        if (addr_ptr->second.street3.isset()) {
                            params.push_back(addr_ptr->second.street3.get_value());
                            sql << "$" << params.size() << "::text,";
                        }
                        else {
                            sql << "NULL,";
                        }
                        //city
                        params.push_back(addr_ptr->second.city);
                        sql << "$" << params.size() << "::text,";
                        //stateorprovince
                        if (addr_ptr->second.stateorprovince.isset()) {
                            params.push_back(addr_ptr->second.stateorprovince.get_value());
                            sql << "$" << params.size() << "::text,";
                        }
                        else {
                            sql << "NULL,";
                        }
                        //postalcode
                        params.push_back(addr_ptr->second.postalcode);
                        sql << "$" << params.size() << "::text,";
                        //country
                        params.push_back(addr_ptr->second.country);
                        sql << "$" << params.size() << "::text)";
                    }
                    ctx.get_conn().exec_params(sql.str(), params);
                }

                //get crdate from object_registry
                {
                    Database::Result crdate_res = ctx.get_conn().exec_params(
                            "SELECT crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text "
                            "  FROM object_registry "
                            " WHERE id = $2::bigint"
                        , Database::query_param_list(returned_timestamp_pg_time_zone_name)(create_object_output.object_id));

                    if (crdate_res.size() != 1)
                    {
                        BOOST_THROW_EXCEPTION(Fred::InternalError("timestamp of the contact creation was not found"));
                    }

                    timestamp = boost::posix_time::time_from_string(std::string(crdate_res[0][0]));
                }
            }

            //save history
            {
                //contact_history
                ctx.get_conn().exec_params(
                    "INSERT INTO contact_history(historyid,id "
                    " , name, organization, street1, street2, street3, city, stateorprovince, postalcode "
                    " , country, telephone, fax, email, notifyemail, vat, ssntype, ssn "
                    " , disclosename, discloseorganization, discloseaddress, disclosetelephone "
                    " , disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail "
                    " ) "
                    " SELECT $1::bigint, id "
                    " , name, organization, street1, street2, street3, city, stateorprovince, postalcode "
                    " , country, telephone, fax, email, notifyemail, vat, ssntype, ssn "
                    " , disclosename, discloseorganization, discloseaddress, disclosetelephone "
                    " , disclosefax, discloseemail, disclosevat, discloseident, disclosenotifyemail "
                    " FROM contact "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                ctx.get_conn().exec_params(
                    "INSERT INTO contact_address_history (historyid, id, contactid, type, company_name,"
                    " street1, street2, street3, city, stateorprovince, postalcode, country)"
                    " SELECT $1::bigint, id, contactid, type, company_name,"
                    " street1, street2, street3, city, stateorprovince, postalcode, country"
                    " FROM contact_address WHERE contactid=$2::bigint"
                    , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));
            }//save history

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return timestamp;
    }

    std::string CreateContact::to_string() const
    {
        return Util::format_operation_state("CreateContact",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
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
        (std::make_pair("ssntype",ssntype_.print_quoted()))
        (std::make_pair("ssn",ssn_.print_quoted()))
        (std::make_pair("addresses",addresses_.print_quoted()))
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

}// namespace Fred
