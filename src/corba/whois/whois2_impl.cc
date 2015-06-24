#include "src/corba/whois/whois2_impl.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_datetime.h"
#include "src/corba/util/corba_conversions_nullable_types.h"

#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"

#include "src/whois/nameserver_exists.h"
#include "src/whois/is_domain_delete_pending.h"

#include <boost/foreach.hpp>

#include <stdexcept>

namespace Registry {
namespace Whois {

    const std::string Server_impl::output_timezone("UTC");

    DisclosableString* wrap_disclosable_string(const Nullable<std::string>& in, bool to_disclose) {
        if(!to_disclose) {
            return NULL;
        }

        return new DisclosableString(in.get_value_or_default().c_str());
    }

    // at least contact and registrar have address (result of different info operations == different types) ...
    template<typename Tinfo> PlaceAddress wrap_address(const Tinfo& in);

    template<> PlaceAddress wrap_address(const Fred::InfoContactData& in) {
        const Fred::Contact::PlaceAddress in_place = in.place.get_value_or_default();
        PlaceAddress result;
        result.street1 =          Corba::wrap_string(in_place.street1);
        result.street2 =          Corba::wrap_string(in_place.street2.get_value_or_default());
        result.street3 =          Corba::wrap_string(in_place.street3.get_value_or_default());
        result.city =             Corba::wrap_string(in_place.city);
        result.postalcode =       Corba::wrap_string(in_place.postalcode);
        result.stateorprovince =  Corba::wrap_string(in_place.stateorprovince.get_value_or_default());
        result.country_code =     Corba::wrap_string(in_place.country);

        return result;
    }

    template<> PlaceAddress wrap_address(const Fred::InfoRegistrarData& in) {
        PlaceAddress result;
        result.street1 =          Corba::wrap_string(in.street1.get_value_or_default());
        result.street2 =          Corba::wrap_string(in.street2.get_value_or_default());
        result.street3 =          Corba::wrap_string(in.street3.get_value_or_default());
        result.city =             Corba::wrap_string(in.city.get_value_or_default());
        result.postalcode =       Corba::wrap_string(in.postalcode.get_value_or_default());
        result.stateorprovince =  Corba::wrap_string(in.stateorprovince.get_value_or_default());
        result.country_code =     Corba::wrap_string(in.country.get_value_or_default());

        return result;
    }

    DisclosablePlaceAddress* wrap_disclosable_address(const Fred::InfoContactData& in) {
        if( ! in.discloseaddress ) {
            return NULL;
        }

        return new DisclosablePlaceAddress( wrap_address(in) );
    }

    DisclosableContactIdentification* wrap_disclosable_identification(const Fred::InfoContactData& in) {
        if( ! in.discloseident ) {
            return NULL;
        }

        DisclosableContactIdentification_var result(new DisclosableContactIdentification);

        result->identification_type(in.ssntype.get_value_or_default().c_str());
        result->identification_data(in.ssn.get_value_or_default().c_str());

        return result._retn();
    }

