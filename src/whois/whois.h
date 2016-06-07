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

#ifndef _WHOIS_H_3513138416434634
#define _WHOIS_H_3513138416434634

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

    PlaceAddress () {}

    PlaceAddress (
        const std::string& _street1, 
        const std::string& _street2, 
        const std::string& _street3, 
        const std::string& _city, 
        const std::string& _stateorprovince, 
        const std::string& _postal_code, 
        const std::string& _country_code
    ) : 
        street1(_street1),
        street2(_street2),
        street3(_street3),
        city(_city),
        stateorprovince(_stateorprovince),
        postal_code(_postal_code),
        country_code(_country_code)
    {}

    PlaceAddress (const PlaceAddress& other) 
    : street1(other.street1),
      street2(other.street2),
      street3(other.street3),
      city(other.city),
      stateorprovince(other.stateorprovince),
      postal_code(other.postal_code),
      country_code(other.country_code)
    {}
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

    Registrar() {}

    Registrar(
        const std::string& _handle, 
        const std::string& _name, 
        const std::string& _organization, 
        const std::string& _url, 
        const std::string& _phone, 
        const std::string& _fax, 
        const PlaceAddress& _address
    ) : 
      id(0),
      handle(_handle), 
      name(_name), 
      organization(_organization), 
      url(_url), 
      phone(_phone), 
      fax(_fax), 
      address(_address)
    {}
};

struct ContactIdentification
{
    std::string identification_type; /**< type of the document which identifies the contact */
    std::string identification_data; /**< actual information of the identification */

    ContactIdentification() {}

    ContactIdentification(
        const std::string& _identification_type,
        const std::string& _identification_data
    ) :
        identification_type(_identification_type),
        identification_data(_identification_data)
    {}
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
    std::string creating_registrar; 
    std::string sponsoring_registrar; 
    boost::posix_time::ptime created; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    std::vector<std::string> statuses; 

    Contact() {} 

    Contact(
        const std::string& _handle, 
        const std::string& _organization, 
        const std::string& _name, 
        const PlaceAddress& _address, 
        const std::string& _phone, 
        const std::string& _fax, 
        const std::string& _email, 
        const std::string& _notify_email, 
        const ContactIdentification& _identification, 
        const std::string& _vat_number, 
        const std::string& _creating_registrar, 
        const std::string& _sponsoring_creating_registrar, 
        const boost::posix_time::ptime& _created, 
        const Nullable<boost::posix_time::ptime>& _changed, 
        const Nullable<boost::posix_time::ptime>& _last_transfer, 
        const std::vector<std::string>& _statuses, 
        bool _disclose_organization = false, 
        bool _disclose_name = false, 
        bool _disclose_address = false, 
        bool _disclose_phone = false, 
        bool _disclose_fax = false, 
        bool _disclose_email = false, 
        bool _disclose_notify_email = false, 
        bool _disclose_identification = false, 
        bool _disclose_vat_number = false
    ) : 
        handle(_handle), 
        organization(_organization), 
        name(_name), 
        address(_address), 
        phone(_phone), 
        fax(_fax), 
        email(_email), 
        notify_email(_notify_email), 
        identification(_identification), 
        vat_number(_vat_number), 
        creating_registrar(_creating_registrar), 
        sponsoring_creating_registrar(_sponsoring_creating_registrar), 
        created(_created), 
        changed(_changed), 
        last_transfer(_last_transfer), 
        statuses(_statuses), 
        disclose_organization(_disclose_organization), 
        disclose_name(_disclose_name), 
        disclose_address(_disclose_address), 
        disclose_phone(_disclose_phone), 
        disclose_fax(_disclose_fax), 
        disclose_email(_disclose_email), 
        disclose_notify_email(_disclose_notify_email), 
        disclose_identification(_disclose_identification), 
        disclose_vat_number(_disclose_vat_number)
    {}
};

struct NameServer
{
    std::string fqdn; 
    std::vector<boost::asio::ip::address> ip_addresses; 

    NameServer() {}

    NameServer(
        const std::string& _fqdn, 
        const std::vector<boost::asio::ip::address>& _ip_addresses 
    ) :
        fqdn(_fqdn),
        ip_addresses(_ip_addresses)
    {}
};

struct NSSet
{
    std::string handle; 
    std::vector<NameServer> nservers; 
    std::vector<std::string> tech_contacts; 
    std::string creating_registrar; 
    boost::posix_time::ptime created; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    std::vector<std::string> statuses; 

    NSSet() {}

    NSSet(
        const std::string& _handle, 
        const std::vector<NameServer>& _nservers, 
        const std::vector<std::string>& _tech_contacts, 
        const std::string& _creating_registrar, 
        const boost::posix_time::ptime& _created, 
        const Nullable<boost::posix_time::ptime>& _changed, 
        const Nullable<boost::posix_time::ptime>& _last_transfer, 
        const std::vector<std::string>& _statuses
    ) :
        handle(_handle), 
        nservers(_nservers), 
        tech_contacts(_tech_contacts), 
        creating_registrar(_creating_registrar), 
        created(_created), 
        changed(_changed), 
        last_transfer(_last_transfer), 
        statuses(_statuses)
    {}
};

struct NSSetSeq
{
    std::vector<NSSet> content; 
    bool limit_exceeded; 

