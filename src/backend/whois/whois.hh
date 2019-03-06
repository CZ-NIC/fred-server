/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
 *  header of whois implementation
 */

#ifndef WHOIS_HH_C352E6E1DED044AAAE6EC8B2EF5DCA30
#define WHOIS_HH_C352E6E1DED044AAAE6EC8B2EF5DCA30

#include "libfred/object_state/get_object_states.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/domain/info_domain_output.hh"
#include "libfred/registrable_object/nsset/info_nsset_output.hh"
#include "util/db/nullable.hh"

#include <boost/asio/ip/address.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Whois {

struct PlaceAddress
{
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string stateorprovince;
    std::string postal_code;
    std::string country_code;

    PlaceAddress()
    {
    }

    PlaceAddress(
            const std::string& _street1,
            const std::string& _street2,
            const std::string& _street3,
            const std::string& _city,
            const std::string& _stateorprovince,
            const std::string& _postal_code,
            const std::string& _country_code)
        : street1(_street1),
          street2(_street2),
          street3(_street3),
          city(_city),
          stateorprovince(_stateorprovince),
          postal_code(_postal_code),
          country_code(_country_code)
    {
    }

    PlaceAddress(const PlaceAddress& other)
        : street1(other.street1),
          street2(other.street2),
          street3(other.street3),
          city(other.city),
          stateorprovince(other.stateorprovince),
          postal_code(other.postal_code),
          country_code(other.country_code)
    {
    }
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
    {
    }

    Registrar(
            const std::string& _handle,
            const std::string& _name,
            const std::string& _organization,
            const std::string& _url,
            const std::string& _phone,
            const std::string& _fax,
            const PlaceAddress& _address)
        : id(0),
          handle(_handle),
          name(_name),
          organization(_organization),
          url(_url),
          phone(_phone),
          fax(_fax),
          address(_address)
    {
    }
};

struct ContactIdentification
{
    std::string identification_type; /**< type of the document which identifies the contact */
    std::string identification_data; /**< actual information of the identification */

    ContactIdentification()
    {
    }

    ContactIdentification(
            const std::string& _identification_type,
            const std::string& _identification_data)
        : identification_type(_identification_type),
          identification_data(_identification_data)
    {
    }
};

/**
 * Disclose suffix means whether corresponding info is allowed to be seen
 */
struct Contact
{
    std::string handle;
    std::string organization;
    std::string name;
    PlaceAddress address;
    std::string phone;
    std::string fax;
    std::string email;
    std::string notify_email;
    ContactIdentification identification;
    std::string vat_number;
    std::string creating_registrar;
    std::string sponsoring_registrar;
    boost::posix_time::ptime created;
    Nullable<boost::posix_time::ptime> changed;
    Nullable<boost::posix_time::ptime> last_transfer;
    std::vector<std::string> statuses;
    bool disclose_organization;
    bool disclose_name;
    bool disclose_address;
    bool disclose_phone;
    bool disclose_fax;
    bool disclose_email;
    bool disclose_notify_email;
    bool disclose_identification;
    bool disclose_vat_number;

    Contact()
    {
    }

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
            const std::string& _sponsoring_registrar,
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
            bool _disclose_vat_number = false)
        : handle(_handle),
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
          sponsoring_registrar(_sponsoring_registrar),
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
    {
    }
};

struct NameServer
{
    std::string fqdn;
    std::vector<boost::asio::ip::address> ip_addresses;

    NameServer()
    {
    }

    NameServer(
            const std::string& _fqdn,
            const std::vector<boost::asio::ip::address>& _ip_addresses)
        : fqdn(_fqdn),
          ip_addresses(_ip_addresses)
    {
    }
};

struct NSSet
{
    std::string handle;
    std::vector<NameServer> nservers;
    std::vector<std::string> tech_contacts;
    std::string sponsoring_registrar;
    boost::posix_time::ptime created;
    Nullable<boost::posix_time::ptime> changed;
    Nullable<boost::posix_time::ptime> last_transfer;
    std::vector<std::string> statuses;

    NSSet()
    {
    }

