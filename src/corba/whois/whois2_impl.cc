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

    void wrap_ipaddress_sequence(const std::vector<boost::asio::ip::address>& in, IPAddressSeq& out ) {
        out.length(in.size());

        typename std::vector<boost::asio::ip::address>::size_type i = 0;

        BOOST_FOREACH(const boost::asio::ip::address& address, in) {
            wrap_ipaddress(address, out[i]);
            ++i;
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

    NullableKeySet* wrap_keyset(const Fred::InfoKeysetData& in) {

        KeySet temp;

        temp.handle = Corba::wrap_string(in.handle);
        temp.registrar_handle = Corba::wrap_string(in.sponsoring_registrar_handle);
        temp.created = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);
        {
            std::vector<std::string> tech_contacts;
            {
                BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_contact, in.tech_contacts) {
                    tech_contacts.push_back(tech_contact.handle);
                }
            }
            wrap_string_sequence(tech_contacts, temp.tech_contact_handles);
        }
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
            wrap_string_sequence(statuses, temp.statuses);
        }

        temp.dns_keys.length(in.dns_keys.size());

        unsigned i = 0;
        BOOST_FOREACH(const Fred::DnsKey& key, in.dns_keys) {
            temp.dns_keys[i].flags = key.get_flags();
            temp.dns_keys[i].protocol = key.get_protocol();
            temp.dns_keys[i].alg = key.get_alg();
            temp.dns_keys[i].public_key = Corba::wrap_string(key.get_key());

            ++i;
        }

        return new NullableKeySet(temp);
    }

    NullableNameServer*  wrap_nameserver( const Fred::DnsHost& in) {
        NameServer temp;
        temp.fqdn = Corba::wrap_string(in.get_fqdn());
        wrap_ipaddress_sequence(in.get_inet_addr(), temp.ip_addresses);

        return new NullableNameServer(temp);
    }

    NullableNSSet* wrap_nsset(const Fred::InfoNssetData& in) {

        NSSet temp;

        temp.handle = Corba::wrap_string(in.handle);
        temp.registrar_handle = Corba::wrap_string(in.sponsoring_registrar_handle);
        temp.created = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);

        {
            temp.nservers.length(in.dns_hosts.size());

            unsigned long i = 0;
            BOOST_FOREACH(const Fred::DnsHost& nsserver, in.dns_hosts) {
                temp.nservers[i] = wrap_nameserver(nsserver)->_value();
                ++i;
            }
        }

        {
            std::vector<std::string> tech_contacts;
            {
                BOOST_FOREACH(const Fred::ObjectIdHandlePair& tech_contact, in.tech_contacts) {
                    tech_contacts.push_back(tech_contact.handle);
                }
            }
            wrap_string_sequence(tech_contacts, temp.tech_contact_handles);
        }

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
            wrap_string_sequence(statuses, temp.statuses);
        }

        return new NullableNSSet(temp);
    }

    NSSetSeq* wrap_nsset_vector(const std::vector<Fred::InfoNssetOutput>& in) {
        NSSetSeq_var result(new NSSetSeq);

        result->length(in.size());

        long i = 0;
        for(std::vector<Fred::InfoNssetOutput>::const_iterator it = in.begin();
            it != in.end();
            ++it, ++i
        ) {
            result[i] = wrap_nsset(it->info_nsset_data)->_value();
        }

        return result._retn();
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

            return
                wrap_nsset(
                    Fred::InfoNssetByHandle(
                        Corba::unwrap_string(handle)
                    ).exec(ctx, output_timezone)
                    .info_nsset_data
                );

        } catch(const Fred::InfoNssetByHandle::Exception& e) {
            if(e.is_set_unknown_handle()) {
                return NULL;
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    // TODO XXX Just to have RDAP prototype quickly deployable (Ticket #10627). Must be implemented later.
    NSSetSeq* Server_impl::get_nssets_by_ns(const char* handle) { return NULL; }
    NSSetSeq* Server_impl::get_nssets_by_tech_c(const char* handle) { return NULL; }

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

    NullableKeySet* Server_impl::get_keyset_by_handle(const char* handle) {
        try {
            Fred::OperationContext ctx;

            return
                wrap_keyset(
                    Fred::InfoKeysetByHandle(
                        Corba::unwrap_string(handle)
                    ).exec(ctx, output_timezone)
                    .info_keyset_data
                );

        } catch(const Fred::InfoKeysetByHandle::Exception& e) {
            if(e.is_set_unknown_handle()) {
                return NULL;
            }
        } catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    // TODO XXX Just to have RDAP prototype quickly deployable (Ticket #10627). Must be implemented later.
    KeySetSeq* Server_impl::get_keysets_by_tech_c(const char* handle) { return NULL; }

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
