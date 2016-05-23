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
 *  header of whois implementation
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

struct PlaceAddress
{
    std::string street1; 
    std::string street2; 
    std::string street3; 
    std::string city; 
    std::string stateorprovince; 
    std::string postal_code; 
    std::string country_code; 
};

struct Registrar
{
    unsigned long long id; /**< database id of the registrar */
    std::string handle; 
    std::string name; 
    std::string organization; 
    std::string url; 
    std::string phone; 
    std::string fax; 
    PlaceAddress address; 

    Registrar()
    : id(0)
    {}
};

struct ContactIdentification
{
    std::string identification_type; /**< type of the document which identifies the contact */
    std::string identification_data; /**< actual information of the identification */
};

struct Contact
{
    std::string handle; 
    std::string   organization; 
    bool disclose_organization; 
    std::string   name; 
    bool disclose_name; 
    PlaceAddress  address; 
    bool disclose_address; 
    std::string   phone; 
    bool disclose_phone; 
    std::string   fax; 
    bool disclose_fax; 
    std::string   email; 
    bool disclose_email; 
    std::string   notify_email; 
    bool disclose_notify_email; 
    ContactIdentification identification; 
    bool         disclose_identification; 
    std::string   vat_number; 
    bool disclose_vat_number; 
    std::string creating_registrar_handle; 
    std::string sponsoring_registrar_handle; 
    boost::posix_time::ptime created; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    std::vector<std::string> statuses; 

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

struct NameServer
{
    std::string fqdn; 
    std::vector<boost::asio::ip::address> ip_addresses; 
};

struct NSSet
{
    std::string handle; 
    std::vector<NameServer> nservers; 
    std::vector<std::string> tech_contact_handles; 
    std::string registrar_handle; 
    boost::posix_time::ptime created; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    std::vector<std::string> statuses; 
};

struct NSSetSeq
{
    std::vector<NSSet> content; 
    bool limit_exceeded; 

    NSSetSeq()
    : limit_exceeded(false)
    {}
};

struct DNSKey
{
    short flags; 
    short protocol; 
    short alg; 
    std::string public_key; 
};

struct KeySet
{
    std::string handle; 
    std::vector<DNSKey> dns_keys; 
    std::vector<std::string> tech_contact_handles; 
    std::string creating_registrar; 
    boost::posix_time::ptime created; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    std::vector<std::string> statuses; 
};

struct KeySetSeq
{
    std::vector<KeySet> content; 
    bool limit_exceeded; 

    KeySetSeq()
    : limit_exceeded(false)
    {}
};

struct Domain
{
    std::string fqdn; 
    std::string registrant_handle; 
    std::vector<std::string> admin_contact_handles; 
    std::string nsset_handle; 
    std::string keyset_handle; 
    std::string registrar_handle; 
    std::vector<std::string> statuses; 
    boost::posix_time::ptime registered; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    boost::gregorian::date expire; 
    Nullable<boost::gregorian::date> validated_to; 
};

struct DomainSeq
{
    std::vector<Domain> content; 
    bool limit_exceeded; 

