#include "util/random.h"

#include "src/corba/whois/whois2_impl.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_datetime.h"
#include "src/corba/util/corba_conversions_nullable_types.h"

#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/check_registrar.h"
#include "src/fredlib/registrar/get_registrar_handles.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/zone/zone.h"

#include "src/whois/nameserver_exists.h"
#include "src/whois/is_domain_delete_pending.h"
#include "src/whois/registrar_group.h"
#include "src/whois/registrar_certification.h"
#include "src/whois/zone_list.h"

#include <boost/foreach.hpp>
#include <boost/asio.hpp>

#include <stdexcept>

namespace Registry {
namespace Whois {


    std::string create_ctx_invocation_id()
    {
        return str(boost::format("fred-pifd-<%1%>") % Random::integer(0, 10000));
    }

    std::string create_ctx_function_name(const char *fnc)
    {
        if (fnc == NULL)
        {
            return std::string();
        }
        std::string name(fnc);
        std::replace(name.begin(), name.end(), '_', '-');
        return name;
    }

    class LogContext
    {
    public:
        LogContext(const std::string &_op_name)
        :   ctx_server_(create_ctx_invocation_id()),
            ctx_operation_(_op_name)
        {
        }
    private:
        Logging::Context ctx_server_;
        Logging::Context ctx_operation_;
    };

    #define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(create_ctx_function_name(__FUNCTION__))



    const std::string Server_impl::output_timezone("UTC");

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

    PlaceAddress wrap_registrar_address(const Fred::InfoRegistrarData& in)
    {
        PlaceAddress result;
        result.street1 = Corba::wrap_string_to_corba_string(in.street1.get_value_or_default());
        result.street2 = Corba::wrap_string_to_corba_string(in.street2.get_value_or_default());
        result.street3 = Corba::wrap_string_to_corba_string(in.street3.get_value_or_default());
        result.city = Corba::wrap_string_to_corba_string(in.city.get_value_or_default());
        result.postalcode = Corba::wrap_string_to_corba_string(in.postalcode.get_value_or_default());
        result.stateorprovince = Corba::wrap_string_to_corba_string(in.stateorprovince.get_value_or_default());
        result.country_code = Corba::wrap_string_to_corba_string(in.country.get_value_or_default());
        return result;
    }

   Registrar wrap_registrar(const Fred::InfoRegistrarData& in)
   {
        Registrar temp;
        temp.handle = Corba::wrap_string_to_corba_string(in.handle);
        temp.name = Corba::wrap_string_to_corba_string(in.name.get_value_or_default());
        temp.organization = Corba::wrap_string_to_corba_string(in.organization.get_value_or_default());
        temp.url = Corba::wrap_string_to_corba_string(in.url.get_value_or_default());
        temp.phone = Corba::wrap_string_to_corba_string(in.telephone.get_value_or_default());
        temp.fax = Corba::wrap_string_to_corba_string(in.fax.get_value_or_default());
        temp.address = wrap_registrar_address(in);
        return temp;
    }

   DisclosableString wrap_disclosable_string(const std::string& str, bool disclose)
   {
       DisclosableString temp;
       temp.value = Corba::wrap_string_to_corba_string(str);
       temp.disclose = disclose;
       return temp;
   }

   DisclosablePlaceAddress wrap_disclosable_address(const Fred::Contact::PlaceAddress& addr, bool disclose)
   {
       DisclosablePlaceAddress temp;
       temp.value.street1 = Corba::wrap_string_to_corba_string(addr.street1);
       temp.value.street2 = Corba::wrap_string_to_corba_string(addr.street2.get_value_or_default());
       temp.value.street3 = Corba::wrap_string_to_corba_string(addr.street3.get_value_or_default());
       temp.value.city = Corba::wrap_string_to_corba_string(addr.city);
       temp.value.stateorprovince = Corba::wrap_string_to_corba_string(addr.stateorprovince.get_value_or_default());
       temp.value.postalcode = Corba::wrap_string_to_corba_string(addr.postalcode);
       temp.value.country_code = Corba::wrap_string_to_corba_string(addr.country);
       temp.disclose = disclose;
       return temp;
   }

