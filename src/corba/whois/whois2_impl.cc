#include "src/corba/whois/whois2_impl.h"
#include "src/corba/common_wrappers.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_datetime.h"
#include "src/corba/util/corba_conversions_nullable_types.h"

#include <omniORB4/CORBA.h>

#include <vector>

namespace Registry
{
namespace Whois
{

PlaceAddress wrap_address(const Registry::WhoisImpl::PlaceAddress& address)
{
    PlaceAddress result;
    result.street1    = Corba::wrap_string_to_corba_string(address.street1);
    result.street2    = Corba::wrap_string_to_corba_string(address.street2);
    result.street3    = Corba::wrap_string_to_corba_string(address.street3);
    result.city       = Corba::wrap_string_to_corba_string(address.city);
    result.postalcode = Corba::wrap_string_to_corba_string(address.postal_code);
    result.stateorprovince = Corba::wrap_string_to_corba_string(address.stateorprovince);
    result.country_code    = Corba::wrap_string_to_corba_string(address.country_code);
    return result;
}

Registrar wrap_registrar(const Registry::WhoisImpl::Registrar& registrar)
{
    Registrar result;
    result.handle  = Corba::wrap_string_to_corba_string(registrar.handle);
    result.name    = Corba::wrap_string_to_corba_string(registrar.name);
    result.organization = Corba::wrap_string_to_corba_string(registrar.organization);
    result.url     = Corba::wrap_string_to_corba_string(registrar.url);
    result.phone   = Corba::wrap_string_to_corba_string(registrar.phone);
    result.fax     = Corba::wrap_string_to_corba_string(registrar.fax);
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
    catch (...) { }

    // default exception handling
    throw INTERNAL_SERVER_ERROR();
}

template <class T, class SeqType>
void wrap_unbound_sequence(
    const std::vector<T>& vec,
    _CORBA_Unbounded_Sequence<SeqType>& seq,
    SeqType (*wrap_function)(const T&))
{
    seq.length(vec.size());
    for (CORBA::ULong i = 0; i < vec.size(); ++i)
    {
        seq[i] = wrap_function(vec[i]);
    }
}

RegistrarSeq* Server_impl::get_registrars()
{
    try
    {
        RegistrarSeq_var result = new RegistrarSeq;
        std::vector<Registry::WhoisImpl::Registrar> registrars = pimpl_->get_registrars();
        wrap_unbound_sequence(registrars, result.inout(), wrap_registrar);
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

void wrap_string_sequence(const std::vector<std::string>& str_vec, _CORBA_Unbounded_Sequence_String& seq)
{
    seq.length(str_vec.size());
    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator cit = str_vec.begin();
            cit != str_vec.end(); ++cit, ++i)
    {
        seq[i] = Corba::wrap_string_to_corba_string(*cit);
    }
}

RegistrarGroup wrap_registrar_group(const Registry::WhoisImpl::RegistrarGroup& group)
{
    RegistrarGroup result;
    result.name = Corba::wrap_string_to_corba_string(group.name);
    wrap_string_sequence(group.members, result.members);
    return result;
}

RegistrarGroupList* Server_impl::get_registrar_groups()
{
    try
    {
        RegistrarGroupList_var result = new RegistrarGroupList;
        const std::vector<Registry::WhoisImpl::RegistrarGroup>& reg_grps = pimpl_->get_registrar_groups();
        wrap_unbound_sequence(reg_grps, result.inout(), wrap_registrar_group);
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

RegistrarCertification wrap_registrar_certification(const Registry::WhoisImpl::RegistrarCertification& cert)
{
    RegistrarCertification result;
    result.registrar_handle = Corba::wrap_string_to_corba_string(cert.registrar);
    result.score = cert.score;
    result.evaluation_file_id = cert.evaluation_file_id;
    return result;
}

RegistrarCertificationList* Server_impl::get_registrar_certification_list()
{
    try
    {
        RegistrarCertificationList_var result = new RegistrarCertificationList;
        const std::vector<Registry::WhoisImpl::RegistrarCertification>& reg_certs = pimpl_->get_registrar_certification_list();
        wrap_unbound_sequence(reg_certs, result.inout(), wrap_registrar_certification);
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
        wrap_string_sequence(zones, result);
        return result._retn();
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DisclosableString wrap_disclosable_string(const std::string& str, bool disclose)
{
    DisclosableString temp;
    temp.value = Corba::wrap_string_to_corba_string(str);
    temp.disclose = disclose;
    return temp;
}

DisclosablePlaceAddress wrap_disclosable_address(const Registry::WhoisImpl::PlaceAddress& addr, bool disclose)
{
    DisclosablePlaceAddress temp;
    temp.value.street1 = Corba::wrap_string_to_corba_string(addr.street1);
    temp.value.street2 = Corba::wrap_string_to_corba_string(addr.street2);
    temp.value.street3 = Corba::wrap_string_to_corba_string(addr.street3);
    temp.value.city    = Corba::wrap_string_to_corba_string(addr.city);
    temp.value.stateorprovince = Corba::wrap_string_to_corba_string(addr.stateorprovince);
    temp.value.postalcode      = Corba::wrap_string_to_corba_string(addr.postal_code);
    temp.value.country_code    = Corba::wrap_string_to_corba_string(addr.country_code);
    temp.disclose = disclose;
    return temp;
}

Contact wrap_contact(const Registry::WhoisImpl::Contact& con)
{
    Contact result;
    result.handle       = Corba::wrap_string_to_corba_string(con.handle);
    result.organization = wrap_disclosable_string(con.organization, con.disclose_organization);
    result.name         = wrap_disclosable_string(con.name, con.disclose_name);
    result.address      = wrap_disclosable_address(con.address, con.disclose_address);
    result.phone        = wrap_disclosable_string(con.phone, con.disclose_phone);
    result.fax          = wrap_disclosable_string(con.fax, con.disclose_fax);;
    result.email        = wrap_disclosable_string(con.email, con.disclose_email);
    result.notify_email = wrap_disclosable_string(con.notify_email, con.disclose_notify_email);
    result.vat_number   = wrap_disclosable_string(con.vat_number, con.disclose_vat_number);

    result.identification.value.identification_type =
            Corba::wrap_string_to_corba_string(con.identification.identification_type);
    result.identification.value.identification_data =
            Corba::wrap_string_to_corba_string(con.identification.identification_data);
    result.identification.disclose = con.disclose_identification;

    result.creating_registrar_handle =
            Corba::wrap_string_to_corba_string(con.creating_registrar);
    result.sponsoring_registrar_handle =
            Corba::wrap_string_to_corba_string(con.sponsoring_registrar);
    result.created = Corba::wrap_time(con.created);
    result.changed = Corba::wrap_nullable_datetime(con.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(con.last_transfer);

    wrap_string_sequence(con.statuses, result.statuses);

    return result;
}

Contact* Server_impl::get_contact_by_handle(const char* handle)
{
    try
    {
        return new Contact(wrap_contact(pimpl_->get_contact_by_handle(handle)));
    }
    catch (const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

struct InvalidIPAddressException : public std::runtime_error 
{
    static const char* what_msg() throw() {return "invalid IP address";}

    InvalidIPAddressException()
    : std::runtime_error(what_msg())
    {}

    const char* what() const throw() {return what_msg();}
};

/**
 * @throws InvalidIPAddressException
 */
void wrap_ipaddress(const boost::asio::ip::address& in, IPAddress& out )
{
    out.address = Corba::wrap_string(in.to_string());
    if (in.is_v4()) 
    {
        out.version = IPv4;
    }
    else if (in.is_v6()) 
    {
        out.version = IPv6;
    }
    else 
    {
        throw InvalidIPAddressException();
    }
}

NameServer wrap_nameserver(const Registry::WhoisImpl::NameServer& ns)
{
    NameServer result;
    result.fqdn = Corba::wrap_string_to_corba_string(ns.fqdn);
    wrap_unbound_sequence(ns.ip_addresses, result.ip_addresses, wrap_ipaddress);
    return result;
}

NSSet wrap_nsset(const Registry::WhoisImpl::NSSet& nsset)
{
    NSSet result;

    result.handle = Corba::wrap_string_to_corba_string(nsset.handle);
    result.registrar_handle = Corba::wrap_string_to_corba_string(nsset.sponsoring_registrar);
    result.created = Corba::wrap_time(nsset.created);
    result.changed = Corba::wrap_nullable_datetime(nsset.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(nsset.last_transfer);

    wrap_unbound_sequence(nsset.nservers, result.nservers, wrap_nameserver);
    wrap_string_sequence(nsset.tech_contacts, result.tech_contact_handles);
    wrap_string_sequence(nsset.statuses, result.statuses);
    return result;
}

NSSet* Server_impl::get_nsset_by_handle(const char* handle)
{
    try
    {
        return new NSSet(wrap_nsset(pimpl_->get_nsset_by_handle(handle)));
    }
    catch (const ::CORBA::UserException& )
    {
        throw;
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
        wrap_unbound_sequence(nss_seq.content, result.inout(), wrap_nsset);
        return result._retn();
    } 
    catch (const ::CORBA::UserException&)
    {
        throw;
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
        wrap_unbound_sequence(nss_seq.content, result.inout(), wrap_nsset);
        return result._retn();
    } 
    catch (const ::CORBA::UserException& )
    {
        throw;
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
        result.fqdn = Corba::wrap_string_to_corba_string(pimpl_->get_nameserver_by_fqdn(handle).fqdn);
        /*
         * Because of grouping nameservers in NSSet we don't include
         * IP address in output (given nameserver can be in different
         * NSSets with different IP addresses)
         *
         * result.ip_addresses;
         */
        return new NameServer(result);
    }
    catch (const ::CORBA::UserException& )
    {
        throw;
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
    result.public_key = Corba::wrap_string_to_corba_string(dnskey.public_key);
    return result;
}

KeySet wrap_keyset(const Registry::WhoisImpl::KeySet& keyset)
{
    KeySet result;

    result.handle  = Corba::wrap_string_to_corba_string(keyset.handle);
    result.registrar_handle = Corba::wrap_string_to_corba_string(keyset.sponsoring_registrar);
    result.created = Corba::wrap_time(keyset.created);
    result.changed = Corba::wrap_nullable_datetime(keyset.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(keyset.last_transfer);

    wrap_string_sequence(keyset.tech_contacts, result.tech_contact_handles);
    wrap_string_sequence(keyset.statuses, result.statuses);
    wrap_unbound_sequence(keyset.dns_keys, result.dns_keys, wrap_dnskey);
    return result;
}

KeySet* Server_impl::get_keyset_by_handle(const char* handle)
{
    try
    {
        return new KeySet(wrap_keyset(pimpl_->get_keyset_by_handle(handle)));
    }
    catch (const ::CORBA::UserException& )
    {
        throw;
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
        wrap_unbound_sequence(ks_seq.content, result.inout(), wrap_keyset);
        return result._retn();
    }
    catch (const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

Domain wrap_domain(const Registry::WhoisImpl::Domain& domain)
{
    Domain result;
    result.handle = Corba::wrap_string_to_corba_string(domain.fqdn);
    result.registrant_handle = Corba::wrap_string_to_corba_string(domain.registrant);
    if (domain.nsset.size() == 0)
    {
        result.nsset_handle = NULL;
    }
    else 
    {
        result.nsset_handle = new NullableString(Corba::wrap_string_to_corba_string(domain.nsset));
    }
    if (domain.keyset.size() == 0)
    {
        result.keyset_handle = NULL;
    }
    else 
    {
        result.keyset_handle = new NullableString(Corba::wrap_string_to_corba_string(domain.keyset));
    }
    result.registrar_handle = Corba::wrap_string_to_corba_string(domain.sponsoring_registrar);
    result.registered = Corba::wrap_time(domain.registered);
    result.changed = Corba::wrap_nullable_datetime(domain.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(domain.last_transfer);
    result.expire = Corba::wrap_date(domain.expire);
    result.expire_time_estimate = Corba::wrap_time(domain.expire_time_estimate);
    result.expire_time_actual = Corba::wrap_nullable_datetime(domain.expire_time_actual);
    if (domain.validated_to.isnull()) 
    {
        result.validated_to = NULL;
        result.validated_to_time_estimate = NULL;
        result.validated_to_time_actual = NULL;
    }
    else 
    {
        result.validated_to = new Registry::NullableDate(Corba::wrap_date(domain.validated_to.get_value()));
        result.validated_to_time_estimate = Corba::wrap_nullable_datetime(domain.validated_to_time_estimate);
        result.validated_to_time_actual = Corba::wrap_nullable_datetime(domain.validated_to_time_actual);
    } 
    wrap_string_sequence(domain.admin_contacts, result.admin_contact_handles);
    wrap_string_sequence(domain.statuses, result.statuses);

    return result;

}

Domain* Server_impl::get_domain_by_handle(const char* handle)
{
    try
    {
        return new Domain(wrap_domain(pimpl_->get_domain_by_handle(handle)));
    }
    catch (const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

static DomainSeq* get_domains_by_(
    const Registry::WhoisImpl::DomainSeq& dom_seq,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        DomainSeq_var result = new DomainSeq;
        limit_exceeded = dom_seq.limit_exceeded;
        wrap_unbound_sequence(dom_seq.content, result.inout(), wrap_domain);
        return result._retn();
    }
    catch (const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DomainSeq* Server_impl::get_domains_by_registrant(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    return get_domains_by_(pimpl_->get_domains_by_registrant(handle, limit), limit_exceeded);
}

DomainSeq* Server_impl::get_domains_by_admin_contact(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    return get_domains_by_(pimpl_->get_domains_by_admin_contact(handle, limit), limit_exceeded);
}

DomainSeq* Server_impl::get_domains_by_nsset(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    return get_domains_by_(pimpl_->get_domains_by_nsset(handle, limit), limit_exceeded);
}

DomainSeq* Server_impl::get_domains_by_keyset(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    return get_domains_by_(pimpl_->get_domains_by_keyset(handle, limit), limit_exceeded);
}

ObjectStatusDesc wrap_ObjectStatusDesc(const Registry::WhoisImpl::ObjectStatusDesc& osd)
{
    ObjectStatusDesc result;
    result.handle = Corba::wrap_string_to_corba_string(osd.handle);
    result.name = Corba::wrap_string_to_corba_string(osd.name);
    return result;
}

static ObjectStatusDescSeq* get_object_status_descriptions(
    const std::vector<Registry::WhoisImpl::ObjectStatusDesc>& state_vec)
{
    try
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
    catch (...) { }

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

ObjectStatusDescSeq* Server_impl::get_domain_status_descriptions(const char* lang)
{
    return get_object_status_descriptions(pimpl_->get_domain_status_descriptions(lang));
}

ObjectStatusDescSeq* Server_impl::get_contact_status_descriptions(const char* lang)
{
    return get_object_status_descriptions(pimpl_->get_contact_status_descriptions(lang));
}

ObjectStatusDescSeq* Server_impl::get_nsset_status_descriptions(const char* lang)
{
    return get_object_status_descriptions(pimpl_->get_nsset_status_descriptions(lang));
}

ObjectStatusDescSeq* Server_impl::get_keyset_status_descriptions(const char* lang)
{
    return get_object_status_descriptions(pimpl_->get_keyset_status_descriptions(lang));
}

}//Whois 
}//Registry