    template<typename T_container>
        void wrap_string_sequence(const T_container& in, StringSeq& out )
    {
        out.length(in.size());

        typename T_container::size_type i = 0;

        BOOST_FOREACH(const std::string& item, in) {
            out[i] = Corba::wrap_string(item);
            ++i;
        }
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

    NullableRegistrar*  wrap_registrar(const Fred::InfoRegistrarData& in) {
        Registrar temp;
        temp.handle = Corba::wrap_string(in.handle);
        temp.organization = Corba::wrap_string(in.organization.get_value_or_default());
        temp.url = Corba::wrap_string(in.url.get_value_or_default());
        temp.phone = Corba::wrap_string(in.telephone.get_value_or_default());
        temp.address = wrap_address(in);

        return new NullableRegistrar(temp);
    }

    NullableContact* wrap_contact(const Fred::InfoContactData& in) {

        Contact temp;

        temp.handle = Corba::wrap_string(in.handle);
        temp.organization = wrap_disclosable_string(in.organization, in.discloseorganization);
        temp.name = wrap_disclosable_string(in.name, in.disclosename);
        temp.address = wrap_disclosable_address(in);
        temp.phone = wrap_disclosable_string(in.telephone, in.disclosetelephone);
        temp.fax = wrap_disclosable_string(in.fax, in.disclosefax);
        temp.email = wrap_disclosable_string(in.email, in.discloseemail);
        temp.notify_email = wrap_disclosable_string(in.notifyemail, in.disclosenotifyemail);
        temp.vat_number = wrap_disclosable_string(in.vat, in.disclosevat);
        temp.identification = wrap_disclosable_identification(in);
        temp.creating_registrar_handle = Corba::wrap_string(in.create_registrar_handle);
        temp.sponsoring_registrar_handle = Corba::wrap_string(in.sponsoring_registrar_handle);
        temp.created = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);

        std::vector<std::string> statuses;
        {
            Fred::OperationContext ctx;

            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(in.id).exec(ctx)) {
                if(state.is_external) {
                    statuses.push_back(state.state_name);
                }
            }
        }
        wrap_string_sequence(statuses, temp.statuses);

        return new NullableContact(temp);
    }

    NullableDomain* generate_obfuscate_domain_delete_candidate(const std::string& _handle, Fred::OperationContext& _ctx) {
        Domain temp;

        temp.handle = Corba::wrap_string(_handle);

        temp.registrant_handle = Corba::wrap_string("");
        temp.nsset_handle = NULL;
        temp.keyset_handle = NULL;
        temp.registrar_handle = Corba::wrap_string("");
        temp.registered = Corba::wrap_time(boost::posix_time::ptime());
        temp.changed = Corba::wrap_nullable_datetime(Nullable<boost::posix_time::ptime>());
        temp.last_transfer = Corba::wrap_nullable_datetime(Nullable<boost::posix_time::ptime>());
        temp.expire = Corba::wrap_date(boost::gregorian::date());
        temp.validated_to = NULL;

        std::vector<std::string> statuses;
        statuses.push_back("deleteCandidate");
        wrap_string_sequence(statuses, temp.statuses);

        return new NullableDomain(temp);
    }

    NullableDomain* wrap_domain(const Fred::InfoDomainData& in) {

        Domain temp;

        temp.handle = Corba::wrap_string(in.fqdn);
        temp.registrant_handle = Corba::wrap_string(in.registrant.handle);
        if( in.nsset.isnull() ) {
            temp.nsset_handle = NULL;
        } else {
            temp.nsset_handle = new NullableString(Corba::wrap_string(in.nsset.get_value().handle));
        }
        if( in.keyset.isnull() ) {
            temp.keyset_handle = NULL;
        } else {
            temp.keyset_handle = new NullableString(Corba::wrap_string(in.keyset.get_value().handle));
        }
        temp.registrar_handle = Corba::wrap_string(in.sponsoring_registrar_handle);
        temp.registered = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);
        temp.expire = Corba::wrap_date(in.expiration_date);
        if(! in.enum_domain_validation.isnull()) {
            temp.validated_to = new Registry::NullableDate(
                Corba::wrap_date(in.enum_domain_validation.get_value().validation_expiration)
            );
        } else {
            temp.validated_to = NULL;
        }

        std::vector<std::string> admin_contacts;
        {
            BOOST_FOREACH(const Fred::ObjectIdHandlePair& contact, in.admin_contacts) {
                admin_contacts.push_back(contact.handle);
            }
        }
        wrap_string_sequence(admin_contacts, temp.admin_contact_handles);