   DisclosableContactIdentification wrap_disclosable_contact_identification(
       const std::string& ssntype, const std::string& ssn, bool disclose)
   {
       DisclosableContactIdentification temp;
       temp.value.identification_type = Corba::wrap_string_to_corba_string(ssntype);
       temp.value.identification_data = Corba::wrap_string_to_corba_string(ssn);
       temp.disclose = disclose;
       return temp;
   }


    /**
     * CORBA sequence element factory, template to be specialized, there is no generic enough implementation
     */
    template<class CORBA_SEQ_ELEMENT, class IN_LIST_ELEMENT>
        CORBA_SEQ_ELEMENT set_element_of_corba_seq(const IN_LIST_ELEMENT& ile);

    /**
     * generic implementation of allocation and setting CORBA sequence
     * from container with begin(), end(), size() and value_type member
     */
    template<class CORBA_SEQ, class CORBA_SEQ_ELEMENT,
        class IN_LIST>
    void set_corba_seq(CORBA_SEQ& cs, const IN_LIST& il)
    {
        cs.length(il.size());
        unsigned long long i = 0;
        for(typename IN_LIST::const_iterator ci = il.begin() ; ci != il.end(); ++ci,++i)
        {
            cs[i] = set_element_of_corba_seq<CORBA_SEQ_ELEMENT, typename IN_LIST::value_type >(*ci);
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
        set_corba_seq<IPAddressSeq, IPAddress>(ns.ip_addresses, ile.get_inet_addr());
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

    template<> Registrar set_element_of_corba_seq<
    Registrar, Fred::InfoRegistrarData>(const Fred::InfoRegistrarData& ile)
    {
        return wrap_registrar(ile);
    }


    RegistrarGroup wrap_registrar_group(const std::pair<const std::string, std::vector<std::string> >& in)
    {
        RegistrarGroup temp;
         temp.name = Corba::wrap_string_to_corba_string(in.first);

         set_corba_seq<RegistrarHandleList, CORBA::String_var>(temp.members, in.second);

         return temp;
     }

    template<> RegistrarGroup set_element_of_corba_seq<
    RegistrarGroup, std::pair<const std::string, std::vector<std::string> > >(const std::pair<const std::string, std::vector<std::string> >& ile)
    {
        return wrap_registrar_group(ile);
    }


    RegistrarCertification wrap_registrar_certification(const ::Whois::RegistrarCertificationData& in)
    {
        RegistrarCertification temp;
         temp.registrar_handle = Corba::wrap_string_to_corba_string(in.get_registrar_handle());
         temp.score = in.get_registrar_score();
         temp.evaluation_file_id = in.get_registrar_evaluation_file_id();
         return temp;
     }

    template<> RegistrarCertification set_element_of_corba_seq<
    RegistrarCertification, ::Whois::RegistrarCertificationData >(const ::Whois::RegistrarCertificationData& ile)
    {
        return wrap_registrar_certification(ile);
    }


    void wrap_object_states(StringSeq& states_seq, unsigned long long object_id)
    {
        std::vector<std::string> statuses;
        {
            Fred::OperationContext ctx;

            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(object_id).exec(ctx)) {
                if(state.is_external) {
                    statuses.push_back(state.state_name);
                }
            }
        }

        set_corba_seq<StringSeq, CORBA::String_var>(states_seq, statuses);
    }

    Contact wrap_contact(const Fred::InfoContactData& in)
    {

        Contact temp;

        temp.handle = Corba::wrap_string_to_corba_string(in.handle);
        temp.organization = wrap_disclosable_string(in.organization.get_value_or_default(), in.discloseaddress);
        temp.name = wrap_disclosable_string(in.name.get_value_or_default(), in.disclosename);
        temp.address = wrap_disclosable_address(in.place.get_value_or_default(), in.discloseaddress);
        temp.phone = wrap_disclosable_string(in.telephone.get_value_or_default(), in.disclosetelephone);
        temp.fax = wrap_disclosable_string(in.fax.get_value_or_default(), in.disclosefax);;
        temp.email = wrap_disclosable_string(in.email.get_value_or_default(), in.discloseemail);
        temp.notify_email = wrap_disclosable_string(in.notifyemail.get_value_or_default(), in.disclosenotifyemail);
        temp.vat_number = wrap_disclosable_string(in.vat.get_value_or_default(), in.disclosevat);
        temp.identification = wrap_disclosable_contact_identification(
            in.ssntype.get_value_or_default(), in.ssn.get_value_or_default(), in.discloseident);
        temp.creating_registrar_handle = Corba::wrap_string_to_corba_string(in.create_registrar_handle);
        temp.sponsoring_registrar_handle = Corba::wrap_string_to_corba_string(in.sponsoring_registrar_handle);
        temp.created = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);

        wrap_object_states(temp.statuses, in.id);

        return temp;
    }

