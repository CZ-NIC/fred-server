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

#include "fredlib/contact/update_contact.h"
#include "fredlib/object/object.h"
#include "fredlib/contact/contact_enum.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    UpdateContactByHandle::UpdateContactByHandle(const std::string& handle
                , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    UpdateContactByHandle::UpdateContactByHandle(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& authinfo
            , const Optional<std::string>& name
            , const Optional<std::string>& organization
            , const Optional<std::string>& street1
            , const Optional<std::string>& street2
            , const Optional<std::string>& street3
            , const Optional<std::string>& city
            , const Optional<std::string>& stateorprovince
            , const Optional<std::string>& postalcode
            , const Optional<std::string>& country
            , const Optional<std::string>& telephone
            , const Optional<std::string>& fax
            , const Optional<std::string>& email
            , const Optional<std::string>& notifyemail
            , const Optional<std::string>& vat
            , const Optional<std::string>& ssntype
            , const Optional<std::string>& ssn
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
    , sponsoring_registrar_(sponsoring_registrar)
    , authinfo_(authinfo)
    , name_(name)
    , organization_(organization)
    , street1_(street1)
    , street2_(street2)
    , street3_(street3)
    , city_(city)
    , stateorprovince_(stateorprovince)
    , postalcode_(postalcode)
    , country_(country)
    , telephone_(telephone)
    , fax_(fax)
    , email_(email)
    , notifyemail_(notifyemail)
    , vat_(vat)
    , ssntype_(ssntype)
    , ssn_(ssn)
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

    UpdateContactByHandle& UpdateContactByHandle::set_sponsoring_registrar(const std::string& sponsoring_registrar)
    {
        sponsoring_registrar_ = sponsoring_registrar;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_name(const std::string& name)
    {
        name_ = name;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_organization(const std::string& organization)
    {
        organization_ = organization;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_street1(const std::string& street1)
    {
        street1_ = street1;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_street2(const std::string& street2)
    {
        street2_ = street2;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_street3(const std::string& street3)
    {
        street3_ = street3;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_city(const std::string& city)
    {
        city_ = city;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_stateorprovince(const std::string& stateorprovince)
    {
        stateorprovince_ = stateorprovince;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_postalcode(const std::string& postalcode)
    {
        postalcode_ = postalcode;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_country(const std::string& country)
    {
        country_ = country;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_telephone(const std::string& telephone)
    {
        telephone_ = telephone;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_fax(const std::string& fax)
    {
        fax_ = fax;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_email(const std::string& email)
    {
        email_ = email;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_notifyemail(const std::string& notifyemail)
    {
        notifyemail_ = notifyemail;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_vat(const std::string& vat)
    {
        vat_ = vat;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_ssntype(const std::string& ssntype)
    {
        ssntype_ = ssntype;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_ssn(const std::string& ssn)
    {
        ssn_ = ssn;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_disclosename(const bool disclosename)
    {
        disclosename_ = disclosename;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_discloseorganization(const bool discloseorganization)
    {
        discloseorganization_ = discloseorganization;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_discloseaddress(const bool discloseaddress)
    {
        discloseaddress_ = discloseaddress;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_disclosetelephone(const bool disclosetelephone)
    {
        disclosetelephone_ = disclosetelephone;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_disclosefax(const bool disclosefax)
    {
        disclosefax_ = disclosefax;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_discloseemail(const bool discloseemail)
    {
        discloseemail_ = discloseemail;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_disclosevat(const bool disclosevat)
    {
        disclosevat_ = disclosevat;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_discloseident(const bool discloseident)
    {
        discloseident_ = discloseident;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_disclosenotifyemail(const bool disclosenotifyemail)
    {
        disclosenotifyemail_ = disclosenotifyemail;
        return *this;
    }

    UpdateContactByHandle& UpdateContactByHandle::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    unsigned long long UpdateContactByHandle::exec(OperationContext& ctx)
    {
        unsigned long long history_id = 0;

        try
        {
            //check registrar
            {
                Database::Result registrar_res = ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
                    , Database::query_param_list(registrar_));
                if(registrar_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(registrar_));
                }
                if (registrar_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
                }
            }

            //lock row and get contact_id
            unsigned long long contact_id =0;
            {
                Database::Result contact_id_res = ctx.get_conn().exec_params(
                    "SELECT oreg.id FROM contact c "
                    " JOIN object_registry oreg ON c.id = oreg.id "
                    " JOIN enum_object_type eot ON eot.id = oreg.type "
                    " WHERE eot.name = 'contact' AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                    " FOR UPDATE OF oreg "
                    , Database::query_param_list(handle_));

                if (contact_id_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_contact_handle(handle_));
                }
                if (contact_id_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get contact"));
                }
                contact_id = contact_id_res[0][0];
            }

            Exception update_contact_exception;

            try
            {
                history_id = Fred::UpdateObject(handle_,"contact", registrar_
                    , sponsoring_registrar_, authinfo_, logd_request_id_
                ).exec(ctx);
            }
            catch(const Fred::UpdateObject::Exception& ex)
            {
                if(ex.is_set_unknown_object_handle())
                {
                    update_contact_exception.set_unknown_contact_handle(
                            ex.get_unknown_object_handle());
                }

                if(ex.is_set_unknown_object_type()) throw;//kind of internal error

                if(ex.is_set_unknown_registrar_handle())
                {
                    update_contact_exception.set_unknown_registrar_handle(
                            ex.get_unknown_registrar_handle());
                }

                if(ex.is_set_unknown_sponsoring_registrar_handle())
                {
                    update_contact_exception.set_unknown_sponsoring_registrar_handle(
                            ex.get_unknown_sponsoring_registrar_handle());
                }
            }

            if(update_contact_exception.throw_me())
            {
                BOOST_THROW_EXCEPTION(update_contact_exception);
            }

            //update contact
            {
                Database::QueryParams params;//query params
                std::stringstream sql;
                Util::HeadSeparator set_separator("SET ",", ");
                sql <<"UPDATE contact ";

                if(name_.isset())
                {
                    params.push_back(name_.get_value());
                    sql << set_separator.get() << "name = $" << params.size() << "::text ";
                }

                if(organization_.isset())
                {
                    params.push_back(organization_.get_value());
                    sql << set_separator.get() << "organization = $" << params.size() << "::text ";
                }

                if(street1_.isset())
                {
                    params.push_back(street1_.get_value());
                    sql << set_separator.get() << "street1 = $" << params.size() << "::text ";
                }

                if(street2_.isset())
                {
                    params.push_back(street2_.get_value());
                    sql << set_separator.get() << "street2 = $" << params.size() << "::text ";
                }

                if(street3_.isset())
                {
                    params.push_back(street3_.get_value());
                    sql << set_separator.get() << "street3 = $" << params.size() << "::text ";
                }

                if(city_.isset())
                {
                    params.push_back(city_.get_value());
                    sql << set_separator.get() << "city = $" << params.size() << "::text ";
                }

                if(stateorprovince_.isset())
                {
                    params.push_back(stateorprovince_.get_value());
                    sql << set_separator.get() << "stateorprovince = $" << params.size() << "::text ";
                }

                if(postalcode_.isset())
                {
                    params.push_back(postalcode_.get_value());
                    sql << set_separator.get() << "postalcode = $" << params.size() << "::text ";
                }

                if(country_.isset())
                {
                    params.push_back(Contact::get_country_code(country_, ctx,
                        static_cast<Exception*>(0), &Exception::set_unknown_country));
                    sql << set_separator.get() << "country = $" << params.size() << "::text ";
                }

                if(telephone_.isset())
                {
                    params.push_back(telephone_.get_value());
                    sql << set_separator.get() << "telephone = $" << params.size() << "::text ";
                }

                if(fax_.isset())
                {
                    params.push_back(fax_.get_value());
                    sql << set_separator.get() << "fax = $" << params.size() << "::text ";
                }

                if(email_.isset())
                {
                    params.push_back(email_.get_value());
                    sql << set_separator.get() << "email = $" << params.size() << "::text ";
                }

                if(notifyemail_.isset())
                {
                    params.push_back(notifyemail_.get_value());
                    sql << set_separator.get() << "notifyemail = $" << params.size() << "::text ";
                }

                if(vat_.isset())
                {
                    params.push_back(vat_.get_value());
                    sql << set_separator.get() << "vat = $" << params.size() << "::text ";
                }

                if(ssntype_.isset())
                {
                    params.push_back(Contact::get_ssntype_id(ssntype_,ctx, static_cast<Exception*>(0), &Exception::set_unknown_ssntype));
                    sql << set_separator.get() << "ssntype = $" << params.size() << "::integer ";
                }

                if(ssn_.isset())
                {
                    params.push_back(ssn_.get_value());
                    sql << set_separator.get() << "ssn = $" << params.size() << "::text ";
                }

                if(disclosename_.isset())
                {
                    params.push_back(disclosename_.get_value());
                    sql << set_separator.get() << "disclosename = $" << params.size() << "::boolean ";
                }

                if(discloseorganization_.isset())
                {
                    params.push_back(discloseorganization_.get_value());
                    sql << set_separator.get() << "discloseorganization = $" << params.size() << "::boolean ";
                }

                if(discloseaddress_.isset())
                {
                    params.push_back(discloseaddress_.get_value());
                    sql << set_separator.get() << "discloseaddress = $" << params.size() << "::boolean ";
                }

                if(disclosetelephone_.isset())
                {
                    params.push_back(disclosetelephone_.get_value());
                    sql << set_separator.get() << "disclosetelephone = $" << params.size() << "::boolean ";
                }

                if(disclosefax_.isset())
                {
                    params.push_back(disclosefax_.get_value());
                    sql << set_separator.get() << "disclosefax = $" << params.size() << "::boolean ";
                }

                if(discloseemail_.isset())
                {
                    params.push_back(discloseemail_.get_value());
                    sql << set_separator.get() << "discloseemail = $" << params.size() << "::boolean ";
                }

                if(disclosevat_.isset())
                {
                    params.push_back(disclosevat_.get_value());
                    sql << set_separator.get() << "disclosevat = $" << params.size() << "::boolean ";
                }

                if(discloseident_.isset())
                {
                    params.push_back(discloseident_.get_value());
                    sql << set_separator.get() << "discloseident = $" << params.size() << "::boolean ";
                }

                if(disclosenotifyemail_.isset())
                {
                    params.push_back(disclosenotifyemail_.get_value());
                    sql << set_separator.get() << "disclosenotifyemail = $" << params.size() << "::boolean ";
                }

                params.push_back(contact_id);
                sql <<" WHERE id = $" << params.size() << "::integer  RETURNING id";

                if(params.size() > 1)
                {
                    Database::Result update_contact_res = ctx.get_conn().exec_params(sql.str(), params);
                    if(update_contact_res.size() != 1)
                    {
                        BOOST_THROW_EXCEPTION(InternalError("failed to update contact"));
                    }
                }
            }//update contact

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
                    , Database::query_param_list(history_id)(contact_id));

            }//save history

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
        return Util::format_operation_state("UpdateContact",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("sponsoring_registrar",sponsoring_registrar_.print_quoted()))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("name",name_.print_quoted()))
        (std::make_pair("organization",organization_.print_quoted()))
        (std::make_pair("street1",street1_.print_quoted()))
        (std::make_pair("street2",street2_.print_quoted()))
        (std::make_pair("street3",street3_.print_quoted()))
        (std::make_pair("city",city_.print_quoted()))
        (std::make_pair("stateorprovince",stateorprovince_.print_quoted()))
        (std::make_pair("postalcode",postalcode_.print_quoted()))
        (std::make_pair("country",country_.print_quoted()))
        (std::make_pair("telephone",telephone_.print_quoted()))
        (std::make_pair("fax",fax_.print_quoted()))
        (std::make_pair("email",email_.print_quoted()))
        (std::make_pair("notifyemail_",notifyemail_.print_quoted()))
        (std::make_pair("vat",vat_.print_quoted()))
        (std::make_pair("ssntype",ssntype_.print_quoted()))
        (std::make_pair("ssn",ssn_.print_quoted()))
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

}//namespace Fred

