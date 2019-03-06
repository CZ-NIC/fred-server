/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  header of domain browser implementation
 */

#ifndef DOMAIN_BROWSER_HH_F6A0584311AA4C86B546FD690C10A2FA
#define DOMAIN_BROWSER_HH_F6A0584311AA4C86B546FD690C10A2FA

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <vector>

#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/place_address.hh"
#include "libfred/registrable_object/domain/enum_validation_extension.hh"
#include "libfred/registrable_object/nsset/nsset_dns_host.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"

namespace Fred {
namespace Backend {
namespace DomainBrowser {

/**
 * Registry object id, handle and name data.
 */
struct RegistryReference
{
    unsigned long long id; /**< database id of the object */
    std::string handle; /**< handle of the object */
    std::string name; /**< name of the object */


    RegistryReference()
        : id(0)
    {
    }
};

/**
 * Registrar detail data.
 * Returned by @ref getRegistrarDetail.
 */
struct RegistrarDetail
{
    unsigned long long id; /**< database id of the registrar */
    std::string handle; /**< handle of the registrar */
    std::string name; /**< name of the registrar */
    std::string phone; /**< phone number of the registrar */
    std::string fax; /**< fax number of the registrar */
    std::string url; /**< web address of the registrar */
    std::string address; /**< postal address of the registrar */
};

/**
 * Contact fields disclose data.
 */
struct ContactDiscloseFlags
{
    bool name; /**< whether to reveal contact name */
    bool organization; /**< whether to reveal organization */
    bool email; /**< whether to reveal email address */
    bool address; /**< whether to reveal address */
    bool telephone; /**< whether to reveal phone number */
    bool fax; /**< whether to reveal fax number */
    bool ident; /**< whether to reveal unambiguous identification number */
    bool vat; /**< whether to reveal taxpayer identification number */
    bool notify_email; /**< whether to reveal notify email */


    ContactDiscloseFlags()
        : name(false),
          organization(false),
          email(false),
          address(false),
          telephone(false),
          fax(false),
          ident(false),
          vat(false),
          notify_email(false)
    {
    }
};

/**
 * Contact detail data.
 * Returned by @ref getContactDetail.
 */
struct ContactDetail
{
    unsigned long long id; /**< database id of the contact object*/
    std::string handle; /**< contact handle */
    std::string roid; /**< registry object identifier of the contact */
    RegistryReference sponsoring_registrar; /**< registrar administering the contact */
    boost::posix_time::ptime creation_time; /**< creation time of the contact in set local zone*/
    Nullable<boost::posix_time::ptime> update_time; /**< last update time of the contact in set local zone*/
    Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in set local zone*/
    std::string authinfopw; /**< password for transfer */
    Nullable<std::string> name; /**< name of contact person */
    Nullable<std::string> organization; /**< full trade name of organization */
    LibFred::Contact::PlaceAddress permanent_address; /**< required contact address */
    Nullable<LibFred::Contact::PlaceAddress> mailing_address; /**< optional mailing address */
    Nullable<std::string> telephone; /**<  telephone number */
    Nullable<std::string> fax; /**< fax number */
    Nullable<std::string> email; /**< e-mail address */
    Nullable<std::string> notifyemail; /**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
    Nullable<std::string> vat; /**< taxpayer identification number */
    Nullable<std::string> ssntype; /**< type of identification from enumssntype table */
    Nullable<std::string> ssn; /**< unambiguous identification number e.g. social security number, identity card number, date of birth */
    ContactDiscloseFlags disclose_flags; /**< contact fields disclose flags*/
    std::vector<std::string> state_codes; /**< object states names from db. table enum_object_states*/
    bool is_owner; /**< whether user contact is the same as requested contact */
    Nullable<bool> warning_letter; /**< contact preference for sending domain expiration letters */


    ContactDetail()
        : id(0),
          is_owner(false)
    {
    }
};

/**
 * Domain detail data
 * Returned by @ref getDomainDetail.
 */
struct DomainDetail
{
    unsigned long long id; /**< id of the domain */
    std::string fqdn; /**< fully qualified domain name */
    std::string roid; /**< registry object identifier of domain */
    RegistryReference sponsoring_registrar; /**< registrar administering the domain */
    boost::posix_time::ptime creation_time; /**< creation time of the domain in set local zone*/
    Nullable<boost::posix_time::ptime> update_time; /**< last update time of the domain in set local zone*/
    std::string authinfopw; /**< password for transfer */
    RegistryReference registrant; /**< owner of domain*/
    boost::gregorian::date expiration_date; /**< domain expiration local date */
    Nullable<LibFred::ENUMValidationExtension> enum_domain_validation; /**< ENUM domain validation extension info, is ENUM domain if set */
    RegistryReference nsset; /**< domain nsset */
    RegistryReference keyset; /**< domain keyset */
    std::vector<RegistryReference> admins; /**< domain admin contacts */
    std::vector<std::string> state_codes; /**< object state names from db. table enum_object_states */
    bool is_owner; /**< whether user contact is the same as domain owner*/
    bool is_admin; /**< whether user contact is the same as domain admin*/


