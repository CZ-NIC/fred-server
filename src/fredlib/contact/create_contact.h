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

#ifndef CREATE_CONTACT_H_
#define CREATE_CONTACT_H_

#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"


namespace Fred
{
    /**
    * Create of contact.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the create.
    * Create is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref CreateContact::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class CreateContact : public Util::Printable
    {
    public:
        DECLARE_EXCEPTION_DATA(unknown_ssntype, std::string);/**< exception members for unknown type of identification of the contact generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_country, std::string);/**< exception members for unknown country generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
          , ExceptionData_unknown_ssntype<Exception>
          , ExceptionData_unknown_registrar_handle<Exception>
          , ExceptionData_unknown_country<Exception>
        {};

    private:
        const std::string handle_;/**< contact identifier */
        const std::string registrar_;/**< handle of registrar performing the create */
        Optional<std::string> authinfo_;/**< transfer password */
        Optional<std::string> name_ ;/**< name of contact person */
        Optional<std::string> organization_;/**< full trade name of organization */
        Optional<std::string> street1_;/**< part of address */
        Optional<std::string> street2_;/**< part of address */
        Optional<std::string> street3_;/**< part of address*/
        Optional<std::string> city_;/**< part of address - city */
        Optional<std::string> stateorprovince_;/**< part of address - region */
        Optional<std::string> postalcode_;/**< part of address - postal code */
        Optional<std::string> country_;/**< two character country code or country name */
        Optional<std::string> telephone_;/**<  telephone number */
        Optional<std::string> fax_;/**< fax number */
        Optional<std::string> email_;/**< e-mail address */
        Optional<std::string> notifyemail_;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
        Optional<std::string> vat_;/**< taxpayer identification number */
        Optional<std::string> ssntype_;/**< type of identification from enum_ssntype table */
        Optional<std::string> ssn_;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
        Optional<bool> disclosename_;/**< whether to reveal contact name */
        Optional<bool> discloseorganization_;/**< whether to reveal organization */
        Optional<bool> discloseaddress_;/**< whether to reveal address */
        Optional<bool> disclosetelephone_;/**< whether to reveal phone number */
        Optional<bool> disclosefax_;/**< whether to reveal fax number */
        Optional<bool> discloseemail_;/**< whether to reveal email address */
        Optional<bool> disclosevat_;/**< whether to reveal taxpayer identification number */
        Optional<bool> discloseident_;/**< whether to reveal unambiguous identification number */
        Optional<bool> disclosenotifyemail_;/**< whether to reveal notify email */
        Nullable<unsigned long long> logd_request_id_; /**< id of the new entry in log_entry database table, id is used in other calls to logging within current request */

    public:
        /**
        * Create contact constructor with mandatory parameters.
        * @param handle sets contact identifier into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        */
        CreateContact(const std::string& handle
                , const std::string& registrar);