    NSSet(
            const std::string& _handle,
            const std::vector<NameServer>& _nservers,
            const std::vector<std::string>& _tech_contacts,
            const std::string& _sponsoring_registrar,
            const boost::posix_time::ptime& _created,
            const Nullable<boost::posix_time::ptime>& _changed,
            const Nullable<boost::posix_time::ptime>& _last_transfer,
            const std::vector<std::string>& _statuses)
        : handle(_handle),
          nservers(_nservers),
          tech_contacts(_tech_contacts),
          sponsoring_registrar(_sponsoring_registrar),
          created(_created),
          changed(_changed),
          last_transfer(_last_transfer),
          statuses(_statuses)
    {
    }
};

struct NSSetSeq
{
    std::vector<NSSet> content;
    bool limit_exceeded;

    NSSetSeq()
        : limit_exceeded(false)
    {
    }

    NSSetSeq(
            const std::vector<NSSet>& _content,
            bool _limit_exceeded = false)
        : content(_content),
          limit_exceeded(_limit_exceeded)
    {
    }
};

/**
 * DNSKEY Resource Record data (rfc4034)
 */
struct DNSKey
{
    short flags;
    short protocol;
    short alg;
    std::string public_key;

    DNSKey()
    {
    }

    DNSKey(
            short _flags,
            short _protocol,
            short _alg,
            const std::string& _public_key)
        : flags(_flags),
          protocol(_protocol),
          alg(_alg),
          public_key(_public_key)
    {
    }
};

struct KeySet
{
    std::string handle;
    std::vector<DNSKey> dns_keys;
    std::vector<std::string> tech_contacts;
    std::string sponsoring_registrar;
    boost::posix_time::ptime created;
    Nullable<boost::posix_time::ptime> changed;
    Nullable<boost::posix_time::ptime> last_transfer;
    std::vector<std::string> statuses;

    KeySet()
    {
    }

    KeySet(
            const std::string& _handle,
            const std::vector<DNSKey>& _dns_keys,
            const std::vector<std::string>& _tech_contacts,
            const std::string& _sponsoring_registrar,
            const boost::posix_time::ptime& _created,
            const Nullable<boost::posix_time::ptime>& _changed,
            const Nullable<boost::posix_time::ptime>& _last_transfer,
            const std::vector<std::string>& _statuses)
        : handle(_handle),
          dns_keys(_dns_keys),
          tech_contacts(_tech_contacts),
          sponsoring_registrar(_sponsoring_registrar),
          created(_created),
          changed(_changed),
          last_transfer(_last_transfer),
          statuses(_statuses)
    {
    }
};

struct KeySetSeq
{
    std::vector<KeySet> content;
    bool limit_exceeded;

    KeySetSeq()
        : limit_exceeded(false)
    {
    }

    KeySetSeq(
            const std::vector<KeySet>& _content,
            bool _limit_exceeded = false)
        : content(_content),
          limit_exceeded(_limit_exceeded)
    {
    }
};

struct Domain
{
    std::string fqdn;
    std::string registrant;
    std::vector<std::string> admin_contacts;
    std::string nsset;
    std::string keyset;
    std::string sponsoring_registrar;
    std::vector<std::string> statuses;
    boost::posix_time::ptime registered;
    Nullable<boost::posix_time::ptime> changed;
    Nullable<boost::posix_time::ptime> last_transfer;
    boost::gregorian::date expire;
    Nullable<boost::gregorian::date> validated_to;
    boost::posix_time::ptime expire_time_estimate;
    Nullable<boost::posix_time::ptime> expire_time_actual;
    Nullable<boost::posix_time::ptime> validated_to_time_estimate;
    Nullable<boost::posix_time::ptime> validated_to_time_actual;

    Domain()
    {
    }