    Domain generate_obfuscate_domain_delete_candidate(const std::string& _handle)
    {
        Domain temp;
        temp.handle = Corba::wrap_string_to_corba_string(_handle);
        temp.registrant_handle = Corba::wrap_string_to_corba_string("");
        temp.nsset_handle = NULL;
        temp.keyset_handle = NULL;
        temp.registrar_handle = Corba::wrap_string_to_corba_string("");
        temp.registered = Corba::wrap_time(boost::posix_time::ptime());
        temp.changed = Corba::wrap_nullable_datetime(Nullable<boost::posix_time::ptime>());
        temp.last_transfer = Corba::wrap_nullable_datetime(Nullable<boost::posix_time::ptime>());
        temp.expire = Corba::wrap_date(boost::gregorian::date());
        temp.validated_to = NULL;
        {
            std::vector<std::string> statuses;
            statuses.push_back("deleteCandidate");
            set_corba_seq<StringSeq, CORBA::String_var>(temp.statuses, statuses);
        }
        return temp;
    }

    Domain wrap_domain(const Fred::InfoDomainData& in)
    {
        Domain temp;
        temp.handle = Corba::wrap_string_to_corba_string(in.fqdn);
        temp.registrant_handle = Corba::wrap_string_to_corba_string(in.registrant.handle);
        if( in.nsset.isnull() ) {
            temp.nsset_handle = NULL;
        } else {
            temp.nsset_handle = new NullableString(Corba::wrap_string_to_corba_string(in.nsset.get_value().handle));
        }
        if( in.keyset.isnull() ) {
            temp.keyset_handle = NULL;
        } else {
            temp.keyset_handle = new NullableString(Corba::wrap_string_to_corba_string(in.keyset.get_value().handle));
        }
        temp.registrar_handle = Corba::wrap_string_to_corba_string(in.sponsoring_registrar_handle);
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
        set_corba_seq<StringSeq, CORBA::String_var>(temp.admin_contact_handles, in.admin_contacts);

        wrap_object_states(temp.statuses, in.id);
        return temp;
    }

    class DomainInfoWithDeleteCandidate
    {
        Fred::InfoDomainOutput info;
        bool delete_candidate;

    public:

        DomainInfoWithDeleteCandidate(const Fred::InfoDomainOutput& _info, bool _delete_candidate)
        : info(_info)
        , delete_candidate(_delete_candidate)
        {}

        Fred::InfoDomainOutput get_info() const
        {
            return info;
        }

        bool get_delete_candidate() const
        {
            return delete_candidate;
        }
    };

    template<> Domain set_element_of_corba_seq<Domain,  DomainInfoWithDeleteCandidate>
        (const DomainInfoWithDeleteCandidate& ile)
    {
        if(ile.get_delete_candidate())
        {
            return generate_obfuscate_domain_delete_candidate(ile.get_info().info_domain_data.fqdn);
        }
        else
        {
            return wrap_domain(ile.get_info().info_domain_data);
        }
    }


    KeySet wrap_keyset(const Fred::InfoKeysetData& in)
    {
        KeySet temp;

        temp.handle = Corba::wrap_string_to_corba_string(in.handle);
        temp.registrar_handle = Corba::wrap_string_to_corba_string(in.sponsoring_registrar_handle);
        temp.created = Corba::wrap_time(in.creation_time);
        temp.changed = Corba::wrap_nullable_datetime(in.update_time);
        temp.last_transfer = Corba::wrap_nullable_datetime(in.transfer_time);

        set_corba_seq<StringSeq, CORBA::String_var>(temp.tech_contact_handles, in.tech_contacts);

        wrap_object_states(temp.statuses, in.id);

        set_corba_seq<DNSKeySeq, DNSKey>(temp.dns_keys, in.dns_keys);

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

        set_corba_seq<NameServerSeq, NameServer>(temp.nservers, in.dns_hosts);

        set_corba_seq<StringSeq, CORBA::String_var>(temp.tech_contact_handles, in.tech_contacts);

        wrap_object_states(temp.statuses, in.id);
        return temp;
    }

