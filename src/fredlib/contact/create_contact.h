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

#ifndef CREATE_CONTACT_H_
#define CREATE_CONTACT_H_

#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"


namespace Fred
{

    class CreateContact
    {
        const std::string handle_;//contact identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> authinfo_;//set authinfo
        Optional<std::string> name_ ;//name of contact person
        Optional<std::string> organization_;//full trade name of organization
        Optional<std::string> street1_;//part of address
        Optional<std::string> street2_;//part of address
        Optional<std::string> street3_;//part of address
        Optional<std::string> city_;//part of address - city
        Optional<std::string> stateorprovince_;//part of address - region
        Optional<std::string> postalcode_;//part of address - postal code
        Optional<std::string> country_;//two character country code (e.g. cz) from enum_country table
        Optional<std::string> telephone_;//telephone number
        Optional<std::string> fax_;//fax number
        Optional<std::string> email_;//email address
        Optional<std::string> notifyemail_;//to this email address will be send message in case of any change in domain or nsset affecting contact
        Optional<std::string> vat_;//tax number
        Optional<std::string> ssntype_;//type of identification from enum_ssntype table
        Optional<std::string> ssn_;//unambiguous identification number (e.g. Social Security number, identity card number, date of birth)
        Optional<bool> disclosename_;//whether reveal contact name
        Optional<bool> discloseorganization_;//whether reveal organization
        Optional<bool> discloseaddress_;//whether reveal address
        Optional<bool> disclosetelephone_;//whether reveal phone number
        Optional<bool> disclosefax_;//whether reveal fax number
        Optional<bool> discloseemail_;//whether reveal email address
        Optional<bool> disclosevat_;//whether reveal VAT number
        Optional<bool> discloseident_;//whether reveal SSN number
        Optional<bool> disclosenotifyemail_;//whether reveal notify email
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request


    public:
        CreateContact(const std::string& handle
                , const std::string& registrar);
        CreateContact(const std::string& handle
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
                , const Optional<unsigned long long> logd_request_id
                );

        CreateContact& set_authinfo(const std::string& authinfo);
        CreateContact& set_name(const std::string& name);
        CreateContact& set_organization(const std::string& organization);
        CreateContact& set_street1(const std::string& street1);
        CreateContact& set_street2(const std::string& street2);
        CreateContact& set_street3(const std::string& street3);
        CreateContact& set_city(const std::string& city);
        CreateContact& set_stateorprovince(const std::string& stateorprovince);
        CreateContact& set_postalcode(const std::string& postalcode);
        CreateContact& set_country(const std::string& country);
        CreateContact& set_telephone(const std::string& telephone);
        CreateContact& set_fax(const std::string& fax);
        CreateContact& set_email(const std::string& email);
        CreateContact& set_notifyemail(const std::string& notifyemail);
        CreateContact& set_vat(const std::string& vat);
        CreateContact& set_ssntype(const std::string& ssntype);
        CreateContact& set_ssn(const std::string& ssn);
        CreateContact& set_disclosename(const bool disclosename);
        CreateContact& set_discloseorganization(const bool discloseorganization);
        CreateContact& set_discloseaddress(const bool discloseaddress);
        CreateContact& set_disclosetelephone(const bool disclosetelephone);
        CreateContact& set_disclosefax(const bool disclosefax);
        CreateContact& set_discloseemail(const bool discloseemail);
        CreateContact& set_disclosevat(const bool disclosevat);
        CreateContact& set_discloseident(const bool discloseident);
        CreateContact& set_disclosenotifyemail(const bool disclosenotifyemail);
        CreateContact& set_logd_request_id(unsigned long long logd_request_id);
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

    };//CreateContact

    //exception impl
    class CreateContactException
    : public OperationExceptionImpl<CreateContactException, 8192>
    {
    public:
        CreateContactException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<CreateContactException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"not found:ssntype", "not found crdate:handle"};
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }

    };//class CreateContactException

    typedef CreateContactException::OperationErrorType CreateContactError;
#define CCEX(DATA) CreateContactException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define CCERR(DATA) CreateContactError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}
#endif // CREATE_CONTACT_H_
