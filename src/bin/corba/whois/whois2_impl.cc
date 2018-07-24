#include "src/bin/corba/Whois2.hh"
#include "src/bin/corba/whois/whois2_impl.hh"
#include "src/bin/corba/common_wrappers.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/bin/corba/util/corba_conversions_isodate.hh"
#include "src/bin/corba/util/corba_conversions_isodatetime.hh"
#include "src/bin/corba/util/corba_conversions_nullableisodatetime.hh"
#include "src/bin/corba/util/corba_conversions_nullable_types.hh"

#include <omniORB4/CORBA.h>

#include <vector>

namespace CorbaConversion
{
namespace Whois
{

Registry::Whois::PlaceAddress wrap_address(const Fred::Backend::Whois::PlaceAddress& address)
{
    Registry::Whois::PlaceAddress result;
    result.street1    = LibFred::Corba::wrap_string_to_corba_string(address.street1);
    result.street2    = LibFred::Corba::wrap_string_to_corba_string(address.street2);
    result.street3    = LibFred::Corba::wrap_string_to_corba_string(address.street3);
    result.city       = LibFred::Corba::wrap_string_to_corba_string(address.city);
    result.postalcode = LibFred::Corba::wrap_string_to_corba_string(address.postal_code);
    result.stateorprovince = LibFred::Corba::wrap_string_to_corba_string(address.stateorprovince);
    result.country_code    = LibFred::Corba::wrap_string_to_corba_string(address.country_code);
    return result;
}

Registry::Whois::Registrar wrap_registrar(const Fred::Backend::Whois::Registrar& registrar)
{
    Registry::Whois::Registrar result;
    result.handle  = LibFred::Corba::wrap_string_to_corba_string(registrar.handle);
    result.name    = LibFred::Corba::wrap_string_to_corba_string(registrar.name);
    result.organization = LibFred::Corba::wrap_string_to_corba_string(registrar.organization);
    result.url     = LibFred::Corba::wrap_string_to_corba_string(registrar.url);
    result.phone   = LibFred::Corba::wrap_string_to_corba_string(registrar.phone);
    result.fax     = LibFred::Corba::wrap_string_to_corba_string(registrar.fax);
    result.address = wrap_address(registrar.address);
    return result;
}

Registry::Whois::Registrar* Server_impl::get_registrar_by_handle(const char* handle)
{
    try
    {
        return new Registry::Whois::Registrar(
                wrap_registrar(
                    pimpl_->get_registrar_by_handle(std::string(handle))));
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    // default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::RegistrarSeq* Server_impl::get_registrars()
{
    try
    {
        Registry::Whois::RegistrarSeq_var result = new Registry::Whois::RegistrarSeq;
        const std::vector<Fred::Backend::Whois::Registrar>& registrars = pimpl_->get_registrars();
        result->length(registrars.size());
        for (CORBA::ULong i = 0; i < registrars.size(); ++i)
        {
            result[i] = wrap_registrar(registrars[i]);
        }
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::RegistrarGroup wrap_registrar_group(const Fred::Backend::Whois::RegistrarGroup& group)
{
    Registry::Whois::RegistrarGroup result;
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

Registry::Whois::RegistrarGroupList* Server_impl::get_registrar_groups()
{
    try
    {
        Registry::Whois::RegistrarGroupList_var result = new Registry::Whois::RegistrarGroupList;
        const std::vector<Fred::Backend::Whois::RegistrarGroup>& groups = pimpl_->get_registrar_groups();
        result->length(groups.size());
        for (CORBA::ULong i = 0; i < groups.size(); ++i)
        {
            result[i] = wrap_registrar_group(groups[i]);
        }
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::RegistrarCertification wrap_registrar_certification(const Fred::Backend::Whois::RegistrarCertification& cert)
{
    Registry::Whois::RegistrarCertification result;
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(cert.registrar);
    result.score = cert.score;
    result.evaluation_file_id = cert.evaluation_file_id;
    return result;
}

Registry::Whois::RegistrarCertificationList* Server_impl::get_registrar_certification_list()
{
    try
    {
        Registry::Whois::RegistrarCertificationList_var result = new Registry::Whois::RegistrarCertificationList;
        const std::vector<Fred::Backend::Whois::RegistrarCertification>& certifications = pimpl_->get_registrar_certification_list();
        result->length(certifications.size());
        for (CORBA::ULong i = 0; i < certifications.size(); ++i)
        {
            result[i] = wrap_registrar_certification(certifications[i]);
        }
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::ZoneFqdnList* Server_impl::get_managed_zone_list()
{
    try
    {
        Registry::Whois::ZoneFqdnList_var result = new Registry::Whois::ZoneFqdnList;
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
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::DisclosableString wrap_disclosable_string(const std::string& str, bool disclose)
{
    Registry::Whois::DisclosableString temp;
    temp.value = LibFred::Corba::wrap_string_to_corba_string(str);
    temp.disclose = disclose;
    return temp;
}

Registry::Whois::DisclosablePlaceAddress wrap_disclosable_address(const Fred::Backend::Whois::PlaceAddress& addr, bool disclose)
{
    Registry::Whois::DisclosablePlaceAddress temp;
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

Registry::Whois::Contact wrap_contact(const Fred::Backend::Whois::Contact& con)
{
    Registry::Whois::Contact result;
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
    result.created = Util::wrap_boost_posix_time_ptime_to_IsoDateTime(con.created);
    result.changed = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(con.changed);
    result.last_transfer = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(con.last_transfer);

    result.statuses.length(con.statuses.size());
    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator cit = con.statuses.begin();
            cit != con.statuses.end(); ++cit, ++i)
    {
        result.statuses[i] = LibFred::Corba::wrap_string_to_corba_string(*cit);
    }

    return result;
}

Registry::Whois::Contact* Server_impl::get_contact_by_handle(const char* handle)
{
    try
    {
        return new Registry::Whois::Contact(wrap_contact(pimpl_->get_contact_by_handle(handle)));
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

struct InvalidIPAddressException : public std::runtime_error
{
    InvalidIPAddressException() : std::runtime_error("invalid IP address") {}
};

Registry::Whois::IPAddress wrap_ipaddress(const boost::asio::ip::address& in)
{
    Registry::Whois::IPAddress result;
    result.address = LibFred::Corba::wrap_string(in.to_string());
    if (in.is_v4())
    {
        result.version = Registry::Whois::IPv4;
    }
    else if (in.is_v6())
    {
        result.version = Registry::Whois::IPv6;
    }
    else
    {
        throw InvalidIPAddressException();
    }
    return result;
}

Registry::Whois::NameServer wrap_nameserver(const Fred::Backend::Whois::NameServer& ns)
{
    Registry::Whois::NameServer result;
    result.fqdn = LibFred::Corba::wrap_string_to_corba_string(ns.fqdn);
    result.ip_addresses.length(ns.ip_addresses.size());
    for (CORBA::ULong i = 0; i < ns.ip_addresses.size(); ++i)
    {
        result.ip_addresses[i] = wrap_ipaddress(ns.ip_addresses[i]);
    }
    return result;
}

Registry::Whois::NSSet wrap_nsset(const Fred::Backend::Whois::NSSet& nsset)
{
    Registry::Whois::NSSet result;

    result.handle = LibFred::Corba::wrap_string_to_corba_string(nsset.handle);
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(nsset.sponsoring_registrar);
    result.created = Util::wrap_boost_posix_time_ptime_to_IsoDateTime(nsset.created);
    result.changed = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(nsset.changed);
    result.last_transfer = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(nsset.last_transfer);

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

Registry::Whois::NSSet* Server_impl::get_nsset_by_handle(const char* handle)
{
    try
    {
        return new Registry::Whois::NSSet(wrap_nsset(pimpl_->get_nsset_by_handle(handle)));
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::NSSetSeq* Server_impl::get_nssets_by_ns(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        Registry::Whois::NSSetSeq_var result = new Registry::Whois::NSSetSeq;
        Fred::Backend::Whois::NSSetSeq nss_seq = pimpl_->get_nssets_by_ns(handle, limit);
        limit_exceeded = nss_seq.limit_exceeded;
        result->length(nss_seq.content.size());
        for (CORBA::ULong i = 0; i < nss_seq.content.size(); ++i)
        {
            result[i] = wrap_nsset(nss_seq.content[i]);
        }
        return result._retn();
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::NSSetSeq* Server_impl::get_nssets_by_tech_c(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        Registry::Whois::NSSetSeq_var result = new Registry::Whois::NSSetSeq;
        Fred::Backend::Whois::NSSetSeq nss_seq = pimpl_->get_nssets_by_tech_c(handle, limit);
        limit_exceeded = nss_seq.limit_exceeded;
        result->length(nss_seq.content.size());
        for (CORBA::ULong i = 0; i < nss_seq.content.size(); ++i)
        {
            result[i] = wrap_nsset(nss_seq.content[i]);
        }
        return result._retn();
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::NameServer* Server_impl::get_nameserver_by_fqdn(const char* handle)
{
    try
    {
        Registry::Whois::NameServer result;
        result.fqdn = LibFred::Corba::wrap_string_to_corba_string(pimpl_->get_nameserver_by_fqdn(handle).fqdn);
        /*
         * Because of grouping nameservers in NSSet we don't include
         * IP address in output (given nameserver can be in different
         * NSSets with different IP addresses)
         *
         * result.ip_addresses;
         */
        return new Registry::Whois::NameServer(result);
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::DNSKey wrap_dnskey(const Fred::Backend::Whois::DNSKey& dnskey)
{
    Registry::Whois::DNSKey result;
    result.flags      = dnskey.flags;
    result.protocol   = dnskey.protocol;
    result.alg        = dnskey.alg;
    result.public_key = LibFred::Corba::wrap_string_to_corba_string(dnskey.public_key);
    return result;
}

Registry::Whois::KeySet wrap_keyset(const Fred::Backend::Whois::KeySet& keyset)
{
    Registry::Whois::KeySet result;

    result.handle  = LibFred::Corba::wrap_string_to_corba_string(keyset.handle);
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(keyset.sponsoring_registrar);
    result.created = Util::wrap_boost_posix_time_ptime_to_IsoDateTime(keyset.created);
    result.changed = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(keyset.changed);
    result.last_transfer = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(keyset.last_transfer);

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

Registry::Whois::KeySet* Server_impl::get_keyset_by_handle(const char* handle)
{
    try
    {
        return new Registry::Whois::KeySet(wrap_keyset(pimpl_->get_keyset_by_handle(handle)));
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::KeySetSeq* Server_impl::get_keysets_by_tech_c(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        Registry::Whois::KeySetSeq_var result = new Registry::Whois::KeySetSeq;
        Fred::Backend::Whois::KeySetSeq ks_seq = pimpl_->get_keysets_by_tech_c(handle, limit);
        limit_exceeded = ks_seq.limit_exceeded;
        result->length(ks_seq.content.size());
        for (CORBA::ULong i = 0; i < ks_seq.content.size(); ++i)
        {
            result[i] = wrap_keyset(ks_seq.content[i]);
        }
        return result._retn();
    }
    catch (const Fred::Backend::Whois::InvalidHandle& e)
    {
        throw Registry::Whois::INVALID_HANDLE();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::Domain wrap_domain(const Fred::Backend::Whois::Domain& domain)
{
    Registry::Whois::Domain result;
    result.handle = LibFred::Corba::wrap_string_to_corba_string(domain.fqdn);
    result.registrant_handle = LibFred::Corba::wrap_string_to_corba_string(domain.registrant);
    if (domain.nsset.size() == 0)
    {
        result.nsset_handle = NULL;
    }
    else
    {
        result.nsset_handle = new Registry::NullableString(LibFred::Corba::wrap_string_to_corba_string(domain.nsset));
    }
    if (domain.keyset.size() == 0)
    {
        result.keyset_handle = NULL;
    }
    else
    {
        result.keyset_handle = new Registry::NullableString(LibFred::Corba::wrap_string_to_corba_string(domain.keyset));
    }
    result.registrar_handle = LibFred::Corba::wrap_string_to_corba_string(domain.sponsoring_registrar);
    result.registered = Util::wrap_boost_posix_time_ptime_to_IsoDateTime(domain.registered);
    result.changed = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(domain.changed);
    result.last_transfer = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(domain.last_transfer);
    result.expire = Util::wrap_boost_gregorian_date_to_IsoDate(domain.expire);
    result.expire_time_estimate = Util::wrap_boost_posix_time_ptime_to_IsoDateTime(domain.expire_time_estimate);
    result.expire_time_actual = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(domain.expire_time_actual);
    if (domain.validated_to.isnull())
    {
        result.validated_to = NULL;
        result.validated_to_time_estimate = NULL;
        result.validated_to_time_actual = NULL;
    }
    else
    {
        result.validated_to = new Registry::NullableIsoDate(Util::wrap_boost_gregorian_date_to_IsoDate(domain.validated_to.get_value()));
        result.validated_to_time_estimate = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(domain.validated_to_time_estimate);
        result.validated_to_time_actual = Util::wrap_Nullable_boost_posix_time_ptime_to_NullableIsoDateTime(domain.validated_to_time_actual);
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

Registry::Whois::Domain* Server_impl::get_domain_by_handle(const char* handle)
{
    try
    {
        return new Registry::Whois::Domain(wrap_domain(pimpl_->get_domain_by_handle(handle)));
    }
    catch (const Fred::Backend::Whois::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (const Fred::Backend::Whois::ObjectDeleteCandidate& e)
    {
        throw Registry::Whois::OBJECT_DELETE_CANDIDATE();
    }
    catch (const Fred::Backend::Whois::TooManyLabels& e)
    {
        throw Registry::Whois::TOO_MANY_LABELS();
    }
    catch (const Fred::Backend::Whois::UnmanagedZone& e)
    {
        throw Registry::Whois::UNMANAGED_ZONE();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

static Registry::Whois::DomainSeq* get_domains_by_(
    const Fred::Backend::Whois::DomainSeq& dom_seq,
    ::CORBA::Boolean& limit_exceeded)
{
    Registry::Whois::DomainSeq_var result = new Registry::Whois::DomainSeq;
    limit_exceeded = dom_seq.limit_exceeded;
    result->length(dom_seq.content.size());
    for (CORBA::ULong i = 0; i < dom_seq.content.size(); ++i)
    {
        result[i] = wrap_domain(dom_seq.content[i]);
    }
    return result._retn();
}

Registry::Whois::DomainSeq* Server_impl::get_domains_by_registrant(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_registrant(handle, limit), limit_exceeded);
    }
    catch (const Fred::Backend::Whois::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::DomainSeq* Server_impl::get_domains_by_admin_contact(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_admin_contact(handle, limit), limit_exceeded);
    }
    catch (const Fred::Backend::Whois::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::DomainSeq* Server_impl::get_domains_by_nsset(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_nsset(handle, limit), limit_exceeded);
    }
    catch (const Fred::Backend::Whois::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::DomainSeq* Server_impl::get_domains_by_keyset(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        return get_domains_by_(pimpl_->get_domains_by_keyset(handle, limit), limit_exceeded);
    }
    catch (const Fred::Backend::Whois::InvalidLabel& e)
    {
        throw Registry::Whois::INVALID_LABEL();
    }
    catch (const Fred::Backend::Whois::ObjectNotExists& e)
    {
        throw Registry::Whois::OBJECT_NOT_FOUND();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::ObjectStatusDesc wrap_ObjectStatusDesc(const Fred::Backend::Whois::ObjectStatusDesc& osd)
{
    Registry::Whois::ObjectStatusDesc result;
    result.handle = LibFred::Corba::wrap_string_to_corba_string(osd.handle);
    result.name = LibFred::Corba::wrap_string_to_corba_string(osd.name);
    return result;
}

static Registry::Whois::ObjectStatusDescSeq* get_object_status_descriptions(
    const std::vector<Fred::Backend::Whois::ObjectStatusDesc>& state_vec)
{
    Registry::Whois::ObjectStatusDescSeq_var result = new Registry::Whois::ObjectStatusDescSeq;
    result->length(state_vec.size());
    CORBA::ULong i = 0;
    for (std::vector<Fred::Backend::Whois::ObjectStatusDesc>::const_iterator cit = state_vec.begin();
            cit != state_vec.end(); ++cit, ++i)
    {
        result[i] = wrap_ObjectStatusDesc(*cit);
    }
    return result._retn();
}

Registry::Whois::ObjectStatusDescSeq* Server_impl::get_domain_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_domain_status_descriptions(lang));
    }
    catch (const Fred::Backend::Whois::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::ObjectStatusDescSeq* Server_impl::get_contact_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_contact_status_descriptions(lang));
    }
    catch (const Fred::Backend::Whois::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::ObjectStatusDescSeq* Server_impl::get_nsset_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_nsset_status_descriptions(lang));
    }
    catch (const Fred::Backend::Whois::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

Registry::Whois::ObjectStatusDescSeq* Server_impl::get_keyset_status_descriptions(const char* lang)
{
    try
    {
        return get_object_status_descriptions(pimpl_->get_keyset_status_descriptions(lang));
    }
    catch (const Fred::Backend::Whois::MissingLocalization& e)
    {
        throw Registry::Whois::MISSING_LOCALIZATION();
    }
    catch (...) { }

    //default exception handling
    throw Registry::Whois::INTERNAL_SERVER_ERROR();
}

} // namespace CorbaConversion::Whois
} // namespace CorbaConversion::