    template<> NSSet set_element_of_corba_seq<NSSet,  Fred::InfoNssetOutput>
        (const Fred::InfoNssetOutput& ile)
    {
        return wrap_nsset(ile.info_nsset_data);
    }

    Registrar* Server_impl::get_registrar_by_handle(const char* handle)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);
            try
            {
                Fred::OperationContext ctx;
                return new Registrar(wrap_registrar(
                        Fred::InfoRegistrarByHandle(
                            Corba::unwrap_string_from_const_char_ptr(handle)
                        ).exec(ctx, output_timezone)
                        .info_registrar_data));
            }
            catch(const Fred::InfoRegistrarByHandle::Exception& e)
            {
                if(e.is_set_unknown_registrar_handle())
                {
                    if(Fred::CheckRegistrar(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                    {
                        throw INVALID_HANDLE();
                    }

                    throw OBJECT_NOT_FOUND();
                }
            }
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }


    RegistrarSeq* Server_impl::get_registrars()
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            RegistrarSeq_var registrar_seq = new RegistrarSeq;
            std::vector<Fred::InfoRegistrarData> registrar_data_list;
            Fred::OperationContext ctx;

            BOOST_FOREACH(const Fred::InfoRegistrarOutput& registrar_output, Fred::InfoRegistrarAllExceptSystem().exec(ctx, output_timezone))
            {
                registrar_data_list.push_back(registrar_output.info_registrar_data);
            }

            set_corba_seq<RegistrarSeq, Registrar>(
                    registrar_seq, registrar_data_list);

            return registrar_seq._retn();
        }
        catch (...)
        {}

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    RegistrarGroupList* Server_impl::get_registrar_groups()
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            RegistrarGroupList_var reg_grp_seq = new RegistrarGroupList;
            Fred::OperationContext ctx;

            set_corba_seq<RegistrarGroupList, RegistrarGroup>(
                reg_grp_seq, ::Whois::get_registrar_groups(ctx));

