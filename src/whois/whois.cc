#include "whois.h"

#include "src/corba/admin/common.h"

#include "src/fredlib/registrar/info_registrar_output.h"

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
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return Registrar();
}

std::vector<Registrar> Server_impl::get_registrars()
{
    try
    {
        std::vector<Registrar> result;
        Fred::OperationContext ctx;

        std::vector<Fred::InfoRegistrarOutput> v = Fred::InfoRegistrarAllExceptSystem().exec(ctx, output_timezone);
        Registrar temp;
        for(std::vector<Fred::InfoRegistrarOutput>::iterator it = v.begin(); it!=v.end(); ++it)
        {
            temp.address.city = it->info_registrar_data.city.get_value_or_default();
            temp.address.country_code = it->info_registrar_data.country.get_value_or_default();
            temp.address.postal_code = it->info_registrar_data.postalcode.get_value_or_default();
            temp.address.stateorprovince = it->info_registrar_data.stateorprovince.get_value_or_default();
            temp.address.street1 = it->info_registrar_data.street1.get_value_or_default();
            temp.address.street2 = it->info_registrar_data.street2.get_value_or_default();
            temp.address.street3 = it->info_registrar_data.street3.get_value_or_default();
            temp.fax = it->info_registrar_data.fax.get_value_or_default();
            temp.handle = it->info_registrar_data.handle;
            temp.id = it->info_registrar_data.id;
            temp.name = it->info_registrar_data.name.get_value_or_default();
            temp.organization = it->info_registrar_data.organization.get_value_or_default();
            temp.phone = it->info_registrar_data.telephone.get_value_or_default();
            temp.url = it->info_registrar_data.url.get_value_or_default();
            result.push_back(temp);
        }

        return result;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return RegistrarSeq();
}

std::vector<RegistrarGroup> Server_impl::get_registrar_groups()
{
    try
    {
        std::vector<RegistrarGroup> reg_grp_seq;
        Fred::OperationContext ctx;

        std::map<std::string, std::vector<std::string> > groups = ::Whois::get_registrar_groups(ctx);
        reg_grp_seq.reserve(groups.size());

        for(typename std::vector<RegistrarGroup>::const_iterator ci = reg_grp_seq.begin() ; ci != reg_grp_seq.end(); ++ci)
        {
             RegistrarGroup temp;
             temp.name = ci->name;
             temp.members = ci->members;
        }
        return reg_grp_seq;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<RegistrarGroup>;
}

std::vector<RegistrarCertification> Server_impl::get_registrar_certification_list()
{
    try
    {
        std::vector<RegistrarCertification> result;
        Fred::OperationContext ctx;
        typedef std::vector<::Whois::RegistrarCertificationData> certificate_list;
        certificate_list v_rcd= ::Whois::get_registrar_certifications(ctx);
        RegistrarCertification temp;
        for(certificate_list::iterator it = v_rcd.begin(); it != v_rcd.end(); ++it)
        {
            temp.evaluation_file_id = it->get_registrar_evaluation_file_id();
            temp.registrar_handle = it->get_registrar_handle();
            temp.score = it->get_registrar_score();
            result.push_back(temp);
        }
        return result;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return RegistrarCertificationList();
}

std::vector<std::string> Server_impl::get_managed_zone_list()
{
    try
    {
        Fred::OperationContext ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);

        return zone_seq;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<std::string>;
}

Contact Server_impl::get_contact_by_handle(const std::string& handle)
{
    try
    {
        try
        {
            Fred::OperationContext ctx;
            Fred::InfoContactData icd = Fred::InfoContactByHandle(handle)
                .exec(ctx, output_timezone).info_contact_data;
            Contact con;
            con.address.city = icd.place.get_value_or_default().city;
            con.address.country_code = icd.place.get_value_or_default().country;
            con.address.postal_code = icd.place.get_value_or_default().postalcode;
            con.address.stateorprovince = icd.place.get_value_or_default().stateorprovince;
            con.address.street1 = icd.place.get_value_or_default().street1;
            con.address.street2 = icd.place.get_value_or_default().street2.get_value_or_default();
            con.address.street3 = icd.place.get_value_or_default().street3.get_value_or_default();

            return con;
        }
        catch(const Fred::InfoContactByHandle::Exception& e)
        {
            if(e.is_set_unknown_contact_handle())
            {
                if(Fred::CheckContact(handle).is_invalid_handle())
                {
                    throw InvalidHandle();
                }

                throw ObjectNotExists();
            }
        }
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return Contact();
}

NSSet Server_impl::get_nsset_by_handle(const std::string& handle)
{
    try
    {
        try
        {
            Fred::OperationContext ctx;
            Fred::InfoNssetData ind = Fred::InfoNssetByHandle(handle).exec(ctx, output_timezone).info_nsset_data;
            NSSet ns_set;
            if(!ind.update_time.isnull())
                ns_set.changed.get_value();
            ns_set.created = ind.creation_time;
            ns_set.handle = ind.handle;
            if(!ind.transfer_time.isnull())
                ns_set.last_transfer = ind.transfer_time.get_value()
            NameServer ns;
            for(std::vector<Fred::DnsHost>::iterator it = ind.dns_hosts.begin(); it != ind.dns_hosts.end(); ++it)
            {
                ns.fqdn = it->get_fqdn();
                std::vector<boost::asio::ip::address> ip_addresses = it->get_inet_addr();
                IPAddress ipa;
                for(std::vector<boost::asio::ip::address>::iterator addr_it = ip_addresses.begin(); addr_it != ip_addresses.end(); ++addr_it)
                {
                    ipa.address = addr_it->to_string();
                    ipa.version = addr_it->is_v4() ? IPVersion::IPv4 : IPVersion::IPv6;
                    ns.ip_adadresses.push_back(ipa);
                }
                ns_set.nservers.push_back(ns);
            }
            ns_set.registrar_handle = ind.sponsoring_registrar_handle;
            std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(ind.id).exec(ctx);
            for(std::vector<Fred::ObjectStateData>::iterator it = v_osd.begin(); it != v_osd.end(); ++it)
            {
                if(it->is_external)
                    ns_set.statuses.push_back(it->state_name);
            }

        }
        catch(const Fred::InfoNssetByHandle::Exception& e)
        {
            if(e.is_set_unknown_handle())
            {
                if(Fred::CheckNsset(handle).is_invalid_handle())
                {
                    throw InvalidHandle();
                }

                throw ObjectNotExists();
            }
        }
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return NSSet();
}

NSSetSeq Server_impl::get_nssets_by_ns(const std::string& handle,
                                       unsigned long limit,
                                       bool limit_exceeded)
{
    try
    {
        Fred::OperationContext ctx;
        NSSetSeq nss_seq;
        std::vector<Fred::InfoNssetOutput> nss_info =
                Fred::InfoNssetByDNSFqdn(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);

        if(nss_info.empty())
        {
            if(Fred::CheckDomain(handle).is_invalid_syntax())
            {
                throw InvalidHandle();
            }

            throw ObjectNotExists();
        }

        limit_exceeded = false;
        if(nss_info.size() > limit)
        {
            limit_exceeded = true;
            nss_info.erase(nss_info.begin());//depends on InfoNsset ordering
        }

        return nss_seq;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return NSSetSeq();
}

NSSetSeq* Server_impl::get_nssets_by_tech_c(
    const std::string& handle,
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


NameServer* Server_impl::get_nameserver_by_fqdn(const std::string& fqdn)
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

KeySet* Server_impl::get_keyset_by_handle(const std::string& handle)
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
    const std::string& handle,
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

Domain* Server_impl::get_domain_by_handle(const std::string& handle)
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
    const std::string& handle,
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
    const std::string& handle,
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
    const std::string& handle,
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
    const std::string& handle,
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

ObjectStatusDescSeq* Server_impl::get_domain_status_descriptions(const std::string& lang)
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

ObjectStatusDescSeq* Server_impl::get_contact_status_descriptions(const std::string& lang)
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
ObjectStatusDescSeq* Server_impl::get_nsset_status_descriptions(const std::string& lang)
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
ObjectStatusDescSeq* Server_impl::get_keyset_status_descriptions(const std::string& lang)