        std::vector<std::string> statuses;
        {
            Fred::OperationContext ctx;

            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(in.id).exec(ctx)) {
                if(state.is_external) {
                    statuses.push_back(state.state_name);
                }
            }
        }
        wrap_string_sequence(statuses, temp.statuses);

        return new NullableDomain(temp);
    }

    /**
     * CORBA sequence element factory, template to be specialized, there is no generic enough implementation
     */
    template<class CORBA_SEQ_ELEMENT, class IN_LIST_ELEMENT>
        CORBA_SEQ_ELEMENT set_element_of_corba_seq(const IN_LIST_ELEMENT& ile);

    /**
     * generic implementation of allocation and setting CORBA sequence
     */
    template<class CORBA_SEQ, class CORBA_SEQ_ELEMENT,
        class IN_LIST, class IN_LIST_ELEMENT> void set_corba_seq(CORBA_SEQ& cs, const IN_LIST& il)
    {
        cs.length(il.size());
        for(unsigned long long i = 0 ; i < il.size(); ++i)
        {
            cs[i] = set_element_of_corba_seq<CORBA_SEQ_ELEMENT, IN_LIST_ELEMENT>(il[i]);
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
        NameServer, Fred::DnsHost>(const Fred::DnsHost& ile)
    {
        NameServer ns;
        ns.fqdn = Corba::wrap_string_to_corba_string(ile.get_fqdn());
        set_corba_seq<IPAddressSeq, IPAddress,
            std::vector<boost::asio::ip::address>,
            boost::asio::ip::address>(ns.ip_addresses, ile.get_inet_addr());
        return ns;
    }

    template<> CORBA::String_var set_element_of_corba_seq<
        CORBA::String_var, Fred::ObjectIdHandlePair>(const Fred::ObjectIdHandlePair& ile)
    {
        return Corba::wrap_string_to_corba_string(ile.handle);
    }

    template<> CORBA::String_var set_element_of_corba_seq<
        CORBA::String_var,Fred::ObjectStateData>(
            const Fred::ObjectStateData& ile)
    {
        return Corba::wrap_string_to_corba_string(ile.state_name);
    }

    template<> CORBA::String_var set_element_of_corba_seq<
        CORBA::String_var,std::string>(
            const std::string& ile)
    {
        return Corba::wrap_string_to_corba_string(ile);
    }

    //set_element_of_corba_seq<Registry::Whois::KeySet, Fred::InfoKeysetOutput>

    template<> DNSKey set_element_of_corba_seq<
    DNSKey, Fred::DnsKey>(const Fred::DnsKey& ile)
    {
        DNSKey key;
        key.flags = ile.get_flags();
        key.protocol = ile.get_protocol();
        key.alg = ile.get_alg();
        key.public_key = Corba::wrap_string_to_corba_string(ile.get_key());
        return key;
    }

    KeySet wrap_keyset(const Fred::InfoKeysetData& in)
    {
        KeySet temp;

        temp.handle = Corba::wrap_string_to_corba_string(in.handle);
        temp.registrar_handle = Corba::wrap_string_to_corba_string(in.sponsoring_registrar_handle);
        temp.created = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);

        set_corba_seq<StringSeq, CORBA::String_var,
            std::vector<Fred::ObjectIdHandlePair>, Fred::ObjectIdHandlePair>(temp.tech_contact_handles, in.tech_contacts);

        {
            std::vector<std::string> statuses;
            {
                Fred::OperationContext ctx;

                BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(in.id).exec(ctx)) {
                    if(state.is_external) {
                        statuses.push_back(state.state_name);
                    }
                }
            }

            set_corba_seq<StringSeq, CORBA::String_var,
                std::vector<std::string>, std::string>(temp.statuses, statuses);
        }

        set_corba_seq<DNSKeySeq, DNSKey,
            std::vector<Fred::DnsKey>, Fred::DnsKey>(temp.dns_keys, in.dns_keys);

        return temp;
    }

    template<> KeySet set_element_of_corba_seq<KeySet,  Fred::InfoKeysetOutput>
        (const Fred::InfoKeysetOutput& ile)
    {
        return wrap_keyset(ile.info_keyset_data);
    }


    NSSet wrap_nsset(const Fred::InfoNssetData& in)
    {
        NSSet temp;

        temp.handle = Corba::wrap_string_to_corba_string(in.handle);
        temp.registrar_handle = Corba::wrap_string_to_corba_string(in.sponsoring_registrar_handle);
        temp.created = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);

        set_corba_seq<NameServerSeq, NameServer,
            std::vector<Fred::DnsHost>, Fred::DnsHost>(temp.nservers, in.dns_hosts);

        set_corba_seq<StringSeq, CORBA::String_var,
            std::vector<Fred::ObjectIdHandlePair>, Fred::ObjectIdHandlePair>(temp.tech_contact_handles, in.tech_contacts);

        {
            std::vector<std::string> statuses;
            {
                Fred::OperationContext ctx;

                BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(in.id).exec(ctx)) {
                    if(state.is_external) {
                        statuses.push_back(state.state_name);
                    }
                }
            }

            set_corba_seq<StringSeq, CORBA::String_var,
                std::vector<std::string>, std::string>(temp.statuses, statuses);
        }

        return temp;
    }

    template<> NSSet set_element_of_corba_seq<NSSet,  Fred::InfoNssetOutput>
        (const Fred::InfoNssetOutput& ile)
    {
        return wrap_nsset(ile.info_nsset_data);
    }

    NullableRegistrar* Server_impl::get_registrar_by_handle(const char* handle) {
        try {
            Fred::OperationContext ctx;

            return
                wrap_registrar(
                    Fred::InfoRegistrarByHandle(
                        Corba::unwrap_string(handle)
                    ).exec(ctx, output_timezone)
                    .info_registrar_data
                );

        } catch(const Fred::InfoRegistrarByHandle::Exception& e) {
            if(e.is_set_unknown_registrar_handle()) {
                return NULL;
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NullableContact* Server_impl::get_contact_by_handle(const char* handle) {
        try {
            Fred::OperationContext ctx;

            return
                wrap_contact(
                    Fred::InfoContactByHandle(
                        Corba::unwrap_string(handle)
                    ).exec(ctx, output_timezone)
                    .info_contact_data
                );

        } catch(const Fred::InfoContactByHandle::Exception& e) {
            if(e.is_set_unknown_contact_handle()) {
                return NULL;
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NullableNSSet* Server_impl::get_nsset_by_handle(const char* handle) {
        try {
            Fred::OperationContext ctx;

            return Corba::wrap_nullable_corba_type_to_corba_valuetype<NullableNSSet>(
                Nullable<NSSet>(wrap_nsset(Fred::InfoNssetByHandle(Corba::unwrap_string(handle)
                    ).exec(ctx, output_timezone).info_nsset_data))
                )._retn();

        } catch(const Fred::InfoNssetByHandle::Exception& e) {
            if(e.is_set_unknown_handle()) {
                return NULL;
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NSSetSeq* Server_impl::get_nssets_by_ns(const char* handle)
    {
        try
        {
            Fred::OperationContext ctx;
            NSSetSeq_var nss_seq = new NSSetSeq;
            try
            {
                std::vector<Fred::InfoNssetOutput> nss_info = Fred::InfoNssetByDNSFqdn(
                    Corba::unwrap_string(handle)).exec(ctx, output_timezone);

                set_corba_seq<NSSetSeq, NSSet,std::vector<Fred::InfoNssetOutput>, Fred::InfoNssetOutput>
                    (nss_seq.inout(), nss_info);

                return nss_seq._retn();
            }
            catch(const Fred::InfoNssetByDNSFqdn::Exception& e)
            {
                    if(e.is_set_unknown_fqdn()) {
                        return nss_seq._retn();
                    }
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NSSetSeq* Server_impl::get_nssets_by_tech_c(const char* handle)
    {
        try
        {
            Fred::OperationContext ctx;
            NSSetSeq_var nss_seq = new NSSetSeq;

            std::vector<Fred::InfoNssetOutput> nss_info = Fred::InfoNssetByTechContactHandle(
                Corba::unwrap_string(handle)).exec(ctx, output_timezone);

            set_corba_seq<NSSetSeq, NSSet,std::vector<Fred::InfoNssetOutput>, Fred::InfoNssetOutput>
                (nss_seq.inout(), nss_info);

            return nss_seq._retn();
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }


    NullableNameServer* Server_impl::get_nameserver_by_fqdn(const char* fqdn) {
        try {
            Fred::OperationContext ctx;

            if(::Whois::nameserver_exists(
                Corba::unwrap_string(fqdn),
                ctx)
            ) {
                NameServer temp;
                temp.fqdn = Corba::wrap_string(Corba::unwrap_string(fqdn));
                return new NullableNameServer(temp);
            } else {
                return NULL;
            }

        } catch(...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NullableKeySet* Server_impl::get_keyset_by_handle(const char* handle)
    {
        try {
            Fred::OperationContext ctx;

            return Corba::wrap_nullable_corba_type_to_corba_valuetype<NullableKeySet>(
                Nullable<KeySet>(wrap_keyset(Fred::InfoKeysetByHandle(Corba::unwrap_string(handle)
                    ).exec(ctx, output_timezone).info_keyset_data))
                )._retn();

        } catch(const Fred::InfoKeysetByHandle::Exception& e) {
            if(e.is_set_unknown_handle()) {
                return NULL;
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    KeySetSeq* Server_impl::get_keysets_by_tech_c(const char* handle)
    {
        try
        {
            Fred::OperationContext ctx;

            KeySetSeq_var ks_seq = new KeySetSeq;

            std::vector<Fred::InfoKeysetOutput> ks_info = Fred::InfoKeysetByTechContactHandle(
                Corba::unwrap_string(handle)).exec(ctx, output_timezone);

            set_corba_seq<KeySetSeq, KeySet,std::vector<Fred::InfoKeysetOutput>, Fred::InfoKeysetOutput>
                (ks_seq.inout(), ks_info);

            return ks_seq._retn();
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NullableDomain* Server_impl::get_domain_by_handle(const char* handle) {
        try {
            Fred::OperationContext ctx;

            if(::Whois::is_domain_delete_pending(Corba::unwrap_string(handle), ctx, "Europe/Prague")) {
                return generate_obfuscate_domain_delete_candidate(Corba::unwrap_string(handle), ctx);
            }

            return
                wrap_domain(
                    Fred::InfoDomainByHandle(
                        Corba::unwrap_string(handle)
                    ).exec(ctx, output_timezone)
                    .info_domain_data
                );

        } catch(const Fred::InfoDomainByHandle::Exception& e) {
            if(e.is_set_unknown_fqdn()) {
                return NULL;
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    // TODO XXX Just to have RDAP prototype quickly deployable (Ticket #10627). Must be implemented later.
    DomainSeq* Server_impl::get_domains_by_registrant(const char* handle) { return NULL; }
    DomainSeq* Server_impl::get_domains_by_admin_contact(const char* handle) { return NULL; }
    DomainSeq* Server_impl::get_domains_by_nsset(const char* handle) { return NULL; }
    DomainSeq* Server_impl::get_domains_by_keyset(const char* handle) { return NULL; }

    // TODO XXX Just to have RDAP prototype quickly deployable (Ticket #10627). Must be implemented later.
    ObjectStatusDescSeq* Server_impl::get_domain_status_descriptions(const char* lang) { return NULL; }
    ObjectStatusDescSeq* Server_impl::get_contact_status_descriptions(const char* lang) { return NULL; }
    ObjectStatusDescSeq* Server_impl::get_nsset_status_descriptions(const char* lang) { return NULL; }
    ObjectStatusDescSeq* Server_impl::get_keyset_status_descriptions(const char* lang) { return NULL; }
}
}