            return reg_grp_seq._retn();
        }
        catch (...)
        {}

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    RegistrarCertificationList* Server_impl::get_registrar_certification_list()
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            RegistrarCertificationList_var reg_cert_seq = new RegistrarCertificationList;
            Fred::OperationContext ctx;
            set_corba_seq<RegistrarCertificationList, RegistrarCertification>
                (reg_cert_seq, ::Whois::get_registrar_certifications(ctx));
            return reg_cert_seq._retn();
        }
        catch (...)
        {}

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    ZoneFqdnList* Server_impl::get_managed_zone_list()
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            ZoneFqdnList_var zone_seq = new ZoneFqdnList;
            Fred::OperationContext ctx;
            set_corba_seq<ZoneFqdnList, CORBA::String_var>
                (zone_seq, ::Whois::get_managed_zone_list(ctx));
            return zone_seq._retn();
        }
        catch (...)
        {}

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }


    Contact* Server_impl::get_contact_by_handle(const char* handle)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);
            try
            {
                Fred::OperationContext ctx;
                return new Contact(
                    wrap_contact(
                        Fred::InfoContactByHandle(
                            Corba::unwrap_string_from_const_char_ptr(handle)
                        ).exec(ctx, output_timezone)
                        .info_contact_data
                    ));

            }
            catch(const Fred::InfoContactByHandle::Exception& e)
            {
                if(e.is_set_unknown_contact_handle())
                {
                    if(Fred::CheckContact(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                    {
                        throw INVALID_HANDLE();
                    }

                    throw OBJECT_NOT_FOUND();
                }
            }
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...)
        {}

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NSSet* Server_impl::get_nsset_by_handle(const char* handle)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);
            try
            {
                Fred::OperationContext ctx;
                return new NSSet(wrap_nsset(Fred::InfoNssetByHandle(
                    Corba::unwrap_string_from_const_char_ptr(handle)
                        ).exec(ctx, output_timezone).info_nsset_data));
            }
            catch(const Fred::InfoNssetByHandle::Exception& e)
            {
                if(e.is_set_unknown_handle())
                {
                    if(Fred::CheckNsset(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                    {
                        throw INVALID_HANDLE();
                    }

                    throw OBJECT_NOT_FOUND();
                }
            }
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...)
        { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NSSetSeq* Server_impl::get_nssets_by_ns(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            const std::string fqdn = Fred::Zone::rem_trailing_dot(
                Corba::unwrap_string_from_const_char_ptr(handle)
            );

            Fred::OperationContext ctx;
            NSSetSeq_var nss_seq = new NSSetSeq;

            std::vector<Fred::InfoNssetOutput> nss_info = Fred::InfoNssetByDNSFqdn(fqdn)
                .set_limit(limit + 1)
                .exec(ctx, output_timezone);

            if(nss_info.empty())
            {
                if(Fred::CheckDomain(fqdn).is_invalid_syntax())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }

            limit_exceeded = false;
            if(nss_info.size() > limit)
            {
                limit_exceeded = true;
                nss_info.erase(nss_info.begin());//depends on InfoNsset ordering
            }

            set_corba_seq<NSSetSeq, NSSet>(nss_seq.inout(), nss_info);
            return nss_seq._retn();
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    NSSetSeq* Server_impl::get_nssets_by_tech_c(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            Fred::OperationContext ctx;
            NSSetSeq_var nss_seq = new NSSetSeq;

            std::vector<Fred::InfoNssetOutput> nss_info = Fred::InfoNssetByTechContactHandle(
                Corba::unwrap_string_from_const_char_ptr(handle)).set_limit(limit + 1)
                    .exec(ctx, output_timezone);

            if(nss_info.empty())
            {
                if(Fred::CheckContact(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }

            limit_exceeded = false;
            if(nss_info.size() > limit)
            {
                limit_exceeded = true;
                nss_info.erase(nss_info.begin());//depends on InfoNsset ordering
            }

            set_corba_seq<NSSetSeq, NSSet>(nss_seq.inout(), nss_info);

            return nss_seq._retn();
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }


    NameServer* Server_impl::get_nameserver_by_fqdn(const char* fqdn)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            const std::string ns_fqdn = Fred::Zone::rem_trailing_dot(
                Corba::unwrap_string_from_const_char_ptr(fqdn)
            );

            Fred::OperationContext ctx;

            if(::Whois::nameserver_exists(ns_fqdn, ctx))
            {
                NameServer temp;
                temp.fqdn = Corba::wrap_string_to_corba_string(ns_fqdn);
                /*
                 * Because of grouping nameservers NSSet we don't include
                 * IP address in output (given nameserver can be in different
                 * NSSets with different IP addresses)
                 *
                 * temp.ip_addresses;
                 */
                return new NameServer(temp);
            }
            else
            {
                if(Fred::CheckDomain(ns_fqdn).is_invalid_syntax())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch(...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    KeySet* Server_impl::get_keyset_by_handle(const char* handle)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);
            try
            {
                Fred::OperationContext ctx;

                return new KeySet(wrap_keyset(Fred::InfoKeysetByHandle(
                    Corba::unwrap_string_from_const_char_ptr(handle)
                    ).exec(ctx, output_timezone).info_keyset_data));

            }
            catch(const Fred::InfoKeysetByHandle::Exception& e)
            {
                if(e.is_set_unknown_handle())
                {
                    if(Fred::CheckKeyset(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                    {
                        throw INVALID_HANDLE();
                    }

                    throw OBJECT_NOT_FOUND();
                }
            }
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    KeySetSeq* Server_impl::get_keysets_by_tech_c(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            Fred::OperationContext ctx;
            KeySetSeq_var ks_seq = new KeySetSeq;

            std::vector<Fred::InfoKeysetOutput> ks_info = Fred::InfoKeysetByTechContactHandle(
                Corba::unwrap_string_from_const_char_ptr(handle)
                ).set_limit(limit + 1).exec(ctx, output_timezone);

            if(ks_info.empty())
            {
                if(Fred::CheckContact(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }

            limit_exceeded = false;
            if(ks_info.size() > limit)
            {
                limit_exceeded = true;
                ks_info.erase(ks_info.begin());//depends on InfoKeyset ordering
            }

            set_corba_seq<KeySetSeq, KeySet>(ks_seq.inout(), ks_info);

            return ks_seq._retn();
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    Domain* Server_impl::get_domain_by_handle(const char* handle)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            const std::string fqdn = Fred::Zone::rem_trailing_dot(
                Corba::unwrap_string_from_const_char_ptr(handle)
            );

            Fred::OperationContext ctx;
            try
            {
                //check general name rules
                if(Fred::CheckDomain(fqdn).is_invalid_syntax())
                {
                    throw INVALID_LABEL();
                }

                if(Fred::CheckDomain(fqdn).is_bad_zone(ctx))
                {
                    throw UNMANAGED_ZONE();
                }

                if(Fred::CheckDomain(fqdn).is_bad_length(ctx))
                {
                    throw TOO_MANY_LABELS();
                }

                Domain tmp_domain(wrap_domain(
                    Fred::InfoDomainByHandle(fqdn).exec(ctx, output_timezone).info_domain_data
                ));

                if(::Whois::is_domain_delete_pending(fqdn, ctx, "Europe/Prague"))
                {
                    return new Domain(generate_obfuscate_domain_delete_candidate(fqdn));
                }

                return new Domain(tmp_domain);

            }
            catch(const Fred::InfoDomainByHandle::Exception& e)
            {
                if(e.is_set_unknown_fqdn())
                {
                    //check current registry name rules (that might change over time)
                    if(Fred::CheckDomain(fqdn).is_invalid_handle(ctx))
                    {
                        throw INVALID_LABEL();
                    }

                    throw OBJECT_NOT_FOUND();
                }
            }
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    /**
     * get_domains_by_* implementation of allocation and setting CORBA sequence
     */
    void set_domains_seq(DomainSeq& domain_seq, const std::vector<Fred::InfoDomainOutput>& il, Fred::OperationContext& ctx)
    {
        std::vector<DomainInfoWithDeleteCandidate> didclist;
        didclist.reserve(il.size());

        BOOST_FOREACH(const Fred::InfoDomainOutput& i, il)
        {
            didclist.push_back(DomainInfoWithDeleteCandidate(i,
                ::Whois::is_domain_delete_pending(i.info_domain_data.fqdn, ctx, "Europe/Prague")));
        }

        set_corba_seq<DomainSeq, Domain>(domain_seq, didclist);
    }

    DomainSeq* Server_impl::get_domains_by_registrant(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            Fred::OperationContext ctx;
            DomainSeq_var domain_seq = new DomainSeq;

            std::vector<Fred::InfoDomainOutput> domain_info = Fred::InfoDomainByRegistrantHandle(
                Corba::unwrap_string_from_const_char_ptr(handle)
            ).set_limit(limit + 1).exec(ctx, output_timezone);

            if(domain_info.empty())
            {
                if(Fred::CheckContact(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }

            limit_exceeded = false;
            if(domain_info.size() > limit)
            {
                limit_exceeded = true;
                domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
            }

            set_domains_seq(domain_seq.inout(),domain_info,ctx);

            return domain_seq._retn();
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    DomainSeq* Server_impl::get_domains_by_admin_contact(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            Fred::OperationContext ctx;
            DomainSeq_var domain_seq = new DomainSeq;

            std::vector<Fred::InfoDomainOutput> domain_info = Fred::InfoDomainByAdminContactHandle(
                Corba::unwrap_string_from_const_char_ptr(handle)).set_limit(limit + 1).exec(ctx, output_timezone);

            if(domain_info.empty())
            {
                if(Fred::CheckContact(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }

            limit_exceeded = false;
            if(domain_info.size() > limit)
            {
                limit_exceeded = true;
                domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
            }

            set_domains_seq(domain_seq.inout(),domain_info,ctx);

            return domain_seq._retn();
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    DomainSeq* Server_impl::get_domains_by_nsset(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            Fred::OperationContext ctx;
            DomainSeq_var domain_seq = new DomainSeq;

            std::vector<Fred::InfoDomainOutput> domain_info = Fred::InfoDomainByNssetHandle(
                Corba::unwrap_string_from_const_char_ptr(handle)
            ).set_limit(limit + 1).exec(ctx, output_timezone);

            if(domain_info.empty())
            {
                if(Fred::CheckNsset(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }

            limit_exceeded = false;
            if(domain_info.size() > limit)
            {
                limit_exceeded = true;
                domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
            }

            set_domains_seq(domain_seq.inout(),domain_info,ctx);

            return domain_seq._retn();
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    DomainSeq* Server_impl::get_domains_by_keyset(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            Fred::OperationContext ctx;
            DomainSeq_var domain_seq = new DomainSeq;

            std::vector<Fred::InfoDomainOutput> domain_info = Fred::InfoDomainByKeysetHandle(
                Corba::unwrap_string_from_const_char_ptr(handle)).set_limit(limit + 1).exec(ctx, output_timezone);

            if(domain_info.empty())
            {
                if(Fred::CheckKeyset(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_handle())
                {
                    throw INVALID_HANDLE();
                }

                throw OBJECT_NOT_FOUND();
            }

            limit_exceeded = false;
            if(domain_info.size() > limit)
            {
                limit_exceeded = true;
                domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
            }

            set_domains_seq(domain_seq.inout(),domain_info,ctx);

            return domain_seq._retn();
        }
        catch(const ::CORBA::UserException& )
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    std::vector< std::pair<std::string, std::string> > get_object_status_desc(
        const std::string& lang,const std::string& type, Fred::OperationContext& ctx)
    {
        std::vector<Fred::ObjectStateDescription> states = Fred::GetObjectStateDescriptions(
            lang).set_object_type(type).set_external().exec(ctx);

        if(states.empty()) throw MISSING_LOCALIZATION();

        std::vector< std::pair<std::string, std::string> > temp;
        for(std::vector<Fred::ObjectStateDescription>::const_iterator ci
                = states.begin(); ci != states.end(); ++ci)
        {
            temp.push_back(std::make_pair(ci->handle, ci->description));
        }

        return temp;
    }

    template<> ObjectStatusDesc set_element_of_corba_seq<
    ObjectStatusDesc, std::pair<std::string, std::string> >(const std::pair<std::string, std::string>& ile)
    {
        ObjectStatusDesc temp;
        temp.handle = Corba::wrap_string_to_corba_string(ile.first);
        temp.name = Corba::wrap_string_to_corba_string(ile.second);
        return temp;
    }

    ObjectStatusDescSeq* Server_impl::get_domain_status_descriptions(const char* lang)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            ObjectStatusDescSeq_var state_seq = new ObjectStatusDescSeq;
            Fred::OperationContext ctx;
            set_corba_seq<ObjectStatusDescSeq, ObjectStatusDesc>
            (state_seq, get_object_status_desc(
                Corba::unwrap_string_from_const_char_ptr(lang),"domain", ctx));
            return state_seq._retn();
        }
        catch(const MISSING_LOCALIZATION&)
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }

    ObjectStatusDescSeq* Server_impl::get_contact_status_descriptions(const char* lang)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            ObjectStatusDescSeq_var state_seq = new ObjectStatusDescSeq;
            Fred::OperationContext ctx;
            set_corba_seq<ObjectStatusDescSeq, ObjectStatusDesc>
            (state_seq, get_object_status_desc(
                Corba::unwrap_string_from_const_char_ptr(lang),"contact", ctx));
            return state_seq._retn();
        }
        catch(const MISSING_LOCALIZATION&)
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }
    ObjectStatusDescSeq* Server_impl::get_nsset_status_descriptions(const char* lang)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            ObjectStatusDescSeq_var state_seq = new ObjectStatusDescSeq;
            Fred::OperationContext ctx;
            set_corba_seq<ObjectStatusDescSeq, ObjectStatusDesc>
            (state_seq, get_object_status_desc(
                Corba::unwrap_string_from_const_char_ptr(lang),"nsset", ctx));
            return state_seq._retn();
        }
        catch(const MISSING_LOCALIZATION&)
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }
    ObjectStatusDescSeq* Server_impl::get_keyset_status_descriptions(const char* lang)
    {
        try
        {
            LOGGING_CONTEXT(log_ctx);

            ObjectStatusDescSeq_var state_seq = new ObjectStatusDescSeq;
            Fred::OperationContext ctx;
            set_corba_seq<ObjectStatusDescSeq, ObjectStatusDesc>
            (state_seq, get_object_status_desc(
                Corba::unwrap_string_from_const_char_ptr(lang),"keyset", ctx));
            return state_seq._retn();
        }
        catch(const MISSING_LOCALIZATION&)
        {
            throw;
        }
        catch (...) { }

        // default exception handling
        throw INTERNAL_SERVER_ERROR();
    }
}
}
