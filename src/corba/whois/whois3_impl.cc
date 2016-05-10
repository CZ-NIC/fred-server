#include <src/corba/whois/whois3_impl.h>
#include <src/corba/common_wrappers.h>
#include <src/corba/util/corba_conversions_string.h>
#include <src/corba/util/corba_conversions_datetime.h>
#include <src/corba/util/corba_conversions_nullable_types.h>

#include <src/whois/whois.h>


namespace Registry {
namespace Whois {

const std::string Server_impl::output_timezone("UTC");

PlaceAddress wrap_address(Registry::WhoisImpl::PlaceAddress& address)
{
    PlaceAddress result;
    result.street1    = Corba::wrap_string_to_corba_string(address.street1);
    result.street2    = Corba::wrap_string_to_corba_string(address.street2);
    result.street3    = Corba::wrap_string_to_corba_string(address.street3);
    result.city       = Corba::wrap_string_to_corba_string(address.city);
    result.postalcode = Corba::wrap_string_to_corba_string(address.postal_code); //fix consistency?
    result.stateorprovince = Corba::wrap_string_to_corba_string(address.stateorprovince);
    result.country_code    = Corba::wrap_string_to_corba_string(address.country_code);
    return result;
}

Registrar wrap_registrar(Registry::WhoisImpl::Registrar& registrar)
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
        Registry::WhoisImpl::Registrar whois_reg =
            pimpl_->get_registrar_by_handle(std::string(handle));
        return new Registrar(wrap_registrar(whois_reg));//?
    }
    catch (...)
    {}

    // default exception handling
    throw INTERNAL_SERVER_ERROR();
}