        /**
        * Create contact constructor with all parameters.
        * @param handle sets contact identifier into @ref handle_ attribute
        * @param registrar sets registrar handle into @ref registrar_ attribute
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @param name sets name of contact person into @ref name_ attribute
        * @param organization sets full trade name of organization into @ref organization_ attribute
        * @param street1 sets part of address into @ref street1_ attribute
        * @param street2 sets part of address into @ref street2_ attribute
        * @param street3 sets part of address into @ref street3_ attribute
        * @param city sets part of address - city into @ref city_ attribute
        * @param stateorprovince sets part of address - region into @ref stateorprovince_ attribute
        * @param postalcode sets part of address - postal code into @ref postalcode_ attribute
        * @param country sets two character country code or country name  into @ref country_ attribute
        * @param telephone sets telephone number into @ref telephone_ attribute
        * @param fax sets fax number into @ref fax_ attribute
        * @param email sets e-mail address into @ref email_ attribute
        * @param notifyemail sets e-mail address for notifications into @ref notifyemail_ attribute
        * @param vat sets taxpayer identification number into @ref vat_ attribute
        * @param ssntype sets type of identification into @ref ssntype_ attribute
        * @param ssn sets unambiguous identification number into @ref ssn_ attribute
        * @param disclosename sets whether to reveal contact name into @ref disclosename_ attribute
        * @param discloseorganization sets whether to reveal organization name into @ref discloseorganization_ attribute
        * @param discloseaddress sets whether to reveal contact address into @ref discloseaddress_ attribute
        * @param disclosetelephone sets whether to reveal telephone number into @ref disclosetelephone_ attribute
        * @param disclosefax sets whether to reveal fax number into @ref disclosefax_ attribute
        * @param discloseemail sets whether to reveal e-mail address into @ref discloseemail_ attribute
        * @param disclosevat sets whether to reveal taxpayer identification number into @ref disclosevat_ attribute
        * @param discloseident sets whether to reveal unambiguous identification number into @ref discloseident_ attribute
        * @param disclosenotifyemail sets whether to reveal e-mail address for notifications into @ref disclosenotifyemail_ attribute
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
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

        /**
        * Sets contact transfer password.
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_authinfo(const std::string& authinfo);

        /**
        * Sets contact name.
        * @param name sets name of contact person into @ref name_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_name(const std::string& name);

        /**
        * Sets contact organization name.
        * @param organization sets full trade name of organization into @ref organization_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_organization(const std::string& organization);

        /**
        * Sets contact street1 part of address.
        * @param street1 sets part of address into @ref street1_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_street1(const std::string& street1);

        /**
        * Sets contact street2 part of address.
        * @param street2 sets part of address into @ref street2_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_street2(const std::string& street2);

        /**
        * Sets contact street3 part of address.
        * @param street3 sets part of address into @ref street3_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_street3(const std::string& street3);

        /**
        * Sets contact city part of address.
        * @param city sets part of address - city into @ref city_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_city(const std::string& city);

        /**
        * Sets contact region part of address.
        * @param stateorprovince sets part of address - region into @ref stateorprovince_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_stateorprovince(const std::string& stateorprovince);

        /**
        * Sets contact postal code part of address.
        * @param postalcode sets part of address - postal code into @ref postalcode_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_postalcode(const std::string& postalcode);

        /**
        * Sets contact country part of address.
        * @param country sets two character country code or country name into @ref country_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_country(const std::string& country);

        /**
        * Sets contact telephone number.
        * @param telephone sets telephone number into @ref telephone_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_telephone(const std::string& telephone);

        /**
        * Sets contact fax number.
        * @param fax sets fax number into @ref fax_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_fax(const std::string& fax);

        /**
        * Sets contact e-mail address.
        * @param email sets e-mail address into @ref email_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_email(const std::string& email);

        /**
        * Sets contact e-mail address for notifications.
        * @param notifyemail sets e-mail address for notifications into @ref notifyemail_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_notifyemail(const std::string& notifyemail);

        /**
        * Sets contact taxpayer identification number.
        * @param vat sets taxpayer identification number into @ref vat_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_vat(const std::string& vat);

        /**
        * Sets contact type of identification.
        * @param ssntype sets type of identification into @ref ssntype_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_ssntype(const std::string& ssntype);

        /**
        * Sets contact type of identification.
        * @param ssn sets unambiguous identification number into @ref ssn_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_ssn(const std::string& ssn);

        /**
        * Sets whether to reveal contact name.
        * @param disclosename sets whether to reveal contact name into @ref disclosename_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_disclosename(const bool disclosename);

        /**
        * Sets whether to reveal organization name.
        * @param discloseorganization sets whether to reveal organization name into @ref discloseorganization_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_discloseorganization(const bool discloseorganization);

        /**
        * Sets whether to reveal address.
        * @param discloseaddress sets whether to reveal contact address into @ref discloseaddress_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_discloseaddress(const bool discloseaddress);

        /**
        * Sets whether to reveal telephone number.
        * @param disclosetelephone sets whether to reveal telephone number into @ref disclosetelephone_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_disclosetelephone(const bool disclosetelephone);

        /**
        * Sets whether to reveal fax number.
        * @param disclosefax sets whether to reveal fax number into @ref disclosefax_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_disclosefax(const bool disclosefax);

        /**
        * Sets whether to reveal e-mail address.
        * @param discloseemail sets whether to reveal e-mail address into @ref discloseemail_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_discloseemail(const bool discloseemail);

        /**
        * Sets whether to reveal taxpayer identification number.
        * @param disclosevat sets whether to reveal taxpayer identification number into @ref disclosevat_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_disclosevat(const bool disclosevat);

        /**
        * Sets whether to reveal unambiguous identification number.
        * @param discloseident sets whether to reveal unambiguous identification number into @ref discloseident_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_discloseident(const bool discloseident);

        /**
        * Sets whether to reveal e-mail address for notifications.
        * @param disclosenotifyemail sets whether to reveal e-mail address for notifications into @ref disclosenotifyemail_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_disclosenotifyemail(const bool disclosenotifyemail);

        /**
        * Sets logger request id
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateContact& set_logd_request_id(unsigned long long logd_request_id);

        /**
        * Executes create
        * @param ctx contains reference to database and logging interface
        * @param returned_timestamp_pg_time_zone_name is postgresql time zone name of the returned timestamp
        * @return timestamp of the contact creation
        */
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//CreateContact
}
#endif // CREATE_CONTACT_H_
