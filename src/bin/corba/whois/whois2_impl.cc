#include "src/bin/corba/whois/whois2_impl.hh"
#include "src/bin/corba/common_wrappers.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/bin/corba/util/corba_conversions_datetime.hh"
#include "src/bin/corba/util/corba_conversions_nullable_types.hh"

#include <omniORB4/CORBA.h>

#include <vector>

namespace Registry
{
namespace Whois
{

PlaceAddress wrap_address(const Registry::WhoisImpl::PlaceAddress& address)
{
    PlaceAddress result;
    result.street1    = LibFred::Corba::wrap_string_to_corba_string(address.street1);
    result.street2    = LibFred::Corba::wrap_string_to_corba_string(address.street2);
    result.street3    = LibFred::Corba::wrap_string_to_corba_string(address.street3);
    result.city       = LibFred::Corba::wrap_string_to_corba_string(address.city);
    result.postalcode = LibFred::Corba::wrap_string_to_corba_string(address.postal_code);
    result.stateorprovince = LibFred::Corba::wrap_string_to_corba_string(address.stateorprovince);
    result.country_code    = LibFred::Corba::wrap_string_to_corba_string(address.country_code);
    return result;
}

Registrar wrap_registrar(const Registry::WhoisImpl::Registrar& registrar)
{
    Registrar result;
    result.handle  = LibFred::Corba::wrap_string_to_corba_string(registrar.handle);
    result.name    = LibFred::Corba::wrap_string_to_corba_string(registrar.name);
    result.organization = LibFred::Corba::wrap_string_to_corba_string(registrar.organization);
    result.url     = LibFred::Corba::wrap_string_to_corba_string(registrar.url);
    result.phone   = LibFred::Corba::wrap_string_to_corba_string(registrar.phone);
    result.fax     = LibFred::Corba::wrap_string_to_corba_string(registrar.fax);
    result.address = wrap_address(registrar.address);
    return result;
}

Registrar* Server_impl::get_registrar_by_handle(const char* handle)
{
    try
    {
        return new Registrar(
                wrap_registrar(
                    pimpl_->get_registrar_by_handle(std::string(handle))));
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    // default exception handling
    throw INTERNAL_SERVER_ERROR();
}

RegistrarSeq* Server_impl::get_registrars()
{
    try
    {
        RegistrarSeq_var result = new RegistrarSeq;
        const std::vector<Registry::WhoisImpl::Registrar>& registrars = pimpl_->get_registrars();
        result->length(registrars.size());
        for (CORBA::ULong i = 0; i < registrars.size(); ++i)
        {
            result[i] = wrap_registrar(registrars[i]);
        }
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

RegistrarGroup wrap_registrar_group(const Registry::WhoisImpl::RegistrarGroup& group)
{
    RegistrarGroup result;
    result.name = LibFred::Corba::wrap_string_to_corba_string(group.name);
    result.members.length(group.members.size());
    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator cit = group.members.begin();
            cit != group.members.end(); ++cit, ++i)
    {
        result.members[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }
    return result;
}

RegistrarGroupList* Server_impl::get_registrar_groups()
{
    try
    {
        RegistrarGroupList_var result = new RegistrarGroupList;
        const std::vector<Registry::WhoisImpl::RegistrarGroup>& groups = pimpl_->get_registrar_groups();
        result->length(groups.size());
        for (CORBA::ULong i = 0; i < groups.size(); ++i)
        {
            result[i] = wrap_registrar_group(groups[i]);
        }
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

RegistrarCertification wrap_registrar_certification(const Registry::WhoisImpl::RegistrarCertification& cert)
{
    RegistrarCertification result;
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(cert.registrar);
    result.score = cert.score;
    result.evaluation_file_id = cert.evaluation_file_id;
    return result;
}

RegistrarCertificationList* Server_impl::get_registrar_certification_list()
{
    try
    {
        RegistrarCertificationList_var result = new RegistrarCertificationList;
        const std::vector<Registry::WhoisImpl::RegistrarCertification>& certifications = pimpl_->get_registrar_certification_list();
        result->length(certifications.size());
        for (CORBA::ULong i = 0; i < certifications.size(); ++i)
        {
            result[i] = wrap_registrar_certification(certifications[i]);
        }
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

ZoneFqdnList* Server_impl::get_managed_zone_list()
{
    try
    {
        ZoneFqdnList_var result = new ZoneFqdnList;
        const std::vector<std::string>& zones = pimpl_->get_managed_zone_list();
        result->length(zones.size());
        CORBA::ULong i = 0;
        for (std::vector<std::string>::const_iterator cit = zones.begin();
                cit != zones.end(); ++cit, ++i)
        {
            result[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
        }
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DisclosableString wrap_disclosable_string(const std::string& str, bool disclose)
{
    DisclosableString temp;
    temp.value = LibFred::Corba::wrap_string_to_corba_string(str);
    temp.disclose = disclose;
    return temp;
}

DisclosablePlaceAddress wrap_disclosable_address(const Registry::WhoisImpl::PlaceAddress& addr, bool disclose)
{
    DisclosablePlaceAddress temp;
    temp.value.street1 = LibFred::Corba::wrap_string_to_corba_string(addr.street1);
    temp.value.street2 = LibFred::Corba::wrap_string_to_corba_string(addr.street2);
    temp.value.street3 = LibFred::Corba::wrap_string_to_corba_string(addr.street3);
    temp.value.city    = LibFred::Corba::wrap_string_to_corba_string(addr.city);
    temp.value.stateorprovince = LibFred::Corba::wrap_string_to_corba_string(addr.stateorprovince);
    temp.value.postalcode      = LibFred::Corba::wrap_string_to_corba_string(addr.postal_code);
    temp.value.country_code    = LibFred::Corba::wrap_string_to_corba_string(addr.country_code);
    temp.disclose = disclose;
    return temp;
}

Contact wrap_contact(const Registry::WhoisImpl::Contact& con)
{
    Contact result;
    result.handle       = LibFred::Corba::wrap_string_to_corba_string(con.handle);
    result.organization = wrap_disclosable_string(con.organization, con.disclose_organization);
    result.name         = wrap_disclosable_string(con.name, con.disclose_name);
    result.address      = wrap_disclosable_address(con.address, con.disclose_address);
    result.phone        = wrap_disclosable_string(con.phone, con.disclose_phone);
    result.fax          = wrap_disclosable_string(con.fax, con.disclose_fax);;
    result.email        = wrap_disclosable_string(con.email, con.disclose_email);
    result.notify_email = wrap_disclosable_string(con.notify_email, con.disclose_notify_email);
    result.vat_number   = wrap_disclosable_string(con.vat_number, con.disclose_vat_number);

    result.identification.value.identification_type =
            LibFred::Corba::wrap_string_to_corba_string(con.identification.identification_type);
    result.identification.value.identification_data =
            LibFred::Corba::wrap_string_to_corba_string(con.identification.identification_data);
    result.identification.disclose = con.disclose_identification;

    result.creating_registrar_handle =
            LibFred::Corba::wrap_string_to_corba_string(con.creating_registrar);
    result.sponsoring_registrar_handle =
            LibFred::Corba::wrap_string_to_corba_string(con.sponsoring_registrar);
    result.created = LibFred::Corba::wrap_time(con.created);
    result.changed = LibFred::Corba::wrap_nullable_datetime(con.changed);
    result.last_transfer = LibFred::Corba::wrap_nullable_datetime(con.last_transfer);

    result.statuses.length(con.statuses.size());
    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator cit = con.statuses.begin();
            cit != con.statuses.end(); ++cit, ++i)
    {
        result.statuses[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }

    return result;
}

Contact* Server_impl::get_contact_by_handle(const char* handle)
{
    try
    {
        return new Contact(wrap_contact(pimpl_->get_contact_by_handle(handle)));
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

struct InvalidIPAddressException : public std::runtime_error
{
    InvalidIPAddressException() : std::runtime_error("invalid IP address") {}
};

IPAddress wrap_ipaddress(const boost::asio::ip::address& in)
{
    IPAddress result;
    result.address = LibFred::Corba::wrap_string(in.to_string());
    if (in.is_v4())
    {
        result.version = IPv4;
    }
    else if (in.is_v6())
    {
        result.version = IPv6;
    }
    else
    {
        throw InvalidIPAddressException();
    }
    return result;
}

NameServer wrap_nameserver(const Registry::WhoisImpl::NameServer& ns)
{
    NameServer result;
    result.fqdn = LibFred::Corba::wrap_string_to_corba_string(ns.fqdn);
    result.ip_addresses.length(ns.ip_addresses.size());
    for (CORBA::ULong i = 0; i < ns.ip_addresses.size(); ++i)
    {
        result.ip_addresses[i] = wrap_ipaddress(ns.ip_addresses[i]);
    }
    return result;
}

NSSet wrap_nsset(const Registry::WhoisImpl::NSSet& nsset)
{
    NSSet result;

    result.handle = LibFred::Corba::wrap_string_to_corba_string(nsset.handle);
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(nsset.sponsoring_registrar);
    result.created = LibFred::Corba::wrap_time(nsset.created);
    result.changed = LibFred::Corba::wrap_nullable_datetime(nsset.changed);
    result.last_transfer = LibFred::Corba::wrap_nullable_datetime(nsset.last_transfer);

    result.nservers.length(nsset.nservers.size());
    for (CORBA::ULong i = 0; i < nsset.nservers.size(); ++i)
    {
        result.nservers[i] = wrap_nameserver(nsset.nservers[i]);
    }
    result.tech_contact_handles.length(nsset.tech_contacts.size());
    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator cit = nsset.tech_contacts.begin();
            cit != nsset.tech_contacts.end(); ++cit, ++i)
    {
        result.tech_contact_handles[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }

    result.statuses.length(nsset.statuses.size());
    i = 0;
    for (std::vector<std::string>::const_iterator cit = nsset.statuses.begin();
            cit != nsset.statuses.end(); ++cit, ++i)
    {
        result.statuses[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }
    return result;
}

NSSet* Server_impl::get_nsset_by_handle(const char* handle)
{
    try
    {
        return new NSSet(wrap_nsset(pimpl_->get_nsset_by_handle(handle)));
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

NSSetSeq* Server_impl::get_nssets_by_ns(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        NSSetSeq_var result = new NSSetSeq;
        Registry::WhoisImpl::NSSetSeq nss_seq = pimpl_->get_nssets_by_ns(handle, limit);
        limit_exceeded = nss_seq.limit_exceeded;
        result->length(nss_seq.content.size());
        for (CORBA::ULong i = 0; i < nss_seq.content.size(); ++i)
        {
            result[i] = wrap_nsset(nss_seq.content[i]);
        }
        return result._retn();
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

NSSetSeq* Server_impl::get_nssets_by_tech_c(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        NSSetSeq_var result = new NSSetSeq;
        Registry::WhoisImpl::NSSetSeq nss_seq = pimpl_->get_nssets_by_tech_c(handle, limit);
        limit_exceeded = nss_seq.limit_exceeded;
        result->length(nss_seq.content.size());
        for (CORBA::ULong i = 0; i < nss_seq.content.size(); ++i)
        {
            result[i] = wrap_nsset(nss_seq.content[i]);
        }
        return result._retn();
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

NameServer* Server_impl::get_nameserver_by_fqdn(const char* handle)
{
    try
    {
        NameServer result;
        result.fqdn = LibFred::Corba::wrap_string_to_corba_string(pimpl_->get_nameserver_by_fqdn(handle).fqdn);
        /*
         * Because of grouping nameservers in NSSet we don't include
         * IP address in output (given nameserver can be in different
         * NSSets with different IP addresses)
         *
         * result.ip_addresses;
         */
        return new NameServer(result);
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DNSKey wrap_dnskey(const Registry::WhoisImpl::DNSKey& dnskey)
{
    DNSKey result;
    result.flags      = dnskey.flags;
    result.protocol   = dnskey.protocol;
    result.alg        = dnskey.alg;
    result.public_key = LibFred::Corba::wrap_string_to_corba_string(dnskey.public_key);
    return result;
}

KeySet wrap_keyset(const Registry::WhoisImpl::KeySet& keyset)
{
    KeySet result;

    result.handle  = LibFred::Corba::wrap_string_to_corba_string(keyset.handle);
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(keyset.sponsoring_registrar);
    result.created = LibFred::Corba::wrap_time(keyset.created);
    result.changed = LibFred::Corba::wrap_nullable_datetime(keyset.changed);
    result.last_transfer = LibFred::Corba::wrap_nullable_datetime(keyset.last_transfer);

    result.tech_contact_handles.length(keyset.tech_contacts.size());
    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator cit = keyset.tech_contacts.begin();
            cit != keyset.tech_contacts.end(); ++cit, ++i)
    {
        result.tech_contact_handles[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }

    result.statuses.length(keyset.statuses.size());
    i = 0;
    for (std::vector<std::string>::const_iterator cit = keyset.statuses.begin();
            cit != keyset.statuses.end(); ++cit, ++i)
    {
        result.statuses[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }

    result.dns_keys.length(keyset.dns_keys.size());
    for (CORBA::ULong i = 0; i < keyset.dns_keys.size(); ++i)
    {
        result.dns_keys[i] = wrap_dnskey(keyset.dns_keys[i]);
    }
    return result;
}

KeySet* Server_impl::get_keyset_by_handle(const char* handle)
{
    try
    {
        return new KeySet(wrap_keyset(pimpl_->get_keyset_by_handle(handle)));
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

KeySetSeq* Server_impl::get_keysets_by_tech_c(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        KeySetSeq_var result = new KeySetSeq;
        Registry::WhoisImpl::KeySetSeq ks_seq = pimpl_->get_keysets_by_tech_c(handle, limit);
        limit_exceeded = ks_seq.limit_exceeded;
        result->length(ks_seq.content.size());
        for (CORBA::ULong i = 0; i < ks_seq.content.size(); ++i)
        {
            result[i] = wrap_keyset(ks_seq.content[i]);
        }
        return result._retn();
    }
    catch (const Registry::WhoisImpl::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

Domain wrap_domain(const Registry::WhoisImpl::Domain& domain)
{
    Domain result;
    result.handle = LibFred::Corba::wrap_string_to_corba_string(domain.fqdn);
    result.registrant_handle = LibFred::Corba::wrap_string_to_corba_string(domain.registrant);
    if (domain.nsset.size() == 0)
    {
        result.nsset_handle = NULL;
    }
    else
    {
        result.nsset_handle = new NullableString(LibFred::Corba::wrap_string_to_corba_string(domain.nsset));
    }
    if (domain.keyset.size() == 0)
    {
        result.keyset_handle = NULL;
    }
    else
    {
        result.keyset_handle = new NullableString(LibFred::Corba::wrap_string_to_corba_string(domain.keyset));
    }
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(domain.sponsoring_registrar);
    result.registered = LibFred::Corba::wrap_time(domain.registered);
    result.changed = LibFred::Corba::wrap_nullable_datetime(domain.changed);
    result.last_transfer = LibFred::Corba::wrap_nullable_datetime(domain.last_transfer);
    result.expire = LibFred::Corba::wrap_date(domain.expire);
    result.expire_time_estimate = LibFred::Corba::wrap_time(domain.expire_time_estimate);
    result.expire_time_actual = LibFred::Corba::wrap_nullable_datetime(domain.expire_time_actual);
    if (domain.validated_to.isnull())
    {
        result.validated_to = NULL;
        result.validated_to_time_estimate = NULL;
        result.validated_to_time_actual = NULL;
    }
    else
    {
        result.validated_to = new Registry::NullableDate(LibFred::Corba::wrap_date(domain.validated_to.get_value()));
        result.validated_to_time_estimate = LibFred::Corba::wrap_nullable_datetime(domain.validated_to_time_estimate);
        result.validated_to_time_actual = LibFred::Corba::wrap_nullable_datetime(domain.validated_to_time_actual);
    }

    result.admin_contact_handles.length(domain.admin_contacts.size());
    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator cit = domain.admin_contacts.begin();
            cit != domain.admin_contacts.end(); ++cit, ++i)
    {
        result.admin_contact_handles[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }

    result.statuses.length(domain.statuses.size());
    i = 0;
    for (std::vector<std::string>::const_iterator cit = domain.statuses.begin();
            cit != domain.statuses.end(); ++cit, ++i)
    {
        result.statuses[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }

    return result;

}

Domain* Server_impl::get_domain_by_handle(const char* handle)
{
    try
    {
        return new Domain(wrap_domain(pimpl_->get_domain_by_handle(handle)));
    }
    catch (const Registry::WhoisImpl::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (const Registry::WhoisImpl::TooManyLabels& e)
    {
        throw Registry::Whois::TOO_MANY_LABELS();
    }
    catch (const Registry::WhoisImpl::UnmanagedZone& e)
    {
        throw Registry::Whois::UNMANAGED_ZONE();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

static DomainSeq* get_domains_by_(
    const Registry::WhoisImpl::DomainSeq& dom_seq,
    ::CORBA::Boolean& limit_exceeded)
{
    DomainSeq_var result = new DomainSeq;
    limit_exceeded = dom_seq.limit_exceeded;
    result->length(dom_seq.content.size());
    for (CORBA::ULong i = 0; i < dom_seq.content.size(); ++i)
    {
        result[i] = wrap_domain(dom_seq.content[i]);
    }
    return result._retn();
}

DomainSeq* Server_impl::get_domains_by_registrant(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_registrant(handle, limit), limit_exceeded);
    }
    catch (const Registry::WhoisImpl::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DomainSeq* Server_impl::get_domains_by_admin_contact(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_admin_contact(handle, limit), limit_exceeded);
    }
    catch (const Registry::WhoisImpl::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DomainSeq* Server_impl::get_domains_by_nsset(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_nsset(handle, limit), limit_exceeded);
    }
    catch (const Registry::WhoisImpl::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DomainSeq* Server_impl::get_domains_by_keyset(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_keyset(handle, limit), limit_exceeded);
    }
    catch (const Registry::WhoisImpl::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Registry::WhoisImpl::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

ObjectStatusDesc wrap_ObjectStatusDesc(const Registry::WhoisImpl::ObjectStatusDesc& osd)
{
    ObjectStatusDesc result;
    result.handle = LibFred::Corba::wrap_string_to_corba_string(osd.handle);
    result.name = LibFred::Corba::wrap_string_to_corba_string(osd.name);
    return result;
}

static ObjectStatusDescSeq* get_object_status_descriptions(
    const std::vector<Registry::WhoisImpl::ObjectStatusDesc>& state_vec)
{
    ObjectStatusDescSeq_var result = new ObjectStatusDescSeq;
    result->length(state_vec.size());
    CORBA::ULong i = 0;
    for (std::vector<Registry::WhoisImpl::ObjectStatusDesc>::const_iterator cit = state_vec.begin();
            cit != state_vec.end(); ++cit, ++i)
    {
        result[i] = wrap_ObjectStatusDesc(*cit);
    }
    return result._retn();
}

ObjectStatusDescSeq* Server_impl::get_domain_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_domain_status_descriptions(lang));
    }
    catch (const Registry::WhoisImpl::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

ObjectStatusDescSeq* Server_impl::get_contact_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_contact_status_descriptions(lang));
    }
    catch (const Registry::WhoisImpl::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

ObjectStatusDescSeq* Server_impl::get_nsset_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_nsset_status_descriptions(lang));
    }
    catch (const Registry::WhoisImpl::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

ObjectStatusDescSeq* Server_impl::get_keyset_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_keyset_status_descriptions(lang));
    }
    catch (const Registry::WhoisImpl::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

}//Whois
}//Registry
