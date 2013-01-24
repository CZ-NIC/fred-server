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
 *  @file create_contact.h
 *  create contact
 */


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fredlib/contact/create_contact.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
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
            )
    : handle_(handle)
    , registrar_(registrar)
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

    CreateContact& CreateContact::set_street1(const std::string& street1)
    {
        street1_ = street1;
        return *this;
    }

    CreateContact& CreateContact::set_street2(const std::string& street2)
    {
        street2_ = street2;
        return *this;
    }

    CreateContact& CreateContact::set_street3(const std::string& street3)
    {
        street3_ = street3;
        return *this;
    }

    CreateContact& CreateContact::set_city(const std::string& city)
    {
        city_ = city;
        return *this;
    }

    CreateContact& CreateContact::set_stateorprovince(const std::string& stateorprovince)
    {
        stateorprovince_ = stateorprovince;
        return *this;
    }

    CreateContact& CreateContact::set_postalcode(const std::string& postalcode)
    {
        postalcode_ = postalcode;
        return *this;
    }

    CreateContact& CreateContact::set_country(const std::string& country)
    {
        country_ = country;
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

    std::string CreateContact::exec(OperationContext& ctx)
    {
        std::string timestamp;

        try
        {
            unsigned long long object_id = CreateObject("contact", handle_, registrar_, authinfo_).exec(ctx);
            //create contact
            {
                Database::QueryParams params;//query params
                std::stringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO contact (";
                val_sql << " VALUES (";

                //id
                params.push_back(object_id);
                col_sql << col_separator.get() << "id";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                col_sql <<")";
                val_sql << ")";

            }

        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<CreateContactException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }

        return timestamp;
    }

}//namespace Fred