    Domain(
            const std::string& _fqdn,
            const std::string& _registrant,
            const std::vector<std::string>& _admin_contacts,
            const std::string& _nsset,
            const std::string& _keyset,
            const std::string& _sponsoring_registrar,
            const std::vector<std::string>& _statuses,
            const boost::posix_time::ptime& _registered,
            const Nullable<boost::posix_time::ptime>& _changed,
            const Nullable<boost::posix_time::ptime>& _last_transfer,
            const boost::gregorian::date& _expire,
            const Nullable<boost::gregorian::date>& _validated_to,
            const boost::posix_time::ptime _expire_time_estimate,
            const Nullable<boost::posix_time::ptime> _expire_time_actual,
            const Nullable<boost::posix_time::ptime> _validated_to_time_estimate,
            const Nullable<boost::posix_time::ptime> _validated_to_time_actual)
        : fqdn(_fqdn),
          registrant(_registrant),
          admin_contacts(_admin_contacts),
          nsset(_nsset),
          keyset(_keyset),
          sponsoring_registrar(_sponsoring_registrar),
          statuses(_statuses),
          registered(_registered),
          changed(_changed),
          last_transfer(_last_transfer),
          expire(_expire),
          validated_to(_validated_to),
          expire_time_estimate(_expire_time_estimate),
          expire_time_actual(_expire_time_actual),
          validated_to_time_estimate(_validated_to_time_estimate),
          validated_to_time_actual(_validated_to_time_actual)
    {
    }
};

struct DomainSeq
{
    std::vector<Domain> content;
    bool limit_exceeded;

    DomainSeq()
        : limit_exceeded(false)
    {
    }

    DomainSeq(
            const std::vector<Domain>& _content,
            bool _limit_exceeded = false)
        : content(_content),
          limit_exceeded(_limit_exceeded)
    {
    }
};

struct RegistrarGroup
{
    std::string name;
    std::vector<std::string> members;

    RegistrarGroup()
    {
    }

    RegistrarGroup(
            const std::string& _name,
            const std::vector<std::string>& _members)
        : name(_name),
          members(_members)
    {
    }
};

struct RegistrarCertification
{
    std::string registrar;
    short score;
    unsigned long long evaluation_file_id;

    RegistrarCertification()
    {
    }

    RegistrarCertification(
            const std::string& _registrar,
            short _score,
            unsigned long long _evaluation_file_id)
        : registrar(_registrar),
          score(_score),
          evaluation_file_id(_evaluation_file_id)
    {
    }
};

struct ObjectStatusDesc
{
    std::string handle;
    std::string name;

    ObjectStatusDesc()
    {
    }

    ObjectStatusDesc(
            const std::string& _handle,
            const std::string& _name)
        : handle(_handle),
          name(_name)
    {
    }
};

struct Exception : std::exception
{
};
struct FatalException : Exception
{
};

/**
 * Object requested by ID was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct ObjectNotExists : Exception
{
    const char* what() const noexcept
    {
        return "registry object with specified ID does not exist";
    }
};

/**
 * Object requested by ID was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct ObjectDeleteCandidate : Exception
{
    const char* what() const noexcept
    {
        return "registry object with specified ID is a delete candidate";
    }
};

/**
 * Object requested by handle was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct InvalidHandle : Exception
{
    const char* what() const noexcept
    {
        return "registry object with specified handle does not exist";
    }
};

/**
 * Internal server error.
 * Unexpected failure, requires maintenance.
 */
struct InternalServerError : FatalException
{
    const char* what() const noexcept
    {
        return "internal server error";
    }
};

/**
 * Label of the object was incorrectly specified.
 */
struct InvalidLabel : Exception
{
    const char* what() const noexcept
    {
        return "the label is invalid";
    }
};

/**
 * Zone of the domain is not managed by the service.
 */
struct UnmanagedZone : Exception
{
    const char* what() const noexcept
    {
        return "this zone is not managed";
    }
};

/**
 * Domain name contains more labels than allowed for its type.
 */
struct TooManyLabels : Exception
{
    const char* what() const noexcept
    {
        return "domain has too many labels";
    }
};

/**
 * Database does not contain the localization of the description of the object.
 */
struct MissingLocalization : Exception
{
    const char* what() const noexcept
    {
        return "the localization is missing";
    }
};

/**
 * The main class implementing the service.
 */
class Server_impl
{
public:
    Server_impl(const std::string& _server_name);
    virtual ~Server_impl()
    {
    }

