/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef _WHOIS_H_
#define _WHOIS_H_

#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/asio/ip/address.hpp>
#include <src/fredlib/object_state/get_object_states.h>
#include <src/fredlib/domain/info_domain_output.h>
#include <src/fredlib/nsset/info_nsset_output.h>
#include <src/fredlib/opcontext.h>

#include "util/db/nullable.h"

namespace Registry {
namespace WhoisImpl {

/**
 * Registrar's address details.
 */
struct PlaceAddress
{
    std::string street1; /**< Registrar's street #1 */
    std::string street2; /**< Registrar's street #2 */
    std::string street3; /**< Registrar's street #3 */
    std::string city; /**< Registrar's city */
    std::string stateorprovince; /**< Registrar's state or province */
    std::string postal_code; /**< Registrar's postal code */
    std::string country_code; /**< Registrar's country code */
};

/**
 * Registrar details data.
 */
struct Registrar
{
    unsigned long long id; /**< database id of the registrar */
    std::string handle; /**< handle of the registrar */
    std::string name; /**< name of the registrar */
    std::string organization; /**< organization of the registrar */
    std::string url; /**< web address of the registrar */
    std::string phone; /**< phone number of the registrar */
    std::string fax; /**< fax address of the registrar */
    PlaceAddress address; /**< address details of the registrar */

    Registrar()
    : id(0)
    {}
};

/**
 * Personal identification of the contact.
 */
struct ContactIdentification
{
    std::string identification_type; /**< type of the document */
    std::string identification_data; /**< actual information of the identification */
};

/**
 * Contact details data.
 */
struct Contact
{
    std::string handle; /**< handle of the contact */
    std::string organization; /**< organization of the contact */
    bool disclose_organization; /**< whether to disclose the organization of the contact */
    std::string name; /**< name of the contact */
    bool disclose_name; /**< whether to disclose the name of the contact */
    PlaceAddress address; /**< address of the contact */
    bool disclose_address; /**< whether to disclose the address of the contact */
    std::string phone; /**< phone number of the contact */
    bool disclose_phone; /**< whether to disclose the phone number of the contact */
    std::string fax; /**< fax number of the contact */
    bool disclose_fax; /**< whether to disclose the fax number of the contact */
    std::string email; /**< email address of the contact */
    bool disclose_email; /**< whether to disclose the email address of the contact */
    std::string notify_email; /**< notification email of the contact */
    bool disclose_notify_email; /**< whether to disclose the notification email of the contact */
    ContactIdentification identification; /**< identification of the contact */
    bool disclose_identification; /**< whether to disclose the identification of the contact */
    std::string vat_number; /**< taxpayer identification number of the contact */
    bool disclose_vat_number; /**< whether to disclose the of taxpayer identification number the contact */
    //?
    std::string creating_registrar_handle; /**< registrar created by the contact */
    //?
    std::string sponsoring_registrar_handle; /**< registrar sponsored the contact */
    boost::posix_time::ptime created; /**< creation date of the contact */
    Nullable<boost::posix_time::ptime> changed; /**< date contact was last changed */
    Nullable<boost::posix_time::ptime> last_transfer; /**< date contact was last transfered */
    std::vector<std::string> statuses; /**< statuses of the contact */

    Contact()
    : disclose_organization(false),
      disclose_name(false),
      disclose_address(false),
      disclose_phone(false),
      disclose_fax(false),
      disclose_email(false),
      disclose_notify_email(false),
      disclose_identification(false),
      disclose_vat_number(false)
    {}
};

/**
 * Details data of the name server.
 */
struct NameServer
{
    //?
    std::string fqdn; /**< fully qualified domain name **/
    std::vector<boost::asio::ip::address> ip_addresses; /**< IP addresses of the name server */
};

/**
 * Details data of the NameServer set.
 */
struct NSSet
{
    std::string handle; /**< handle of the name servers' set */
    std::vector<NameServer> nservers; /**< name servers' handles of the set */
    std::vector<std::string> tech_contact_handles; /**< technical contact handles of the name servers' set */
    std::string registrar_handle; /**< registrar handle of the name servers' set */
    boost::posix_time::ptime created; /**< creation date of the name servers' set */
    Nullable<boost::posix_time::ptime> changed; /**< date name servers' set was last changed */
    Nullable<boost::posix_time::ptime> last_transfer; /**< date name servers' set was last transfered */
    std::vector<std::string> statuses; /**< statuses of the name servers' set */
};

/**
 * Details data of the list of NSSets.
 */
struct NSSetSeq
{
    std::vector<NSSet> content; /**< Content of the list */
    bool limit_exceeded; /**< there are more data to get using higher offset in next call*/

