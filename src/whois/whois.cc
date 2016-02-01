#include "whois.h"

#include "src/corba/admin/common.h"

//#include "util/db/manager_tss.h"

//#include "log/logger.h"
//#include "src/old_utils/log.h"

#include <boost/foreach.hpp>

static void log_and_rethrow_exception_handler(Fred::OperationContext& ctx)//code duplication?
{
    try
    {
        throw;
    }
    catch(const Fred::OperationException& ex)
    {
        ctx.get_log().warning(ex.what());
        ctx.get_log().warning(boost::algorithm::replace_all_copy(boost::diagnostic_information(ex),"\n", " "));
        throw;
    }
    catch(const Fred::InternalError& ex)
    {
        ctx.get_log().error(boost::algorithm::replace_all_copy(ex.get_exception_stack_info(),"\n", " "));
        ctx.get_log().error(boost::algorithm::replace_all_copy(boost::diagnostic_information(ex),"\n", " "));
        ctx.get_log().error(ex.what());
        throw;
    }
    catch(const std::exception& ex)
    {
        ctx.get_log().error(ex.what());
        throw;
    }
    catch(...)
    {
        ctx.get_log().error("unknown exception");
        throw;
    }
}

Registrar Server_impl::get_registrar_by_handle(const std::string& handle)
{
    try
    {
        try
        {
            Fred::OperationContext ctx;
            Registrar reg;
            Fred::InfoRegistrarData ird = Fred::InfoRegistrarByHandle(handle)
                                          .exec(ctx, output_timezone)
                                          .info_registrar_data;
            //TODO put ifs
            reg.address.city = ird.city;
            reg.address.country_code = ird.country.get_value_or_default()();
            reg.address.postal_code = ird.postalcode.get_value_or_default();
            reg.address.stateorprovince = ird.stateorprovince.get_value_or_default();
            reg.address.street1 = ird.street1.get_value_or_default();
            reg.address.street2 = ird.street2.get_value_or_default();
            reg.address.street3 = ird.street3.get_value_or_default();
            reg.fax = ird.fax.get_value_or_default();
            reg.handle = ird.handle;
            reg.id = ird.id;
            reg.name = ird.name.get_value_or_default();
            reg.organization = ird.organization.get_value_or_default();
            reg.phone = ird.telephone.get_value_or_default();
            reg.url = ird.url.get_value_or_default();
            return reg;
        }
        catch(const Fred::InfoRegistrarByHandle::Exception& e)
        {
            if(e.is_set_unknown_registrar_handle())
            {
                if(Fred::CheckRegistrar(handle).is_invalid_handle())
                {
                    throw InvalidHandle();
                }

                throw ObjectNotExists();
            }
        }
    }
//    catch(const ::CORBA::UserException& )
//    {
//        throw;
//    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return Registrar();
}

RegistrarSeq Server_impl::get_registrars()
{
    try
    {
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
        Fred::OperationContext ctx;
        NSSetSeq_var nss_seq = new NSSetSeq;
        std::vector<Fred::InfoNssetOutput> nss_info = Fred::InfoNssetByDNSFqdn(
            Corba::unwrap_string_from_const_char_ptr(handle)).set_limit(limit + 1).exec(ctx, output_timezone);

        if(nss_info.empty())
        {
            if(Fred::CheckDomain(Corba::unwrap_string_from_const_char_ptr(handle)).is_invalid_syntax())
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
        std::string ns_fqdn = Corba::unwrap_string_from_const_char_ptr(fqdn);

        Fred::OperationContext ctx;

        if(::Whois::nameserver_exists(ns_fqdn,ctx))
        {
            NameServer temp;
            temp.fqdn = Corba::wrap_string_to_corba_string(ns_fqdn);
            //temp.ip_addresses;//TODO missing implementation #13722
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
        std::string fqdn;
        Fred::OperationContext ctx;
        try
        {
            fqdn = Corba::unwrap_string_from_const_char_ptr(handle);

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
                        Fred::InfoDomainByHandle(
                            Corba::unwrap_string_from_const_char_ptr(handle)
                        ).exec(ctx, output_timezone)//this will throw if object not found
                        .info_domain_data
                    ));

            if(::Whois::is_domain_delete_pending(Corba::unwrap_string_from_const_char_ptr(handle), ctx, "Europe/Prague"))
            {
                return new Domain(generate_obfuscate_domain_delete_candidate(
                        Corba::unwrap_string_from_const_char_ptr(handle)));
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
        Fred::OperationContext ctx;

        DomainSeq_var domain_seq = new DomainSeq;

        std::vector<Fred::InfoDomainOutput> domain_info = Fred::InfoDomainByNssetHandle(
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

DomainSeq* Server_impl::get_domains_by_keyset(
    const char* handle,
    ::CORBA::ULong limit,
    ::CORBA::Boolean& limit_exceeded)
{
    try
    {
        Fred::OperationContext ctx;

        DomainSeq_var domain_seq = new DomainSeq;

        std::vector<Fred::InfoDomainOutput> domain_info = Fred::InfoDomainByKeysetHandle(
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
