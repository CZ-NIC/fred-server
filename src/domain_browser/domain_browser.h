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
 *  header of domain browser implementation
 */

#ifndef DOMAIN_BROWSER_H_
#define DOMAIN_BROWSER_H_

#include <string>
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "src/fredlib/opcontext.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

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
            std::string states;/**< object states descriptions in given language from db. table enum_object_states_desc delimited by pipe '|' character */
            std::string state_codes;/**< object states names from db. table enum_object_states delimited by coma ',' character */
            bool is_owner;/**< whether user contact is the same as requested contact */

            ContactDetail()
            : id(0)
            , is_owner(false)
            {}
        };

        /**
         * Domain detail data
         * Returned by @ref getDomainDetail.
         */
        struct DomainDetail
        {
            unsigned long long id;/**< id of the domain */
            std::string fqdn;/**< fully qualified domain name */
            std::string roid;/**< registry object identifier of domain */
            RegistryReference sponsoring_registrar;/**< registrar administering the domain */
            boost::posix_time::ptime creation_time;/**< creation time of the domain in set local zone*/
            Nullable<boost::posix_time::ptime> update_time; /**< last update time of the domain in set local zone*/
            std::string authinfopw;/**< password for transfer */
            RegistryReference registrant; /**< owner of domain*/
            boost::gregorian::date expiration_date; /**< domain expiration local date */
            Nullable<Fred::ENUMValidationExtension > enum_domain_validation;/**< ENUM domain validation extension info, is ENUM domain if set */
            RegistryReference nsset; /**< domain nsset */
            RegistryReference keyset;/**< domain keyset */
            std::vector<RegistryReference> admins; /**< domain admin contacts */
            std::string states;/**< object states descriptions in given language from db. table enum_object_states_desc delimited by pipe '|' character */
            std::string state_codes;/**< object states names from db. table enum_object_states delimited by coma ',' character */
            bool is_owner;/**< whether user contact is the same as domain owner*/

            DomainDetail()
            : id(0)
            , is_owner(false)
            {}
        };

        /**
         * DNSHost data
         */
        struct DNSHost
        {
            std::string fqdn;/**< fully qualified name of the nameserver host*/
            std::string inet_addr;/**< list of IPv4 or IPv6 addresses of the nameserver host*/
        };

        /**
         * Nsset detail data
         * Returned by @ref getNssetDetail.
         */
        struct NssetDetail
        {
            unsigned long long id;/**< database id of the nsset */
            std::string handle;/**< nsset handle */
            std::string roid;/**< registry object identifier of nsset */
            RegistryReference sponsoring_registrar;/**< registrar administering the nsset */
            boost::posix_time::ptime creation_time;/**< creation time of the nsset in set local zone*/
            Nullable<boost::posix_time::ptime> transfer_time; /**< last transfer time of the nsset in set local zone*/
            Nullable<boost::posix_time::ptime> update_time; /**< last update time of the nsset in set local zone*/
            RegistryReference create_registrar;/**< registrar that created the nsset */
            RegistryReference update_registrar;/**< registrar that updated the nsset */
            std::string authinfopw;/**< password for transfer */
            std::vector<RegistryReference> admins; /**< nsset admin contacts */
            std::vector<DNSHost> hosts; /**< nsset DNS hosts */
            std::string states;/**< object states descriptions in given language from db. table enum_object_states_desc delimited by pipe '|' character */
            std::string state_codes;/**< object states names from db. table enum_object_states delimited by coma ',' character */
            short report_level; /**< nsset level of technical checks */
            bool is_owner;/**< user contact is owner of the nsset if it's also admin contact*/

            NssetDetail()
            : id(0)
            , report_level(0)
            , is_owner(false)
            {}
        };

        /**
         * DNSKey data
         */
        struct DNSKey
        {
            unsigned short flags;/**< the flags field */
            unsigned short protocol;/**< the protocol field, only valid value is 3*/
            unsigned short alg;/**< the algorithm field identifies the public key's cryptographic algorithm, values can be found in RFC 4034 Apendix A.1. */
            std::string key;/**< the public key field in base64 encoding */
        };

        /**
         * Keyset detail data
         * Returned by @ref getKeysetDetail.
         */
        struct KeysetDetail
        {
            unsigned long long id;/**< database id of the keyset */
            std::string handle;/**< keyset handle */
            std::string roid;/**< registry object identifier of keyset */
            RegistryReference sponsoring_registrar;/**< registrar administering the keyset */
            boost::posix_time::ptime creation_time;/**< creation time of the keyset in set local zone*/
            Nullable<boost::posix_time::ptime> transfer_time; /**< last transfer time of the keyset in set local zone*/
            Nullable<boost::posix_time::ptime> update_time; /**< last update time of the keyset in set local zone*/
            RegistryReference create_registrar;/**< registrar that created the keyset */
            RegistryReference update_registrar;/**< registrar that updated the keyset */
            std::string authinfopw;/**< password for transfer */
            std::vector<RegistryReference> admins; /**< keyset admin contacts */
            std::vector<DNSKey> dnskeys; /**< DNS keys */
            std::string states;/**< object states descriptions in given language from db. table enum_object_states_desc delimited by pipe '|' character */
            std::string state_codes;/**< object states names from db. table enum_object_states delimited by coma ',' character */
            bool is_owner;/**< user contact is owner of the keyset if it's also admin contact*/

            KeysetDetail()
            : id(0)
            , is_owner(false)
            {}
        };

        /**
         * Contact disclose flags to be set.
         */
        struct ContactDiscloseFlagsToSet
        {
            bool email;/**< whether to reveal email address */
            bool address;/**< whether to reveal address */
            bool telephone;/**< whether to reveal phone number */
            bool fax;/**< whether to reveal fax number */
            bool ident;/**< whether to reveal unambiguous identification number */
            bool vat;/**< whether to reveal taxpayer identification number */
            bool notify_email;/**< whether to reveal notify email */

            ContactDiscloseFlagsToSet()
            : email(false)
            , address(false)
            , telephone(false)
            , fax(false)
            , ident(false)
            , vat(false)
            , notify_email(false)
            {}
        };

        /**
         * Next domain state data.
         */
        struct NextDomainState
        {
            std::string state; /**< next state */
            boost::gregorian::date state_date; /**< next state date*/

            /**
             * Default state is "N/A" and default date is not_a_date_time.
             */
            NextDomainState()
            : state("N/A")
            {}

            /**
             * Init both members.
             */
            NextDomainState(const std::string& _state, const boost::gregorian::date& _state_date)
            : state(_state)
            , state_date(_state_date)
            {}
        };

        /**
         * Internal server error.
         * Unexpected failure, requires maintenance.
         */
        struct InternalServerError
        : virtual std::exception
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "internal server error";}
        };

        /**
         * Contact of the user requesting the service was not found.
         * The contact could have been deleted or set into inappropriate state.
         */
        struct UserNotExists
        : virtual std::exception
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "given contact (representing user who is calling the method) does not exist";}
        };

        /**
         * Requested object was not found.
         * Requested object could have been deleted or set into inappropriate state.
         */
        struct ObjectNotExists
        : virtual std::exception
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "registry object with specified ID does not exist";}
        };

        /**
         * Incorrect usage of the service.
         * Unexpected input data. This should not happen, probably result of bad interface design.
         */
        struct IncorrectUsage
        : virtual std::exception
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "given parameter value (like SortSpec.field or handle) is not valid";}
        };

        /**
         * Access to requested information is forbidden.
         */
        struct AccessDenied
        : virtual std::exception
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "given contact (user) does not have access to requested object";}
        };

        /**
         * Requested object does not allow update.
         * Requested object has a status that does not allow update.
         */
        struct ObjectBlocked
        : virtual std::exception
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "object has a status that does not allow update";}
        };

        /**
         * Invalid contacts.
         * Unable to merge contacts.
         */
        struct InvalidContacts
        : virtual std::exception
        {
            /**
             * Returns failure description.
             * @return string with the general cause of the current error.
             */
            const char* what() const throw() {return "unable to merge given contacts";}
        };


        /**
         * Type of blocking to be applied (value related to enum Registry::DomainBrowser::ObjectBlockType)
         */
        static const unsigned BLOCK_TRANSFER = 0;
        static const unsigned UNBLOCK_TRANSFER = 1;
        static const unsigned BLOCK_TRANSFER_AND_UPDATE = 2;
        static const unsigned UNBLOCK_TRANSFER_AND_UPDATE = 3;
        static const unsigned INVALID_BLOCK_TYPE = 10;
        class DomainBrowser
        {
        public:
            static const std::string output_timezone;
        private:
            std::string server_name_;
            std::string update_registrar_;/**< handle of registrar performing the updates */
            unsigned int domain_list_limit_;/**< domain list chunk size */
            unsigned int nsset_list_limit_;/**< nsset list chunk size */
            unsigned int keyset_list_limit_;/**< keyset list chunk size */
            unsigned int contact_list_limit_;/**< contact list chunk size */

            unsigned int minimal_status_importance_;// NOTE: rename to lowest_status_importance_ or
                                                    //                 default_status_importance_value_ or
                                                    //                 the_most_trivial_status

            /**
             * Fill object state codes and description into given strings.
             * @param ctx contains reference to database and logging interface
             * @param object_id is database id of object
             * @param lang is required language of object state description e.g. "EN" or "CS"
             * @param state_codes is output string of object state codes delimited by '|'
             * @param states is output string with descriptions of external object states delimited by ','
             */
             void get_object_states(Fred::OperationContext& ctx, unsigned long long object_id, const std::string& lang
                     , std::string& state_codes, std::string& states);

            /**
             * Fill authinfo into given string.
             * @param user_is_owner means that user contact owns requested object so that transfer password can be revealed
             * @param authinfopw is transfer password of requested object
             * @return transfer password returned to the user, if user is not owner of requested object output is "********"
             */
            std::string filter_authinfo(bool user_is_owner,const std::string& authinfopw);

            /**
             * Get next domain state from given dates.
             *
\verbatim

 # resolve next domain state:
 #    today   exdate         protected period
 #      |       |<- - - - - - - - - - - - - - - - - - ->|
 # |------------|-------------------|-------------------|------------>
 #             0|                +30|                +61|
 #          expiration           outzone              delete

\endverbatim
             * @param today_date current date
             * @param expiration_date domain expiration
             * @param outzone_date domain outzone date
             * @param delete_date domain delete date
             */
            NextDomainState getNextDomainState(
                const boost::gregorian::date&  today_date,
                const boost::gregorian::date& expiration_date,
                const boost::gregorian::date& outzone_date,
                const boost::gregorian::date& delete_date);

        public:
            DomainBrowser(const std::string &server_name,
                    const std::string& update_registrar_handle,
                    unsigned int domain_list_limit,
                    unsigned int nsset_list_limit,
                    unsigned int keyset_list_limit,
                    unsigned int contact_list_limit);
            virtual ~DomainBrowser();

            /**
             * Gets database id of the object.
             * @param objtype is type of the object from table enum_object_type
             * @param handle is object registry handle of the object
             * @return object database id
             * @throw @ref IncorrectUsage if objype not found, @ref ObjectNotExists if object with given type not found or anything else in case of failure
             */
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
             * @return contact detail data.
             */
            ContactDetail getContactDetail(unsigned long long user_contact_id,
                    unsigned long long contact_id,
                    const std::string& lang);

            /**
             * Returns domain detail.
             * @param user_contact_id contains database id of the user contact
             * @param domain_id contains database id of the domain
             * @param lang contains language for state description "EN" or "CS"
             * @return domain detail data.
             */
            DomainDetail getDomainDetail(unsigned long long user_contact_id,
                    unsigned long long domain_id,
                    const std::string& lang);

            /**
             * Returns nsset detail.
             * @param user_contact_id contains database id of the user contact
             * @param nsset_id contains database id of the nsset
             * @param lang contains language for state description "EN" or "CS"
             * @return nsset detail data.
             */
            NssetDetail getNssetDetail(unsigned long long user_contact_id,
                    unsigned long long nsset_id,
                    const std::string& lang);

            /**
             * Returns keyset detail.
             * @param user_contact_id contains database id of the user contact
             * @param keyset_id contains database id of the keyset
             * @param lang contains language for state description "EN" or "CS"
             * @return keyset detail data.
             */
            KeysetDetail getKeysetDetail(unsigned long long user_contact_id,
                    unsigned long long keyset_id,
                    const std::string& lang);

            /**
             * Sets contact disclose flags.
             * @param contact_id contains database id of the contact
             * @param flags contains contact disclose flags
             * @param request_id is id of the new entry in log_entry database table
             * @return true if disclose flags were set, false if not or exception in case of failure
             */
            bool setContactDiscloseFlags(
                unsigned long long contact_id,
                const ContactDiscloseFlagsToSet& flags,
                unsigned long long request_id);

            /**
             * Sets contact transfer password.
             * @param user_contact_id contains database id of the user contact
             * @param contact_id is database id of the contact to be modified
             * @param authinfo is new transfer password
             * @param request_id is id of the new entry in log_entry database table
             * @return true if authinfo were set, false if not or exception in case of failure
             */
            bool setContactAuthInfo(unsigned long long user_contact_id,
                unsigned long long contact_id,
                const std::string& authinfo,
                unsigned long long request_id);

            /**
             * Sets blocking state of the object.
             * @param user_contact_id contains database id of the user contact
             * @param objtype type of the objects to be set
             * @param object_id list of database ids of the objects to be set
             * @param block_type is type of blocking to be applied (value from enum Registry::DomainBrowser::ObjectBlockType)
             * @param blocked_objects is list of object handles of objects with SERVER_BLOCKED state or objects attempted to be partially unblocked (which is now forbidden)
             * @return false if no object blocked, true if not or exception in case of failure
             */
            bool setObjectBlockStatus(unsigned long long user_contact_id,
                const std::string& objtype,
                const std::vector<unsigned long long>& object_id,
                unsigned block_type,
                std::vector<std::string>& blocked_objects);

            /**
             * Get list of domains registered or administered by user contact except the case when contact id, nsset id or keyset id (or all of them) is set.
             * @param user_contact_id contains database id of the user contact
             * @param list_domains_for_contact_id if set list domains linked to contact with given id regardless of user contact relation to listed domains
             * @param list_domains_for_nsset_id if set list domains linked to nsset with given id regardless of user contact relation to listed domains
             * @param list_domains_for_keyset_id if set list domains linked to keyset with given id regardless of user contact relation to listed domains
             * @param lang contains language for state description "EN" or "CS"
             * @param offset contains list offset
             * @param  domain_list_out references output domain list
             * @return limit_exceeded flag
             */
            bool getDomainList(unsigned long long user_contact_id,
                const Optional<unsigned long long>& list_domains_for_contact_id,
                const Optional<unsigned long long>& list_domains_for_nsset_id,
                const Optional<unsigned long long>& list_domains_for_keyset_id,
                const std::string& lang,
                unsigned long long offset,
                std::vector<std::vector<std::string> >& domain_list_out);

            /**
             * Get list of nssets administered by user contact.
             * @param user_contact_id contains database id of the user contact
             * @param list_nssets_for_contact_id if set list nssets linked to contact with given id regardless of user contact relation to listed nssets
             * @param lang contains language for state description "EN" or "CS"
             * @param offset contains list offset
             * @param  nsset_list_out references output nsset list
             * @return limit_exceeded flag
             */
            bool getNssetList(unsigned long long user_contact_id,
                const Optional<unsigned long long>& list_nssets_for_contact_id,
                const std::string& lang,
                unsigned long long offset,
                std::vector<std::vector<std::string> >& nsset_list_out);

            /**
             * Get list of keysets administered by user contact.
             * @param user_contact_id contains database id of the user contact
             * @param list_keysets_for_contact_id if set list keysets linked to contact with given id regardless of user contact relation to listed keysets
             * @param lang contains language for state description "EN" or "CS"
             * @param offset contains list offset
             * @param  keyset_list_out references output keyset list
             * @return limit_exceeded flag
             */
            bool getKeysetList(unsigned long long user_contact_id,
                const Optional<unsigned long long>& list_keysets_for_contact_id,
                const std::string& lang,
                unsigned long long offset,
                std::vector<std::vector<std::string> >& keyset_list_out);

            /**
             * Get descriptions of public states.
             * @param lang contains language for state description "EN" or "CS"
             * @param  status_description_out references output list of descriptions
             */
            void getPublicStatusDesc(const std::string& lang,
                std::vector<std::string>& status_description_out);

            /**
             * Get list of contacts mergeable to user contact.
             * @param user_contact_id contains database id of the user contact
             * @param offset contains list offset
             * @param  contact_list_out references output candidate contact list
             * @return limit_exceeded flag
             */
            bool getMergeContactCandidateList(unsigned long long user_contact_id,
                unsigned long long offset,
                std::vector<std::vector<std::string> >& contact_list_out);

            /**
             * Merge contact list to destination contact
             * @param dst_contact_id id of destination contact
             * @param contact_list id list of source contacts
             * @param request_id is id of the new entry in log_entry database table
             */
            void mergeContacts(unsigned long long dst_contact_id,
                const std::vector<unsigned long long>& contact_list,
                unsigned long long request_id);

            /**
             * Get server name
             * @return name for logging context
             */
            std::string get_server_name();
        };//class DomainBrowser

    }//namespace DomainBrowserImpl
}//namespace Registry

#endif // DOMAIN_BROWSER_H_