    NSSetSeq()
    : limit_exceeded(false)
    {}
};

/**
 * Details data of the DNSKey.
 */
struct DNSKey
{
    short flags; /**< flags of the DNSKey */
    short protocol; /**< protocol type of the DNSKey */
    short alg; /**< algorithm type of the DNSKey */
    std::string public_key; /**< public key of the DNSKey */
};

/**
 * Details data of the KeySet.
 */
struct KeySet
{
    std::string handle; /**< of the KeySet */
    std::vector<DNSKey> dns_keys; /**< list of DNSKeys of the KeySet */
    std::vector<std::string> tech_contact_handles; /**< technical contact handles of the KeySet */
    //?
    std::string registrar_handle; /**< registrar handle of the KeySet */
    boost::posix_time::ptime created; /**< date of creation of the KeySet */
    Nullable<boost::posix_time::ptime> changed; /**< date the KeySet was last changed */
    Nullable<boost::posix_time::ptime> last_transfer; /**< date the KeySet was last transfered */
    std::vector<std::string> statuses; /**< statuses of the KeySet */
};

/**
 * Details data of the list of KeySets.
 */
struct KeySetSeq
{
    std::vector<KeySet> content; /**< Content of the list */
    bool limit_exceeded; /**< there are more data to get using higher offset in next call*/

    KeySetSeq()
    : limit_exceeded(false)
    {}
};

/**
 * Details data of the domain.
 */
struct Domain
{
    std::string fqdn; /**< fully qualified domain name **/
    std::string registrant_handle; /**< registrant_handle of the domain **/
    std::vector<std::string> admin_contact_handles; /**< admin contact handles of the domain **/
    std::string nsset_handle; /**< NSSet handle of the domain **/
    std::string keyset_handle; /**< KeySet handle of the domain **/
    std::string registrar_handle; /**< registrar handle of the domain **/
    std::vector<std::string> statuses; /**< statuses of the domain **/
    boost::posix_time::ptime registered; /**< date the domain was registered **/
    Nullable<boost::posix_time::ptime> changed; /**< date the domain was last changed **/
    Nullable<boost::posix_time::ptime> last_transfer; /**< date the domain was last transfered **/
    boost::gregorian::date expire; /**< expiration date of the domain **/
    Nullable<boost::gregorian::date> validated_to; /**< date till which domain is validated **/
};

/**
 * Details data of the list of domains.
 */
struct DomainSeq
{
    std::vector<Domain> content; /**< Content of the list */
    bool limit_exceeded; /**< there are more data to get using higher offset in next call*/

    DomainSeq()
    : limit_exceeded(false)
    {}
};

/**
 * Details data of the group of registrars.
 */
struct RegistrarGroup
{
    std::string name; /**< name of the group */
    std::vector<std::string> members; /**< members of the group */
};

//?
/**
 * Details data of the registrar certification.
 */
struct RegistrarCertification
{
    std::string registrar_handle; /**< handle of the registrar */
    short score; /**< score of the registrar */
    unsigned long long evaluation_file_id; /**< id of the evaluation file of registrar */
};

/**
 * Description of the object status.
 */
struct ObjectStatusDesc
{
    std::string handle; /**< handle of the description */
    std::string name; /**< the description of the object */
};

/**
 * Object requested by ID was not found.
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
 * Object requested by handle was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct InvalidHandle
: virtual std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const throw() {return "registry object with specified handle does not exist";}
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
 * Label of the object was incorrectly specified.
 */
struct InvalidLabel
: virtual std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const throw() {return "the label is invalid";}
};

//?
/**
 * Zone of the domain is not managed by the service.
 */
struct UnmanagedZone
: virtual std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const throw() {return "this zone is not managed";}
};

/**
 * Domain name contains more labels than allowed for its type.
 */
struct TooManyLabels
: virtual std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const throw() {return "domain has too many labels";}
};

/**
 * Database does not contain the localization of the description of the object.
 */
struct MissingLocalization
: virtual std::exception
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    const char* what() const throw() {return "the localization is missing";}
};

//?
typedef std::vector< std::pair<std::string, std::string> > str_str_vector;
typedef std::vector<Fred::ObjectStateData> ObjectStateDataList;
typedef std::vector<Fred::InfoNssetOutput> InfoNssetOutputList;
typedef std::vector<Fred::InfoDomainOutput> InfoDomainOutputList;

/**
 * The main class implementing the service.
 */
class Server_impl
{
private:
    std::vector<ObjectStatusDesc> get_object_status_descriptions(
        const std::string& lang,
        const std::string& type);

    DomainSeq get_domains_by_(Fred::OperationContext& ctx,
        unsigned long limit,
        const InfoDomainOutputList& domain_info);

    NSSetSeq get_nssets_by_(Fred::OperationContext& ctx,
        const InfoNssetOutputList& nss_info,
        const std::string& handle,
        unsigned long limit);

    WhoisImpl::NSSet make_nsset_from_info_data(const Fred::InfoNssetData& ind,
        Fred::OperationContext& ctx);

    WhoisImpl::Domain make_domain_from_info_data(
        const Fred::InfoDomainData& idd,
        Fred::OperationContext& ctx);
public:
    