RegistrarSeq* Server_impl::get_registrars()
{
    try
    {
        RegistrarSeq_var result = new RegistrarSeq;
        std::vector<Registry::WhoisImpl::Registrar> registrars =
            pimpl_->get_registrars();
        result->length(registrars.size());
        for(CORBA::ULong i = 0; i < result->length(); ++i)
        {
            result[i] = wrap_registrar(registrars[i]);
        }
        return result._retn();
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

/**
 * CORBA sequence element factory, template to be specialized, there is no generic enough implementation
 */
template<class CORBA_SEQ_ELEMENT, class IN_LIST_ELEMENT>
CORBA_SEQ_ELEMENT set_element_of_corba_seq(const IN_LIST_ELEMENT& ile);

template<> CORBA::String_var set_element_of_corba_seq<
    CORBA::String_var,std::string>(const std::string& ile)
{
    return Corba::wrap_string_to_corba_string(ile);
}

/** COPIED FROM src/corba/whois2_impl
 * generic implementation of allocation and setting CORBA sequence
 * from container with begin(), end(), size() and value_type member
 */
template<class CORBA_SEQ, class CORBA_SEQ_ELEMENT, class IN_LIST>
void set_corba_seq(CORBA_SEQ& cs, const IN_LIST& il)
{
    cs.length(il.size());
    unsigned long long i = 0;
    for(typename IN_LIST::const_iterator ci = il.begin() ; ci != il.end(); ++ci,++i)
    {
        cs[i] = set_element_of_corba_seq<CORBA_SEQ_ELEMENT, 
            typename IN_LIST::value_type>(*ci);
    }
}

RegistrarGroup wrap_registrar_group(const Registry::WhoisImpl::RegistrarGroup& group)
{
    RegistrarGroup result;
    result.name = Corba::wrap_string_to_corba_string(group.name);
    set_corba_seq<RegistrarHandleList, CORBA::String_var>(result.members, group.members);
    return result;
}

template<> RegistrarGroup set_element_of_corba_seq<
    RegistrarGroup, Registry::WhoisImpl::RegistrarGroup>(
        const Registry::WhoisImpl::RegistrarGroup& ile)
{
    return wrap_registrar_group(ile);
}

RegistrarGroupList* Server_impl::get_registrar_groups()
{
    try
    {
        RegistrarGroupList_var result = new RegistrarGroupList;

        set_corba_seq<RegistrarGroupList, RegistrarGroup>(
                result, pimpl_->get_registrar_groups());

        return result._retn();
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

RegistrarCertification wrap_registrar_certification(
    const Registry::WhoisImpl::RegistrarCertification& cert)
{
    RegistrarCertification result;
    result.registrar_handle = Corba::wrap_string_to_corba_string(cert.registrar_handle);
    result.score = cert.score;
    result.evaluation_file_id = cert.evaluation_file_id;
    return result;
}

template<> RegistrarCertification set_element_of_corba_seq<
    RegistrarCertification, Registry::WhoisImpl::RegistrarCertification>(
        const Registry::WhoisImpl::RegistrarCertification& ile)
{
    return wrap_registrar_certification(ile);
}

RegistrarCertificationList* Server_impl::get_registrar_certification_list()
{
    try
    {
        RegistrarCertificationList_var result = new RegistrarCertificationList;
        set_corba_seq<RegistrarCertificationList, RegistrarCertification>(
                result, pimpl_->get_registrar_certification_list());
        return result._retn();
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

ZoneFqdnList* Server_impl::get_managed_zone_list() 
{
    try
    {
        ZoneFqdnList_var zone_seq = new ZoneFqdnList;
        set_corba_seq<ZoneFqdnList, CORBA::String_var>(
                zone_seq, pimpl_->get_managed_zone_list());
        return zone_seq._retn();
    }
    catch (...)
    {}

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

void wrap_object_states(StringSeq& states_seq, std::vector<std::string>& statuses)
{
//    BOOST_FOREACH(std::string status, statuses) 
//    {
//        statuses.push_back(state.state_name);
//    }
    set_corba_seq<StringSeq, CORBA::String_var>(states_seq, statuses);
}

DisclosablePlaceAddress wrap_disclosable_address(const Registry::WhoisImpl::PlaceAddress& addr, bool disclose)
{
    DisclosablePlaceAddress temp;
    temp.value.street1 = Corba::wrap_string_to_corba_string(addr.street1);
    temp.value.street2 = Corba::wrap_string_to_corba_string(addr.street2);
    temp.value.street3 = Corba::wrap_string_to_corba_string(addr.street3);
    temp.value.city = Corba::wrap_string_to_corba_string(addr.city);
    temp.value.stateorprovince = Corba::wrap_string_to_corba_string(addr.stateorprovince);
    temp.value.postalcode = Corba::wrap_string_to_corba_string(addr.postal_code); //fix consistency?
    temp.value.country_code = Corba::wrap_string_to_corba_string(addr.country_code);
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
        Corba::wrap_string_to_corba_string(con.creating_registrar_handle);
    result.sponsoring_registrar_handle =
        Corba::wrap_string_to_corba_string(con.sponsoring_registrar_handle);
    result.created = Corba::wrap_time(con.created);
    result.changed = Corba::wrap_nullable_datetime(con.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(con.last_transfer);

    set_corba_seq<StringSeq, CORBA::String_var>(result.statuses, con.statuses);

    return result;
}

Contact* Server_impl::get_contact_by_handle(const char* handle)
{
    try
    {
        return new Contact(wrap_contact(pimpl_->get_contact_by_handle(handle)));
    }
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

struct InvalidIPAddressException : public std::runtime_error {
    static const char* what_msg() throw() {return "invalid IP address";}

    InvalidIPAddressException()
        : std::runtime_error(what_msg())
    { }

    const char* what() const throw() {return what_msg();}
};

/**
 * @throws InvalidIPAddressException
 */
void wrap_ipaddress(const boost::asio::ip::address& in, IPAddress& out ) {
    out.address = Corba::wrap_string(in.to_string());
    if(in.is_v4()) {
        out.version = IPv4;
    } else if(in.is_v6()) {
        out.version = IPv6;
    } else {
        throw InvalidIPAddressException();
    }
}

template<> IPAddress set_element_of_corba_seq<
    IPAddress, boost::asio::ip::address>(const boost::asio::ip::address& ile)
{
    IPAddress ip;
    wrap_ipaddress(ile,ip);
    return ip;
}

template<> NameServer set_element_of_corba_seq<
    NameServer, Registry::WhoisImpl::NameServer>(const Registry::WhoisImpl::NameServer& ns)
{
    NameServer result;
    result.fqdn = Corba::wrap_string_to_corba_string(ns.fqdn);
    set_corba_seq<IPAddressSeq, IPAddress>(result.ip_addresses, ns.ip_addresses);
    return result;
}

NSSet wrap_nsset(const Registry::WhoisImpl::NSSet& nsset)
{
    NSSet result;

    result.handle = Corba::wrap_string_to_corba_string(nsset.handle);
    result.registrar_handle = Corba::wrap_string_to_corba_string(nsset.registrar_handle);
    result.created = Corba::wrap_time(nsset.created);
    result.changed = Corba::wrap_nullable_datetime(nsset.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(nsset.last_transfer);

    set_corba_seq<NameServerSeq, NameServer>(result.nservers, nsset.nservers);

    set_corba_seq<StringSeq, CORBA::String_var>(
            result.tech_contact_handles, nsset.tech_contact_handles);

    set_corba_seq<StringSeq, CORBA::String_var>(result.statuses, nsset.statuses);
    return result;
}

NSSet* Server_impl::get_nsset_by_handle(const char* handle)
{
    try
    {
        return new NSSet(wrap_nsset(pimpl_->get_nsset_by_handle(handle)));
    }
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

template<> NSSet set_element_of_corba_seq<NSSet, Registry::WhoisImpl::NSSet>(
    const Registry::WhoisImpl::NSSet& nsset)
{
    return wrap_nsset(nsset);
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
        set_corba_seq<NSSetSeq, NSSet>(result.inout(), nss_seq.content);
        return result._retn();
    } 
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

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
        set_corba_seq<NSSetSeq, NSSet>(result.inout(), nss_seq.content);
        return result._retn();
    } 
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

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
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

template<> DNSKey set_element_of_corba_seq<
    DNSKey, Registry::WhoisImpl::DNSKey>(const Registry::WhoisImpl::DNSKey& dnskey)
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
    result.registrar_handle = Corba::wrap_string_to_corba_string(keyset.registrar_handle);
    result.created = Corba::wrap_time(keyset.created);
    result.changed = Corba::wrap_nullable_datetime(keyset.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(keyset.last_transfer);

    set_corba_seq<StringSeq, CORBA::String_var>(result.tech_contact_handles, keyset.tech_contact_handles);

    set_corba_seq<StringSeq, CORBA::String_var>(result.statuses, keyset.statuses);

    set_corba_seq<DNSKeySeq, DNSKey>(result.dns_keys, keyset.dns_keys);//?

    return result;
}

KeySet* Server_impl::get_keyset_by_handle(const char* handle)
{
    try
    {
        return new KeySet(wrap_keyset(pimpl_->get_keyset_by_handle(handle)));
    }
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

template<> KeySet set_element_of_corba_seq<KeySet, Registry::WhoisImpl::KeySet>(
    const Registry::WhoisImpl::KeySet& keyset)
{
    return wrap_keyset(keyset);
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
        set_corba_seq<KeySetSeq, KeySet>(result.inout(), ks_seq.content);
        return result._retn();
    }
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

Domain wrap_domain(const Registry::WhoisImpl::Domain& domain)
{
    Domain result;
    result.handle = Corba::wrap_string_to_corba_string(domain.fqdn);
    result.registrant_handle = Corba::wrap_string_to_corba_string(domain.registrant_handle);
    if(domain.nsset_handle.size() == 0)
    {
        result.nsset_handle = NULL;
    }
    else 
    {
        result.nsset_handle = new NullableString(Corba::wrap_string_to_corba_string(domain.nsset_handle));
    }
    if(domain.keyset_handle.size() == 0)
    {
        result.keyset_handle = NULL;
    }
    else 
    {
        result.keyset_handle = new NullableString(Corba::wrap_string_to_corba_string(domain.keyset_handle));
    }
    result.registrar_handle = Corba::wrap_string_to_corba_string(domain.registrar_handle);
    result.registered = Corba::wrap_time(domain.registered);
    result.changed = Corba::wrap_nullable_datetime(domain.changed);
    result.last_transfer = Corba::wrap_nullable_datetime(domain.last_transfer);
    result.expire = Corba::wrap_date(domain.expire);
    //??
//    result.expire_time_estimate = Corba::wrap_time(_expire_time_estimate);
//    result.expire_time_actual = Corba::wrap_optional_datetime(_expire_time_actual);
    if(domain.validated_to.isnull()) 
    {
        result.validated_to = NULL;
        result.validated_to_time_estimate = NULL;
        result.validated_to_time_actual = NULL;
    }
    else 
    {
        result.validated_to = new Registry::NullableDate(
                Corba::wrap_date(domain.validated_to.get_value()));
        //       ?
//        result.validated_to_time_estimate = Corba::wrap_optional_datetime(_val_expire_time_estimate);
//        result.validated_to_time_actual = Corba::wrap_optional_datetime(_val_expire_time_actual);
    } 
    set_corba_seq<StringSeq, CORBA::String_var>(result.admin_contact_handles, domain.admin_contact_handles);

    set_corba_seq<StringSeq, CORBA::String_var>(result.statuses, domain.statuses);
    return result;

}

Domain* Server_impl::get_domain_by_handle(const char* handle)
{
    try
    {
        return new Domain(wrap_domain(pimpl_->get_domain_by_handle(handle)));
    }
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

    //default exception handling
    throw INTERNAL_SERVER_ERROR();
}

DomainSeq* Server_impl::get_domains_by_(const Registry::WhoisImpl::DomainSeq& dom_seq, ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        DomainSeq_var result = new DomainSeq;
        limit_exceeded = dom_seq.limit_exceeded;
        set_corba_seq<DomainSeq, Domain>(result.inout(), dom_seq.content);
        return result._retn();
    }
    catch(const ::CORBA::UserException& )
    {
        throw;
    }
    catch (...)
    {}

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

ObjectStatusDescSeq* Server_impl::get_object_status_descriptions(const std::vector<Registry::WhoisImpl::ObjectStatusDesc>& state_vec)
{
    try
    {
        ObjectStatusDescSeq_var state_seq = new ObjectStatusDescSeq;
        set_corba_seq<ObjectStatusDescSeq, ObjectStatusDesc>(state_seq, state_vec);
        return state_seq._retn();
    }
    catch (...)
    {}

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

}
}