    DomainSeq()
    : limit_exceeded(false)
    {}
};

struct RegistrarGroup
{
    std::string name; 
    std::vector<std::string> members; 
};

struct RegistrarCertification
{
    std::string registrar_handle; 
    short score; 
    unsigned long long evaluation_file_id; 
};

struct ObjectStatusDesc
{
    std::string handle; 
    std::string name; 
};

/**
 * Object requested by ID was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct ObjectNotExists
: virtual std::exception
{
    const char* what() const throw() {return "registry object with specified ID does not exist";}
};

/**
 * Object requested by handle was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct InvalidHandle
: virtual std::exception
{
    const char* what() const throw() {return "registry object with specified handle does not exist";}
};

/**
 * Internal server error.
 * Unexpected failure, requires maintenance.
 */
struct InternalServerError
: virtual std::exception
{
    const char* what() const throw() {return "internal server error";}
};

/**
 * Label of the object was incorrectly specified.
 */
struct InvalidLabel
: virtual std::exception
{
    const char* what() const throw() {return "the label is invalid";}
};

/**
 * Zone of the domain is not managed by the service.
 */
struct UnmanagedZone
: virtual std::exception
{
    const char* what() const throw() {return "this zone is not managed";}
};

/**
 * Domain name contains more labels than allowed for its type.
 */
struct TooManyLabels
: virtual std::exception
{
    const char* what() const throw() {return "domain has too many labels";}
};

/**
 * Database does not contain the localization of the description of the object.
 */
struct MissingLocalization
: virtual std::exception
{
    const char* what() const throw() {return "the localization is missing";}
};

/*
typedef std::vector< std::pair<std::string, std::string> > str_str_vector;
typedef std::vector<Fred::ObjectStateData> std::vector<Fred::ObjectStateData>;
typedef std::vector<Fred::InfoNssetOutput> std::vector<Fred::InfoNssetOutput>;
typedef std::vector<Fred::InfoDomainOutput> std::vector<Fred::InfoDomainOutput>;
*/

/**
 * The main class implementing the service.
 */
class Server_impl
{
private:
    std::vector<ObjectStatusDesc> get_object_status_descriptions(
            const std::string& lang,
            const std::string& type);

    DomainSeq get_domains_by_(
            Fred::OperationContextCreator& ctx,
            unsigned long limit,
            const std::vector<Fred::InfoDomainOutput>& domain_info);

    NSSetSeq get_nssets_by_(
            Fred::OperationContextCreator& ctx,
            const std::vector<Fred::InfoNssetOutput>& nss_info,
            const std::string& handle,
            unsigned long limit);

    WhoisImpl::NSSet make_nsset_from_info_data(
            const Fred::InfoNssetData& ind,
            Fred::OperationContextCreator& ctx);

    WhoisImpl::Domain make_domain_from_info_data(
            const Fred::InfoDomainData& idd,
            Fred::OperationContextCreator& ctx);
public:
    
    static const std::string output_timezone; 

    virtual ~Server_impl() {};

    Registrar get_registrar_by_handle(const std::string& handle);

    std::vector<Registrar> get_registrars(); /** Returns the list of non-system registrars.  */

    std::vector<RegistrarGroup> get_registrar_groups();

    std::vector<RegistrarCertification> get_registrar_certification_list();

    std::vector<std::string> get_managed_zone_list();

    Contact get_contact_by_handle(const std::string& handle);

    NSSet get_nsset_by_handle(const std::string& handle);

    NSSetSeq get_nssets_by_ns(const std::string& handle, unsigned long limit);

    NSSetSeq get_nssets_by_tech_c(const std::string& handle, unsigned long limit);

    NameServer get_nameserver_by_fqdn(const std::string& handle);

    KeySet get_keyset_by_handle(const std::string& handle);

    KeySetSeq get_keysets_by_tech_c(const std::string& handle, unsigned long limit);

    Domain get_domain_by_handle(const std::string& handle);

    DomainSeq get_domains_by_registrant(const std::string& handle, unsigned long limit);

    DomainSeq get_domains_by_admin_contact(const std::string& handle, unsigned long limit);

    DomainSeq get_domains_by_nsset(const std::string& handle, unsigned long limit);

    DomainSeq get_domains_by_keyset(const std::string& handle, unsigned long limit);

    std::vector<ObjectStatusDesc> get_domain_status_descriptions(const std::string& lang);

    std::vector<ObjectStatusDesc> get_contact_status_descriptions(const std::string& lang);

    std::vector<ObjectStatusDesc> get_nsset_status_descriptions(const std::string& lang);

    std::vector<ObjectStatusDesc> get_keyset_status_descriptions(const std::string& lang);

};//Server_impl

}//WhoisImpl
}//Registry

#endif /* _WHOIS_H_ */
