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
 *  @domain_browser.h
 *  header of domain browser implementation
 */

#ifndef DOMAIN_BROWSER_H_
#define DOMAIN_BROWSER_H_

#include <string>
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread/mutex.hpp>

#include "src/fredlib/opexception.h"

#include "cfg/handle_registry_args.h"

namespace Registry
{
    namespace DomainBrowserImpl
    {

        /**
         * Registry object id, handle and name data.
         */
        struct RegistryReference
        {
            unsigned long long id;/**< database id of the object */
            std::string handle;/**< handle of the object */
            std::string name;/**< name of the object */

            RegistryReference()
            : id(0)
            {}
        };
        /**
         * Registrar detail data.
         * Returned by @ref getRegistrarDetail.
         */
        struct RegistrarDetail
        {
            unsigned long long id;/**< database id of the registrar */
            std::string handle;/**< handle of the registrar */
            std::string name;/**< name of the registrar */
            std::string phone;/**< phone number of the registrar */
            std::string fax;/**< fax number of the registrar */
            std::string url;/**< web address of the registrar */
            std::string address; /**< postal address of the registrar */
        };

        /**
         * Contact fields disclose data.
         */
        struct ContactDiscloseFlags
        {
            bool name;/**< whether to reveal contact name */
            bool organization;/**< whether to reveal organization */
            bool email;/**< whether to reveal email address */
            bool address;/**< whether to reveal address */
            bool telephone;/**< whether to reveal phone number */
            bool fax;/**< whether to reveal fax number */
            bool ident;/**< whether to reveal unambiguous identification number */
            bool vat;/**< whether to reveal taxpayer identification number */
            bool notify_email;/**< whether to reveal notify email */

            ContactDiscloseFlags()
            : name(false)
            , organization(false)
            , email(false)
            , address(false)
            , telephone(false)
            , fax(false)
            , ident(false)
            , vat(false)
            , notify_email(false)
            {}
        };


        /**
         * Contact detail data.
         * Returned by @ref getContactDetail.
         */
        struct ContactDetail
        {
            unsigned long long id;/**< database id of the contact object*/
            std::string handle;/**< contact handle */
            std::string roid;/**< registry object identifier of the contact */
            RegistryReference sponsoring_registrar;/**< registrar administering the contact */
            boost::posix_time::ptime creation_time;/**< creation time of the contact in set local zone*/
            Nullable<boost::posix_time::ptime> update_time; /**< last update time of the contact in set local zone*/
            Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in set local zone*/
            std::string authinfopw;/**< password for transfer */
            Nullable<std::string> name ;/**< name of contact person */
            Nullable<std::string> organization;/**< full trade name of organization */
            Nullable<std::string> street1;/**< part of address */
            Nullable<std::string> street2;/**< part of address */
            Nullable<std::string> street3;/**< part of address*/
            Nullable<std::string> city;/**< part of address - city */
            Nullable<std::string> stateorprovince;/**< part of address - region */
            Nullable<std::string> postalcode;/**< part of address - postal code */
            Nullable<std::string> country;/**< two character country code or country name */
            Nullable<std::string> telephone;/**<  telephone number */
            Nullable<std::string> fax;/**< fax number */
            Nullable<std::string> email;/**< e-mail address */
            Nullable<std::string> notifyemail;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
            Nullable<std::string> vat;/**< taxpayer identification number */
            Nullable<std::string> ssntype;/**< type of identification from enumssntype table */
            Nullable<std::string> ssn;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
            ContactDiscloseFlags disclose_flags;/**< contact fields disclose flags*/
            std::string states;
            std::string state_codes;

            ContactDetail()
            : id(0)
            {}
        };

        /**
         * Internal server error.
         * Unexpected failure, requires maintenance. Exception should contain boost diagnostic information.
         */
        struct InternalServerError
        : virtual Fred::OperationException
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "internal server error";}
        };

        /**
         * Contact of the user requesting the service was not found.
         * The contact could have been deleted or set into inappropriate state. Exception should contain boost diagnostic information.
         */
        struct UserNotExists
        : virtual Fred::OperationException
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "given contact (representing user who is calling the method) does not exist";}
        };

        /**
         * Requested object was not found.
         * Requested object could have been deleted or set into inappropriate state. Exception should contain boost diagnostic information.
         */
        struct ObjectNotExists
        : virtual Fred::OperationException
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "registry object with specified ID does not exist";}
        };

        /**
         * Incorrect usage of the service.
         * Unexpected input data. This should not happen, probably result of bad interface design. Exception should contain boost diagnostic information.
         */
        struct IncorrectUsage
        : virtual Fred::OperationException
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "given parameter value (like SortSpec.field or handle) is not valid";}
        };

        /**
         * Access to requested information is forbidden.
         * Exception should contain boost diagnostic information.
         */
        struct AccessDenied
        : virtual Fred::OperationException
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "given contact (user) does not have access to requested object";}
        };

        /**
         * Requested object does not allow update.
         * Requested object has a status that does not allow update. Exception should contain boost diagnostic information.
         */
        struct ObjectBlocked
        : virtual Fred::OperationException
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "object has a status that does not allow update";}
        };

        class DomainBrowser
        {
            std::string server_name_;
        public:
            //dummy decl - impl
            DomainBrowser(const std::string &server_name);
            virtual ~DomainBrowser();

            unsigned long long getObjectRegistryId(const std::string& objtype, const std::string& handle);

            /**
             * Returns registrar detail.
             * @param user_contact_id contains database id of the user contact
             * @param registrar_handle contains handle of the registrar
             * @return registrar detail data.
             */
            RegistrarDetail getRegistrarDetail(unsigned long long user_contact_id, const std::string& registrar_handle);

            /**
             * Returns contact detail.
             * @param user_contact_id contains database id of the user contact
             * @param contact_id contains database id of the contact
             * @param lang contains language for state description "EN" or "CS"
             * @return registrar detail data.
             */
            ContactDetail getContactDetail(unsigned long long user_contact_id,
                    unsigned long long contact_id,
                    const std::string& lang);

            std::string get_server_name();
        };//class DomainBrowser

    }//namespace DomainBrowserImpl
}//namespace Registry

#endif // DOMAIN_BROWSER_H_