    DomainDetail()
        : id(0),
          is_owner(false),
          is_admin(false)
    {
    }
};

/**
 * Nsset detail data
 * Returned by @ref getNssetDetail.
 */
struct NssetDetail
{
    unsigned long long id; /**< database id of the nsset */
    std::string handle; /**< nsset handle */
    std::string roid; /**< registry object identifier of nsset */
    RegistryReference sponsoring_registrar; /**< registrar administering the nsset */
    boost::posix_time::ptime creation_time; /**< creation time of the nsset in set local zone*/
    Nullable<boost::posix_time::ptime> transfer_time; /**< last transfer time of the nsset in set local zone*/
    Nullable<boost::posix_time::ptime> update_time; /**< last update time of the nsset in set local zone*/
    RegistryReference create_registrar; /**< registrar that created the nsset */
    RegistryReference update_registrar; /**< registrar that updated the nsset */
    std::string authinfopw; /**< password for transfer */
    std::vector<RegistryReference> admins; /**< nsset admin contacts */
    std::vector<LibFred::DnsHost> hosts; /**< nsset DNS hosts */
    std::vector<std::string> state_codes; /**< object states names from db. table enum_object_states*/
    short report_level; /**< nsset level of technical checks */
    bool is_owner; /**< user contact is owner of the nsset if it's also admin contact*/


    NssetDetail()
        : id(0),
          report_level(0),
          is_owner(false)
    {
    }
};

/**
 * DNSKey data
 */
struct DNSKey
{
    unsigned short flags; /**< the flags field */
    unsigned short protocol; /**< the protocol field, only valid value is 3*/
    unsigned short alg; /**< the algorithm field identifies the public key's cryptographic algorithm, values can be found in RFC 4034 Apendix A.1. */
    std::string key; /**< the public key field in base64 encoding */
};

/**
 * Keyset detail data
 * Returned by @ref getKeysetDetail.
 */
struct KeysetDetail
{
    unsigned long long id; /**< database id of the keyset */
    std::string handle; /**< keyset handle */
    std::string roid; /**< registry object identifier of keyset */
    RegistryReference sponsoring_registrar; /**< registrar administering the keyset */
    boost::posix_time::ptime creation_time; /**< creation time of the keyset in set local zone*/
    Nullable<boost::posix_time::ptime> transfer_time; /**< last transfer time of the keyset in set local zone*/
    Nullable<boost::posix_time::ptime> update_time; /**< last update time of the keyset in set local zone*/
    RegistryReference create_registrar; /**< registrar that created the keyset */
    RegistryReference update_registrar; /**< registrar that updated the keyset */
    std::string authinfopw; /**< password for transfer */
    std::vector<RegistryReference> admins; /**< keyset admin contacts */
    std::vector<DNSKey> dnskeys; /**< DNS keys */
    std::vector<std::string> state_codes; /**< object states names from db. table enum_object_states */
    bool is_owner; /**< user contact is owner of the keyset if it's also admin contact*/


    KeysetDetail()
        : id(0),
          is_owner(false)
    {
    }
};

/**
 * Contact disclose flags to be set.
 */
struct ContactDiscloseFlagsToSet
{
    bool email; /**< whether to reveal email address */
    bool address; /**< whether to reveal address */
    bool telephone; /**< whether to reveal phone number */
    bool fax; /**< whether to reveal fax number */
    bool ident; /**< whether to reveal unambiguous identification number */
    bool vat; /**< whether to reveal taxpayer identification number */
    bool notify_email; /**< whether to reveal notify email */


    ContactDiscloseFlagsToSet()
        : email(false),
          address(false),
          telephone(false),
          fax(false),
          ident(false),
          vat(false),
          notify_email(false)
    {
    }
};

/**
 * Next domain state data.
 */
struct NextDomainState
{
    std::string state_code; /**< next state code */
    boost::gregorian::date state_date; /**< next state date */


