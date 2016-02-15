#include "whois.h"

//#include "src/corba/admin/common.h"

#include "src/fredlib/registrar/info_registrar_output.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/whois/zone_list.h"
#include "src/whois/is_domain_delete_pending.h"

#include "src/whois/nameserver_exists.h"
#include "src/whois/registrar_group.h"
#include "src/whois/registrar_certification.h"
//#include "src/domain_browser/domain_browser.cc"
#include "src/fredlib/registrar/check_registrar.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/info_domain_data.h"
#include "src/fredlib/contact/check_contact.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"

#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/nsset_dns_host.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/info_keyset_data.h"
#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/keyset/keyset_dns_key.h"
#include "src/fredlib/nsset/check_nsset.h"

//#include "util/db/manager_tss.h"

//#include "log/logger.h"
//#include "src/old_utils/log.h"

#include <boost/foreach.hpp>


namespace Registry{
namespace WhoisImpl{

const std::string Server_impl::output_timezone("UTC");

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

Registrar make_registrar_from_info_data(Fred::InfoRegistrarData& ird)
{
    Registrar reg;
    reg.address.city = ird.city.get_value_or_default();
    reg.address.country_code = ird.country.get_value_or_default();
    reg.address.postal_code = ird.postalcode.get_value_or_default();
    reg.address.stateorprovince =
            ird.stateorprovince.get_value_or_default();
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

Registrar Server_impl::get_registrar_by_handle(const std::string& handle)
{
    Fred::OperationContext ctx;
    try
    {
        try
        {
            Fred::InfoRegistrarData ird =
                    Fred::InfoRegistrarByHandle(handle).exec(ctx, output_timezone)
                    .info_registrar_data;
            return make_registrar_from_info_data(ird);
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
    Fred::OperationContext ctx;
    try
    {
        std::vector<Registrar> result;
        std::vector<Fred::InfoRegistrarOutput> v =
                Fred::InfoRegistrarAllExceptSystem().exec(ctx, output_timezone);
        result.reserve(v.size());
        for(std::vector<Fred::InfoRegistrarOutput>::iterator it = v.begin();
                it!=v.end(); ++it)
        {
            result.push_back(make_registrar_from_info_data(it->info_registrar_data));
        }

        return result;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<Registrar>();
}

std::vector<RegistrarGroup> Server_impl::get_registrar_groups()
{
    Fred::OperationContext ctx;
    try
    {
        std::vector<RegistrarGroup> reg_grp_seq;

        std::map<std::string, std::vector<std::string> > groups =
                ::Whois::get_registrar_groups(ctx);
        reg_grp_seq.reserve(groups.size());
        typename std::vector<RegistrarGroup>::const_iterator ci = reg_grp_seq.begin();
        for(; ci != reg_grp_seq.end(); ++ci)
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
    return std::vector<RegistrarGroup>();
}

std::vector<RegistrarCertification> Server_impl::get_registrar_certification_list()
{
    Fred::OperationContext ctx;
    try
    {
        std::vector<RegistrarCertification> result;
        typedef std::vector< ::Whois::RegistrarCertificationData> CertificateList;
        CertificateList v_rcd = ::Whois::get_registrar_certifications(ctx);
        RegistrarCertification temp;
        result.reserve(v_rcd.size());
        for(CertificateList::iterator it = v_rcd.begin(); it != v_rcd.end(); ++it)
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
    return std::vector<RegistrarCertification>();
}

std::vector<std::string> Server_impl::get_managed_zone_list()
{
    Fred::OperationContext ctx;
    try
    {
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);

        return zone_seq;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<std::string>();
}

Contact Server_impl::get_contact_by_handle(const std::string& handle)
{
    Fred::OperationContext ctx;
    try
    {
        try
        {
            Fred::InfoContactData icd = Fred::InfoContactByHandle(handle)
                .exec(ctx, output_timezone).info_contact_data;
            Contact con;
            con.address.city = icd.place.get_value_or_default().city;
            con.address.country_code = icd.place.get_value_or_default().country;
            con.address.postal_code = icd.place.get_value_or_default().postalcode;
            con.address.stateorprovince =
                    icd.place.get_value_or_default().stateorprovince.get_value_or_default();
            con.address.street1 =
                    icd.place.get_value_or_default().street1;
            con.address.street2 =
                    icd.place.get_value_or_default().street2.get_value_or_default();
            con.address.street3 =
                    icd.place.get_value_or_default().street3.get_value_or_default();
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
            throw;
        }
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return Contact();
}

WhoisImpl::NSSet make_nsset_from_info_data(const Fred::InfoNssetData& ind, Fred::OperationContext& ctx)
{
    WhoisImpl::NSSet nss;
    nss.changed = Nullable<boost::posix_time::ptime>(ind.update_time.get_value_or_default());
    nss.created = ind.creation_time;
    nss.handle = ind.handle;
    nss.last_transfer = ind.transfer_time.get_value_or_default();
    nss.nservers.reserve(ind.dns_hosts.size());
    WhoisImpl::NameServer ns;
    for(std::vector<Fred::DnsHost>::const_iterator it = ind.dns_hosts.begin(); //was ::iterator
            it != ind.dns_hosts.end(); ++it)
    {
        ns.fqdn = it->get_fqdn();
        std::vector<boost::asio::ip::address> ip_addresses =
                it->get_inet_addr();
        ns.ip_addresses.reserve(ip_addresses.size());
//                IPAddress ipa;
        for(std::vector<boost::asio::ip::address>::iterator addr_it =
                ip_addresses.begin(); addr_it != ip_addresses.end(); ++addr_it)
        {
//                    ipa.address = addr_it->to_string();
//                    ipa.version = addr_it->is_v4() ? IPVersion::IPv4
//                                                   : IPVersion::IPv6;
            ns.ip_addresses.push_back(*addr_it);
        }
        nss.nservers.push_back(ns);
    }
    nss.registrar_handle = ind.sponsoring_registrar_handle;
    std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(ind.id).exec(ctx);
    nss.statuses.reserve(v_osd.size());
    for(std::vector<Fred::ObjectStateData>::iterator it2 = v_osd.begin();
            it2 != v_osd.end(); ++it2)
    {
        if(it2->is_external)
            nss.statuses.push_back(it2->state_name);
    }
    return nss;
}

WhoisImpl::NSSet Server_impl::get_nsset_by_handle(const std::string& handle)
{
    Fred::OperationContext ctx;
    try
    {
        try
        {
            Fred::InfoNssetData ind =
                    Fred::InfoNssetByHandle(handle).exec(ctx, output_timezone)
                    .info_nsset_data;
            return make_nsset_from_info_data(ind, ctx);
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
    return WhoisImpl::NSSet();
}

NSSetSeq Server_impl::get_nssets_by_ns(const std::string& handle,
                                       unsigned long limit,
                                       bool limit_exceeded)
{
    Fred::OperationContext ctx;
    try
    {
        NSSetSeq nss_seq;
        std::vector<Fred::InfoNssetOutput> nss_info =
                Fred::InfoNssetByDNSFqdn(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);

        nss_seq.content.reserve(nss_info.size());
        WhoisImpl::NSSet temp;
        for(std::vector<Fred::InfoNssetOutput>::iterator it = nss_info.begin();
                it != nss_info.end(); ++it)
        {
            nss_seq.content.push_back(make_nsset_from_info_data(it->info_nsset_data, ctx));
        }

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

NSSetSeq Server_impl::get_nssets_by_tech_c(const std::string& handle,
                                           unsigned long limit,
                                           bool limit_exceeded)
{
    Fred::OperationContext ctx;
    try
    {
        NSSetSeq nss_seq;

        std::vector<Fred::InfoNssetOutput> nss_info =
                Fred::InfoNssetByTechContactHandle(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);

        if(nss_info.empty())
        {
            if(Fred::CheckContact(handle).is_invalid_handle())
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
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return NSSetSeq();
}


NameServer Server_impl::get_nameserver_by_fqdn(const std::string& fqdn)
{
    Fred::OperationContext ctx;
    try
    {

        if(::Whois::nameserver_exists(fqdn,ctx))
        {
            NameServer temp;
            temp.fqdn = fqdn;
            return temp;
        }
        else
        {
            if(Fred::CheckDomain(fqdn).is_invalid_syntax())
            {
                throw InvalidHandle();
            }

            throw ObjectNotExists();
        }
    }
    catch(...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return NameServer();
}

WhoisImpl::KeySet Server_impl::get_keyset_by_handle(const std::string& handle)
{
    Fred::OperationContext ctx;
    try
    {
        try
        {
            Fred::InfoKeysetData ikd = Fred::InfoKeysetByHandle(handle)
                .exec(ctx, output_timezone).info_keyset_data;
            WhoisImpl::KeySet ks;
            ks.changed = ikd.update_time.get_value_or_default();
            ks.created = ikd.creation_time;
            ks.dns_keys.reserve(ikd.dns_keys.size());
            DNSKey dns_k;
            for(std::vector<Fred::DnsKey>::iterator it = ikd.dns_keys.begin();
                    it != ikd.dns_keys.end(); ++it)
            {
                dns_k.alg = it->get_alg();
                dns_k.flags = it->get_flags();
                dns_k.protocol = it->get_protocol();
                dns_k.public_key = it->get_key();
                ks.dns_keys.push_back(dns_k);
            }
            return ks;
        }
        catch(const Fred::InfoKeysetByHandle::Exception& e)
        {
            if(e.is_set_unknown_handle())
            {
                if(Fred::CheckKeyset(handle).is_invalid_handle())
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
    return KeySet();
}

KeySetSeq Server_impl::get_keysets_by_tech_c(const std::string& handle,
                                             unsigned long limit,
                                             bool limit_exceeded)
{
    Fred::OperationContext ctx;
    try
    {
        KeySetSeq ks_seq;

        std::vector<Fred::InfoKeysetOutput> ks_info =
                Fred::InfoKeysetByTechContactHandle(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);
        ks_seq.content.reserve(ks_info.size());
        WhoisImpl::KeySet temp;
        for(std::vector<Fred::InfoKeysetOutput>::iterator it = ks_info.begin();
                it != ks_info.end(); ++it)
        {
            temp.changed = it->info_keyset_data.update_time.get_value_or_default();
            temp.created = it->info_keyset_data.creation_time;

            temp.dns_keys.reserve(it->info_keyset_data.dns_keys.size());
            DNSKey tmp_dns;
            for(std::vector<Fred::DnsKey>::iterator it_dns =
                    it->info_keyset_data.dns_keys.begin();
                    it_dns != it->info_keyset_data.dns_keys.end(); ++it_dns)
            {
                tmp_dns.alg = it_dns->get_alg();
                tmp_dns.flags = it_dns->get_flags();
                tmp_dns.protocol = it_dns->get_protocol();
                tmp_dns.public_key = it_dns->get_key();
                temp.dns_keys.push_back(tmp_dns);
            }

            temp.handle = it->info_keyset_data.handle;
            temp.last_transfer = it->info_keyset_data.transfer_time.get_value_or_default();
            temp.registrar_handle = it->info_keyset_data.create_registrar_handle;

            std::vector<Fred::ObjectStateData> v_osd =
                    Fred::GetObjectStates(it->info_keyset_data.id).exec(ctx);
            temp.statuses.reserve(v_osd.size());
            for(std::vector<Fred::ObjectStateData>::iterator it_osd = v_osd.begin();
                    it_osd != v_osd.end(); ++it_osd)
            {
                temp.statuses.push_back(it_osd->state_name);
            }

            temp.tech_contact_handles.reserve(it->info_keyset_data.tech_contacts.size());
            for(std::vector<Fred::ObjectIdHandlePair>::iterator it_oihp =
                    it->info_keyset_data.tech_contacts.begin();
                    it_oihp != it->info_keyset_data.tech_contacts.end(); ++it_oihp)
            {
                temp.tech_contact_handles.push_back(it_oihp->handle);
            }
            ks_seq.content.push_back(temp);
        }
        if(ks_info.empty())
        {
            if(Fred::CheckContact(handle).is_invalid_handle())
            {
                throw InvalidHandle();
            }

            throw ObjectNotExists();
        }

        limit_exceeded = false;
        if(ks_info.size() > limit)
        {
            limit_exceeded = true;
            ks_info.erase(ks_info.begin());//depends on InfoKeyset ordering
        }

        return ks_seq;
    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return KeySetSeq();
}

WhoisImpl::Domain generate_obfuscate_domain_delete_candidate(const std::string& _handle)
{
    WhoisImpl::Domain temp;
    temp.fqdn = _handle;
    temp.changed = Nullable<boost::posix_time::ptime>();
    temp.last_transfer = Nullable<boost::posix_time::ptime>();
    temp.statuses.push_back("deleteCandidate");
    //all the rest is default constructed
    return temp;
}

WhoisImpl::Domain make_domain_from_info_data(const Fred::InfoDomainData& idd, Fred::OperationContext& ctx)
{
    WhoisImpl::Domain result;
    result.admin_contact_handles.reserve(idd.admin_contacts.size());
    std::vector<Fred::ObjectIdHandlePair>::const_iterator it =
            idd.admin_contacts.begin();
    for(; it != idd.admin_contacts.end(); ++it)
        result.admin_contact_handles.push_back(it->handle);
    result.changed = idd.update_time.get_value_or_default();
    result.expire = idd.expiration_date;
    result.fqdn = idd.fqdn;
    result.keyset_handle = idd.keyset.get_value_or_default().handle;
    result.last_transfer = idd.transfer_time.get_value_or_default();
    result.nsset_handle = idd.nsset.get_value_or_default().handle;
    result.registered = idd.creation_time;
    result.registrant_handle = idd.registrant.handle;
    std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(idd.id).exec(ctx);
    result.statuses.reserve(v_osd.size());
    for(std::vector<Fred::ObjectStateData>::iterator it2 = v_osd.begin();
            it2 != v_osd.end(); ++it2)
    {
        result.statuses.push_back(it2->state_name);
    }
        result.validated_to =
                idd.enum_domain_validation.get_value_or_default().validation_expiration;
    return result;
}

WhoisImpl::Domain Server_impl::get_domain_by_handle(const std::string& handle)
{
    Fred::OperationContext ctx;
    try
    {
        try
        {
            //check general name rules
            if(Fred::CheckDomain(handle).is_invalid_syntax())
            {
                throw InvalidLabel();
            }
            if(Fred::CheckDomain(handle).is_bad_zone(ctx))
            {
                throw UnmanagedZone();
            }
            if(Fred::CheckDomain(handle).is_bad_length(ctx))
            {
                throw TooManyLabels();
            }
            if(::Whois::is_domain_delete_pending(handle, ctx, "Europe/Prague"))
            {
                return Domain(generate_obfuscate_domain_delete_candidate(handle));
            }

            Fred::InfoDomainData idd = Fred::InfoDomainByHandle(handle)
                .exec(ctx, output_timezone).info_domain_data;

            return make_domain_from_info_data(idd, ctx);
        }
        catch(const Fred::InfoDomainByHandle::Exception& e)
        {
            if(e.is_set_unknown_fqdn())
            {
                //check current registry name rules (that might change over time)
                if(Fred::CheckDomain(handle).is_invalid_handle(ctx))
                {
                    throw InvalidLabel();
                }

                throw ObjectNotExists();
            }
        }
    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return Domain();
}

void set_domains_seq(DomainSeq& domain_seq,
                     const std::vector<Fred::InfoDomainOutput>& il,
                     Fred::OperationContext& ctx)
{
    domain_seq.content.reserve(il.size());
    for(std::vector<Fred::InfoDomainOutput>::const_iterator it = il.begin(); //iterator
            it != il.end(); ++it)
    {
        if(::Whois::is_domain_delete_pending(it->info_domain_data.fqdn, ctx,
                                             "Europe/Prague"))
        {
            domain_seq.content.push_back(
                    generate_obfuscate_domain_delete_candidate(
                            it->info_domain_data.fqdn));
        }
        else
        {
            domain_seq.content.push_back(make_domain_from_info_data(it->info_domain_data, ctx));
        }
    }
}

DomainSeq Server_impl::get_domains_by_registrant(const std::string& handle,
                                                 unsigned long limit,
                                                 bool limit_exceeded)
{
    Fred::OperationContext ctx;
    try
    {
        DomainSeq domain_seq;

        std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByRegistrantHandle(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);
        if(domain_info.empty())
        {
            if(Fred::CheckContact(handle).is_invalid_handle())
            {
                throw InvalidHandle();
            }

            throw ObjectNotExists();
        }

        limit_exceeded = false;
        if(domain_info.size() > limit)
        {
            limit_exceeded = true;
            domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
        }

        set_domains_seq(domain_seq,domain_info,ctx);
        return domain_seq;
    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_admin_contact(const std::string& handle,
                                                    unsigned long limit,
                                                    bool limit_exceeded)
{
    Fred::OperationContext ctx;
    try
    {
        DomainSeq domain_seq;
        std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByAdminContactHandle(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);

        if(domain_info.empty())
        {
            if(Fred::CheckContact(handle).is_invalid_handle())
            {
                throw InvalidHandle();
            }

            throw ObjectNotExists();
        }

        limit_exceeded = false;
        if(domain_info.size() > limit)
        {
            limit_exceeded = true;
            domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
        }

        return domain_seq;
    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_nsset(const std::string& handle,
                                            unsigned long limit,
                                            bool limit_exceeded)
{
    Fred::OperationContext ctx;
    try
    {
        DomainSeq domain_seq;
        std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByNssetHandle(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);
        if(domain_info.empty())
        {
            if(Fred::CheckContact(handle).is_invalid_handle())
            {
                throw InvalidHandle();
            }

            throw ObjectNotExists();
        }

        limit_exceeded = false;
        if(domain_info.size() > limit)
        {
            limit_exceeded = true;
            domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
        }

        return domain_seq;
    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_keyset(const std::string& handle,
                                             unsigned long limit,
                                             bool limit_exceeded)
{
    Fred::OperationContext ctx;
    try
    {
        DomainSeq domain_seq;
        std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByKeysetHandle(handle).set_limit(limit + 1)
                .exec(ctx, output_timezone);

        if(domain_info.empty())
        {
            if(Fred::CheckContact(handle).is_invalid_handle())
            {
                throw InvalidHandle();
            }

            throw ObjectNotExists();
        }

        limit_exceeded = false;
        if(domain_info.size() > limit)
        {
            limit_exceeded = true;
            domain_info.erase(domain_info.begin());//depends on InfoDomain ordering
        }

        return domain_seq;
    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

std::vector< std::pair<std::string, std::string> >
get_object_status_desc(const std::string& lang, const std::string& type,
                       Fred::OperationContext& ctx)
{
    std::vector<Fred::ObjectStateDescription> states =
            Fred::GetObjectStateDescriptions(lang).set_object_type(type)
            .set_external().exec(ctx);

    if(states.empty()) throw MissingLocalization();

    std::vector< std::pair<std::string, std::string> > temp;
    temp.reserve(states.size());
    for(std::vector<Fred::ObjectStateDescription>::const_iterator ci
            = states.begin(); ci != states.end(); ++ci)
    {
        temp.push_back(std::make_pair(ci->handle, ci->description));
    }

    return temp;
}

std::vector<ObjectStatusDesc> Server_impl::get_object_status_descriptions(
        const std::string& lang,
        const std::string& type)
{
    Fred::OperationContext ctx;
    try
    {
        std::vector<ObjectStatusDesc> state_seq;
        ObjectStatusDesc tmp;
        std::vector< std::pair<std::string, std::string> > osd =
                get_object_status_desc(lang, type, ctx);
        state_seq.reserve(osd.size());
        for(std::vector< std::pair<std::string, std::string> >::iterator it =
                osd.begin(); it != osd.end(); ++it)
        {
            tmp.handle = it->first;
            tmp.name = it->second;
            state_seq.push_back(tmp);
        }
        return state_seq;
    }
    catch(const MissingLocalization&)
    {
        throw;
    }
    catch (...) {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<ObjectStatusDesc>();
}

std::vector<ObjectStatusDesc> Server_impl::get_domain_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "domain");
}

std::vector<ObjectStatusDesc> Server_impl::get_contact_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "contact");
}

std::vector<ObjectStatusDesc> Server_impl::get_nsset_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "nsset");
}

std::vector<ObjectStatusDesc> Server_impl::get_keyset_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "keyset");
}

}//Registry
}//WhoisImpl