    static const std::string output_timezone; /**< The time zone to consider for objects' dates. */

    virtual ~Server_impl() {};

    /**
     * Returns registrar by a handle.
     * @param handle contains handle of the registrar.
     * @return registrar data.
     */
    Registrar get_registrar_by_handle(const std::string& handle);

    /**
     * Returns the list of non-system registrars.
     * @return registrar list.
     */
    std::vector<Registrar> get_registrars();

    /**
     * Returns the list of registrar groups.
     * @return registrar group list.
     */
    std::vector<RegistrarGroup> get_registrar_groups();

    /**
     * Returns the list of registrar certifications.
     * @return registrar certification list.
     */
    std::vector<RegistrarCertification> get_registrar_certification_list();

    /**
     * Returns the list of managed zones.
     * @return managed zone list.
     */
    std::vector<std::string> get_managed_zone_list();

    /**
     * Returns contact by a handle.
     * @param handle contains handle of the contact.
     * @return contact data.
     */
    Contact get_contact_by_handle(const std::string& handle);

    /**
     * Returns nsset by a handle.
     * @param handle contains handle of the nsset.
     * @return nsset data.
     */
    NSSet get_nsset_by_handle(const std::string& handle);

    /**
     * Returns list of nssets by a name server handle.
     * @param handle contains handle of the name server.
     * @param limit flag whether requested limit was reached.
     * @return list of nsset data with limit_exceeded flag
     */
    NSSetSeq get_nssets_by_ns(const std::string& handle, unsigned long limit);

    /**
     * Returns list of nssets by a handle of technical contact.
     * @param handle contains handle of the technical contact.
     * @param limit flag whether requested limit was reached.
     * @return list of nsset data with limit_exceeded flag
     */
    NSSetSeq get_nssets_by_tech_c(const std::string& handle, unsigned long limit);

    /**
     * Returns name server by a handle.
     * @param handle contains handle of the name server.
     * @return name server data.
     */
    NameServer get_nameserver_by_fqdn(const std::string& handle);

    /**
     * Returns keyset by a handle.
     * @param handle contains handle of the keyset.
     * @return keyset data.
     */
    KeySet get_keyset_by_handle(const std::string& handle);

    /**
     * Returns list of keysets by a handle of technical contact.
     * @param handle contains handle of the technical contact.
     * @param limit flag whether requested limit was reached.
     * @return list of keyset data with limit_exceeded flag
     */
    KeySetSeq get_keysets_by_tech_c(const std::string& handle, unsigned long limit);

    /**
     * Returns domain by a handle.
     * @param handle contains handle of the domain.
     * @return domain data.
     */
    Domain get_domain_by_handle(const std::string& handle);

    /**
     * Returns list of domains by a handle of registrant.
     * @param handle contains handle of the registrant.
     * @param limit flag whether requested limit was reached.
     * @return list of domain data with limit_exceeded flag
     */
    DomainSeq get_domains_by_registrant(const std::string& handle, unsigned long limit);

    /**
     * Returns list of domains by a handle of admin contact.
     * @param handle contains handle of the admin contact.
     * @param limit flag whether requested limit was reached.
     * @return list of domain data with limit_exceeded flag
     */
    DomainSeq get_domains_by_admin_contact(const std::string& handle,
                                           unsigned long limit);

    /**
     * Returns list of domains by a handle of nsset.
     * @param handle contains handle of the nsset.
     * @param limit flag whether requested limit was reached.
     * @return list of domain data with limit_exceeded flag
     */
    DomainSeq get_domains_by_nsset(const std::string& handle, unsigned long limit);

    /**
     * Returns list of domains by a handle of keyset.
     * @param handle contains handle of the keyset.
     * @param limit flag whether requested limit was reached.
     * @return list of domain data with limit_exceeded flag.
     */
    DomainSeq get_domains_by_keyset(const std::string& handle, unsigned long limit);

    /**
     * Returns list of domain status descriptions by language.
     * @param limit flag whether requested limit was reached.
     * @return list of domain status descriptions.
     */
    std::vector<ObjectStatusDesc> get_domain_status_descriptions(const std::string& lang);

    /**
     * Returns list of contact status descriptions by language.
     * @param limit flag whether requested limit was reached.
     * @return list of contact status descriptions.
     */
    std::vector<ObjectStatusDesc> get_contact_status_descriptions(const std::string& lang);

    /**
     * Returns list of nsset status descriptions by language.
     * @param limit flag whether requested limit was reached.
     * @return list of nsset status descriptions.
     */
    std::vector<ObjectStatusDesc> get_nsset_status_descriptions(const std::string& lang);

    /**
     * Returns list of keyset status descriptions by language.
     * @param limit flag whether requested limit was reached.
     * @return list of keyset status descriptions.
     */
    std::vector<ObjectStatusDesc> get_keyset_status_descriptions(const std::string& lang);

};//Server_impl

}//WhoisImpl
}//Registry

#endif /* _WHOIS_H_ */