    NextDomainState()
    {
    }

    /**
     * Init both members.
     */
    NextDomainState(
            const std::string& _state_code,
            const boost::gregorian::date& _state_date)
        : state_code(_state_code),
          state_date(_state_date)
    {
    }
};

/**
 * element of DomainList
 */
struct DomainListData
{
    unsigned long long id; /**< id of the domain */
    std::string fqdn; /**< fully qualified domain name */
    unsigned long long external_importance; /**<  bitwise OR of importance values of states with external flag or @ref lowest_status_importance_ value if bitwise OR is zero */
    Nullable<NextDomainState> next_state; /**< next state of the domain (if any) according to current date and expiration date, outzone date and delete date of the domain with its date*/
    bool have_keyset; /**< domain have keyset flag */
    std::string user_role; /**< domainbrowser user relation to the domain (holder/admin/'') */
    std::string registrar_handle; /**< domain registrar handle*/
    std::string registrar_name; /**< domain registrar name*/
    std::vector<std::string> state_code; /**< domain states*/
    bool is_server_blocked; /**< domain blocked flag */


    DomainListData()
        : id(0),
          external_importance(0),
          have_keyset(false),
          is_server_blocked(false)
    {
    }
};

/**
 * complete domain list data
 */
struct DomainList
{
    std::vector<DomainListData> dld; /**< list of domain data */
    bool limit_exceeded; /**< there are more data to get using higher offset in next call*/


    DomainList()
        : limit_exceeded(false)
    {
    }
};

/**
 * element of NssetList
 */
struct NssetListData
{
    unsigned long long id; /**< id of the nsset */
    std::string handle; /**< nsset handle */
    unsigned long long domain_count; /**<  number of domains using this nsset */
    std::string registrar_handle; /**< nsset registrar handle*/
    std::string registrar_name; /**< nsset registrar name*/
    unsigned long long external_importance; /**<  bitwise OR of importance values of states with external flag or next higher power of 2 value if bitwise OR is zero */
    std::vector<std::string> state_code; /**< nsset states*/
    bool is_server_blocked; /**< whether nsset have serverBlocked state */


    NssetListData()
        : id(0),
          domain_count(0),
          external_importance(0),
          is_server_blocked(false)
    {
    }
};

/**
 * complete nsset list data
 */
struct NssetList
{
    std::vector<NssetListData> nld; /**< list of nsset data */
    bool limit_exceeded; /**< there are more data to get using higher offset in next call*/


    NssetList()
        : limit_exceeded(false)
    {
    }
};

/**
 * element of KeysetList
 */
struct KeysetListData
{
    unsigned long long id; /**< id of the keyset */
    std::string handle; /**< keyset handle */
    unsigned long long domain_count; /**<  number of domains using this keyset */
    std::string registrar_handle; /**< keyset registrar handle*/
    std::string registrar_name; /**< keyset registrar name*/
    unsigned long long external_importance; /**<  bitwise OR of importance values of states with external flag or next higher power of 2 value if bitwise OR is zero */
    std::vector<std::string> state_code; /**< keyset states*/
    bool is_server_blocked; /**< whether keyset have serverBlocked state*/


    KeysetListData()
        : id(0),
          domain_count(0),
          external_importance(0),
          is_server_blocked(false)
    {
    }
};

/**
 * complete keyset list data
 */
struct KeysetList
{
    std::vector<KeysetListData> kld; /**< list of keyset data */
    bool limit_exceeded; /**< there are more data to get using higher offset in next call*/


    KeysetList()
        : limit_exceeded(false)
    {
    }
};

/**
 * element of MergeContactCandidateList
 */
struct MergeContactCandidateData
{
    unsigned long long id; /**< id of the contact */
    std::string handle; /**< contact handle */
    unsigned long long domain_count; /**<  number of domains linked with this contact */
    unsigned long long nsset_count; /**<  number of nssets linked with this contact */
    unsigned long long keyset_count; /**<  number of keysets linked with this contact */
    std::string registrar_handle; /**< contact registrar handle*/
    std::string registrar_name; /**< contact registrar name*/


    MergeContactCandidateData()
        : id(0),
          domain_count(0),
          nsset_count(0),
          keyset_count(0)
    {
    }
};

/**
 * complete merge contact candidates list data
 */
struct MergeContactCandidateList
{
    std::vector<MergeContactCandidateData> mccl; /**< list of merge contact candidates data */
    bool limit_exceeded; /**< there are more data to get using higher offset in next call*/