    NSSetSeq()
    : limit_exceeded(false)
    {}

    NSSetSeq(const std::vector<NSSet>& _content,
            bool _limit_exceeded = false)
    : limit_exceeded(_limit_exceeded),
      content(_content)
    {}
};

struct DNSKey
{
    short flags; 
    short protocol; 
    short alg; 
    std::string public_key; 

    DNSKey() {}

    DNSKey(
        const short& _flags, 
        const short& _protocol, 
        const short& _alg, 
        const std::string& _public_key 
    ) :
        flags(_flags), 
        protocol(_protocol), 
        alg(_alg), 
        public_key(_public_key)
    {}
};

struct KeySet
{
    std::string handle; 
    std::vector<DNSKey> dns_keys; 
    std::vector<std::string> tech_contacts; 
    std::string creating_registrar; 
    boost::posix_time::ptime created; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    std::vector<std::string> statuses; 

    KeySet() {}

    KeySet(
        const std::string& _handle, 
        const std::vector<DNSKey>& _dns_keys, 
        const std::vector<std::string>& _tech_contacts, 
        const std::string& _creating_registrar, 
        const boost::posix_time::ptime& _created, 
        const Nullable<boost::posix_time::ptime>& _changed, 
        const Nullable<boost::posix_time::ptime>& _last_transfer, 
        const std::vector<std::string>& _statuses 
    ) :
        handle(_handle), 
        dns_keys(_dns_keys), 
        tech_contacts(_tech_contacts), 
        creating_registrar(_creating_registrar), 
        created(_created), 
        changed(_changed), 
        last_transfer(_last_transfer), 
        statuses(_statuses)
    {}
};

struct KeySetSeq
{
    std::vector<KeySet> content; 
    bool limit_exceeded; 

    KeySetSeq()
    : limit_exceeded(false)
    {}

    KeySetSeq(
        const std::vector<KeySet>& _content; 
        bool _limit_exceeded = false; 
    )
    : content(_content),
      limit_exceeded(_limit_exceeded)
    {}
};

struct Domain
{
    std::string fqdn; 
    std::string registrant; 
    std::vector<std::string> admin_contacts; 
    std::string nsset; 
    std::string keyset; 
    std::string creating_registrar; 
    std::vector<std::string> statuses; 
    boost::posix_time::ptime registered; 
    Nullable<boost::posix_time::ptime> changed; 
    Nullable<boost::posix_time::ptime> last_transfer; 
    boost::gregorian::date expire; 
    Nullable<boost::gregorian::date> validated_to; 

    Domain() {}

    Domain(
        const std::string& _fqdn, 
        const std::string& _registrant, 
        const std::vector<std::string>& _admin_contacts, 
        const std::string& _nsset, 
        const std::string& _keyset, 
        const std::string& _creating_registrar, 
        const std::vector<std::string>& _statuses, 
        const boost::posix_time::ptime& _registered, 
        const Nullable<boost::posix_time::ptime>& _changed, 
        const Nullable<boost::posix_time::ptime>& _last_transfer, 
        const boost::gregorian::date& _expire, 
        const Nullable<boost::gregorian::date>& _validated_to
    ) :
        fqdn(_fqdn), 
        registrant(_registrant), 
        admin_contacts(_admin_contacts), 
        nsset(_nsset), 
        keyset(_keyset), 
        creating_registrar(_creating_registrar), 
        statuses(_statuses), 
        registered(_registered), 
        changed(_changed), 
        last_transfer(_last_transfer), 
        expire(_expire), 
        validated_to(_validated_to)
    {}
};

struct DomainSeq
{
    std::vector<Domain> content; 
    bool limit_exceeded; 

    DomainSeq()
    : limit_exceeded(false)
    {}

    DomainSeq(
        const std::vector<Domain>& _content, 
        bool _limit_exceeded = false
    )
    : content(_content),
      limit_exceeded(_limit_exceeded)
    {}
};

struct RegistrarGroup
{
    std::string name; 
    std::vector<std::string> members; 

    RegistrarGroup() {}

    RegistrarGroup(
        const std::string& _name, 
        const std::vector<std::string>& _members 
    ) :
        name(_name), 
        members(_members)
    {}
};

struct RegistrarCertification
{
    std::string registrar; 
    short score; 
    unsigned long long evaluation_file_id; 

    RegistrarCertification() {}

    RegistrarCertification(
        const std::string& _registrar, 
        short _score, 
        unsigned long long _evaluation_file_id 
    ) :
        registrar(_registrar), 
        score(_score), 
        evaluation_file_id(_evaluation_file_id) 
    {}
};                               

struct ObjectStatusDesc
{
    std::string handle; 
    std::string name; 

    ObjectStatusDesc() {}

    ObjectStatusDesc(
        const std::string& _handle, 
        const std::string& _name 
    ) :
        handle(_handle), 
        name(_name) 
    {}
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

/**
 * The main class implementing the service.
 */
class Server_impl
{
public:
    
    virtual ~Server_impl() {}

    Registrar get_registrar_by_handle(const std::string& handle);

    std::vector<Registrar> get_registrars(); /** Returns the vector of non-system registrars.  */

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

#endif /* _WHOIS_H_3513138416434634 */