    const std::string& get_server_name() const;
    /**
     * Returns registrar (system/non-system) by a handle.
     * @param handle contains handle of the registrar.
     * @return registrar data.
     */
    Registrar get_registrar_by_handle(const std::string& handle);

    /**
     * Returns the vector of non-system registrars.
     * @return registrar vector.
     */
    std::vector<Registrar> get_registrars();

    /**
     * Returns the vector of registrar groups.
     * @return registrar group vector.
     */
    std::vector<RegistrarGroup> get_registrar_groups();

    /**
     * Returns the vector of registrar certifications.
     * @return registrar certification vector.
     */
    std::vector<RegistrarCertification> get_registrar_certification_list();

    /**
     * Returns the vector of managed zones.
     * @return managed zone vector.
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
     * Returns vector of nssets by the name server handle.
     * @param handle contains handle of the name server.
     * @param limit flag whether requested limit was reached.
     * @return vector of nsset data with limit_exceeded flag.
     */
    NSSetSeq get_nssets_by_ns(const std::string& handle, unsigned long limit);

    /**
     * Returns vector of nssets by a handle of technical contact.
     * @param handle contains handle of the technical contact.
     * @param limit flag whether requested limit was reached.
     * @return vector of nsset data with limit_exceeded flag.
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
     * Returns vector of keysets by the handle of technical contact.
     * @param handle contains handle of the technical contact.
     * @param limit flag whether requested limit was reached.
     * @return vector of keyset data with limit_exceeded flag.
     */
    KeySetSeq get_keysets_by_tech_c(const std::string& handle, unsigned long limit);

    /**
     * Returns domain by a handle.
     * @param handle contains handle of the domain.
     * @return domain data.
     */
    Domain get_domain_by_handle(const std::string& handle);

    /**
     * Returns vector of domains by a handle of registrant.
     * @param handle contains handle of the registrant.
     * @param limit flag whether requested limit was reached.
     * @return vector of domain data with limit_exceeded flag
     */
    DomainSeq get_domains_by_registrant(const std::string& handle, unsigned long limit);

    /**
     * Returns vector of domains by a handle of admin contact.
     * @param handle contains handle of the admin contact.
     * @param limit flag whether requested limit was reached.
     * @return vector of domain data with limit_exceeded flag
     */
    DomainSeq get_domains_by_admin_contact(const std::string& handle, unsigned long limit);

    /**
     * Returns vector of domains by a handle of nsset.
     * @param handle contains handle of the nsset.
     * @param limit flag whether requested limit was reached.
     * @return vector of domain data with limit_exceeded flag
     */
    DomainSeq get_domains_by_nsset(const std::string& handle, unsigned long limit);

    /**
     * Returns vector of domains by a handle of keyset.
     * @param handle contains handle of the keyset.
     * @param limit flag whether requested limit was reached.
     * @return vector of domain data with limit_exceeded flag.
     */
    DomainSeq get_domains_by_keyset(const std::string& handle, unsigned long limit);

    /**
     * Returns vector of domain status descriptions by language.
     * @param lang is the language of the descriptions.
     * @return vector of domain status descriptions.
     */
    std::vector<ObjectStatusDesc> get_domain_status_descriptions(const std::string& lang);

    /**
     * Returns vector of contact status descriptions by language.
     * @param lang is the language of the descriptions.
     * @return vector of contact status descriptions.
     */
    std::vector<ObjectStatusDesc> get_contact_status_descriptions(const std::string& lang);

    /**
     * Returns vector of nsset status descriptions by language.
     * @param lang is the language of the descriptions.
     * @return vector of nsset status descriptions.
     */
    std::vector<ObjectStatusDesc> get_nsset_status_descriptions(const std::string& lang);

    /**
     * Returns vector of keyset status descriptions by language.
     * @param lang is the language of the descriptions.
     * @return vector of keyset status descriptions.
     */
    std::vector<ObjectStatusDesc> get_keyset_status_descriptions(const std::string& lang);

private:
    const std::string server_name;
}; //Server_impl

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