    MergeContactCandidateList()
        : limit_exceeded(false)
    {
    }
};

/**
 * object state description
 */
struct StatusDesc
{
    std::string state_code; /**< state name */
    std::string state_desc; /**< state description in some language */


    StatusDesc(
            const std::string& _state_code,
            const std::string& _state_desc)
        : state_code(_state_code),
          state_desc(_state_desc)
    {
    }
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
    const char* what() const noexcept
    {
        return "internal server error";
    }
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
    const char* what() const noexcept
    {
        return "given contact (representing user who is calling the method) does not exist";
    }
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
    const char* what() const noexcept
    {
        return "registry object with specified ID does not exist";
    }
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
    const char* what() const noexcept
    {
        return "given parameter value (like SortSpec.field or handle) is not valid";
    }
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
    const char* what() const noexcept
    {
        return "given contact (user) does not have access to requested object";
    }
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
    const char* what() const noexcept
    {
        return "object has a status that does not allow update";
    }
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
    const char* what() const noexcept
    {
        return "unable to merge given contacts";
    }
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
    std::string update_registrar_; /**< handle of registrar performing the updates */
    unsigned int domain_list_limit_; /**< domain list chunk size */
    unsigned int nsset_list_limit_; /**< nsset list chunk size */
    unsigned int keyset_list_limit_; /**< keyset list chunk size */
    unsigned int contact_list_limit_; /**< contact list chunk size */

    unsigned long long lowest_status_importance_; /**< the lower the importance, the higher the importance value, so that the lowest importance is MAX(enum_object_states.importance) * 2 */

    /**
     * Fill object state codes.
     * @param ctx contains reference to database and logging interface
     * @param object_id is database id of object
     * @returns list of object state codes
     */
    std::vector<std::string> get_object_states(
            LibFred::OperationContext& ctx,
            unsigned long long object_id);


    /**
     * Fill authinfo into given string.
     * @param user_is_owner means that user contact owns requested object so that transfer password can be revealed
     * @param authinfopw is transfer password of requested object
     * @return transfer password returned to the user, if user is not owner of requested object output is "********"
     */
    std::string filter_authinfo(
            bool user_is_owner,
            const std::string& authinfopw);


    /**
     * Get next domain state from given dates.
     *
     * \verbatim
     *
     # resolve next domain state:
     #    today   exdate         protected period
     #      |       |<- - - - - - - - - - - - - - - - - - ->|
     # |------------|-------------------|-------------------|------------>
     #             0|                +30|                +61|
     #          expiration           outzone              delete
     #
     # \endverbatim
     * @param today_date current date
     * @param expiration_date domain expiration
     * @param outzone_date domain outzone date
     * @param delete_date domain delete date
     * @return next domain state if there is one
     */
    Nullable<NextDomainState> getNextDomainState(
            const boost::gregorian::date& today_date,
            const boost::gregorian::date& expiration_date,
            const boost::gregorian::date& outzone_date,
            const boost::gregorian::date& delete_date);


public:
    DomainBrowser(
            const std::string& server_name,
            const std::string& update_registrar_handle,
            unsigned int domain_list_limit,
            unsigned int nsset_list_limit,
            unsigned int keyset_list_limit,
            unsigned int contact_list_limit);


    virtual ~DomainBrowser();


    /**
     * Gets database id of the contact.
     * @param handle is object registry handle of the contact
     * @return contact database id
     * @throw @ref ObjectNotExists if contact not found or anything else in case of failure
     */
    unsigned long long getContactId(const std::string& handle);


    /**
     * Returns registrar detail.
     * @param user_contact_id contains database id of the user contact
     * @param registrar_handle contains handle of the registrar
     * @return registrar detail data.
     */
    RegistrarDetail getRegistrarDetail(
            unsigned long long user_contact_id,
            const std::string& registrar_handle);


    /**
     * Returns contact detail.
     * @param user_contact_id contains database id of the user contact
     * @param contact_id contains database id of the contact
     * @return contact detail data.
     */
    ContactDetail getContactDetail(
            unsigned long long user_contact_id,
            unsigned long long contact_id);


    /**
     * Returns domain detail.
     * @param user_contact_id contains database id of the user contact
     * @param domain_id contains database id of the domain
     * @return domain detail data.
     */
    DomainDetail getDomainDetail(
            unsigned long long user_contact_id,
            unsigned long long domain_id);


    /**
     * Returns nsset detail.
     * @param user_contact_id contains database id of the user contact
     * @param nsset_id contains database id of the nsset
     * @return nsset detail data.
     */
    NssetDetail getNssetDetail(
            unsigned long long user_contact_id,
            unsigned long long nsset_id);


    /**
     * Returns keyset detail.
     * @param user_contact_id contains database id of the user contact
     * @param keyset_id contains database id of the keyset
     * @return keyset detail data.
     */
    KeysetDetail getKeysetDetail(
            unsigned long long user_contact_id,
            unsigned long long keyset_id);


    /**
     * Sets contact disclose flags.
     * @param user_contact_id contains database id of the user contact
     * @param flags contains contact disclose flags
     * @param request_id is id of the new entry in log_entry database table
     * @return true if disclose flags were set, false if not or exception in case of failure
     */
    bool setContactDiscloseFlags(
            unsigned long long user_contact_id,
            const ContactDiscloseFlagsToSet& flags,
            unsigned long long request_id);


    /**
     * Sets contact transfer password.
     * @param user_contact_id contains database id of the user contact which is the contact to be modified
     * @param authinfo is new transfer password
     * @param request_id is id of the new entry in log_entry database table
     * @return true if authinfo were set, false if not or exception in case of failure
     */
    bool setContactAuthInfo(
            unsigned long long user_contact_id,
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
    bool setObjectBlockStatus(
            unsigned long long user_contact_id,
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
     * @param offset contains list offset
     * @return list of domain data with limit_exceeded flag
     */
    DomainList getDomainList(
            unsigned long long user_contact_id,
            const Optional<unsigned long long>& list_domains_for_contact_id,
            const Optional<unsigned long long>& list_domains_for_nsset_id,
            const Optional<unsigned long long>& list_domains_for_keyset_id,
            unsigned long long offset);


    /**
     * Get list of nssets administered by user contact.
     * @param user_contact_id contains database id of the user contact
     * @param list_nssets_for_contact_id if set list nssets linked to contact with given id regardless of user contact relation to listed nssets
     * @param offset contains list offset
     * @return list of nsset data with limit_exceeded flag
     */
    NssetList getNssetList(
            unsigned long long user_contact_id,
            const Optional<unsigned long long>& list_nssets_for_contact_id,
            unsigned long long offset);


    /**
     * Get list of keysets administered by user contact.
     * @param user_contact_id contains database id of the user contact
     * @param list_keysets_for_contact_id if set list keysets linked to contact with given id regardless of user contact relation to listed keysets
     * @param offset contains list offset
     * @return list of keyset data with limit_exceeded flag
     */
    KeysetList getKeysetList(
            unsigned long long user_contact_id,
            const Optional<unsigned long long>& list_keysets_for_contact_id,
            unsigned long long offset);


    /**
     * Get descriptions of public states.
     * @param lang contains language for state description "EN" or "CS"
     * @return list of status codes and descriptions
     */
    std::vector<StatusDesc> getPublicStatusDesc(const std::string& lang);


    /**
     * Get list of contacts mergeable to user contact.
     * @param user_contact_id contains database id of the user contact
     * @param offset contains list offset
     * @return merge candidate contact list with limit_exceeded flag
     */
    MergeContactCandidateList getMergeContactCandidateList(
            unsigned long long user_contact_id,
            unsigned long long offset);


    /**
     * Merge contact list to destination contact
     * @param dst_contact_id id of destination contact
     * @param contact_list id list of source contacts
     * @param request_id is id of the new entry in log_entry database table
     */
    void mergeContacts(
            unsigned long long dst_contact_id,
            const std::vector<unsigned long long>& contact_list,
            unsigned long long request_id);


    /**
     * Get server name
     * @return name for logging context
     */
    std::string get_server_name();


    /**
     * Sets contact preference for sending domain expiration letters.
     * @param user_contact_id contains database id of the user contact, to set any preference contact have to be mojeid contact, to set FALSE, contact have to be validated mojeid contact
     * @param send_expiration_letters is user preference whether to send domain expiration letters, if TRUE then send domain expiration letters, if FALSE don't send domain expiration letters
     * @param request_id is id of the new entry in log_entry database table
     */
    void setContactPreferenceForDomainExpirationLetters(
            unsigned long long user_contact_id,
            bool send_expiration_letters,
            unsigned long long request_id);


};

} // namespace Fred::Backend::DomainBrowser
} // namespace Fred::Backend
} // namespace Fred

#endif
