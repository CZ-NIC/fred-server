#include "src/whois/whois.h"

#include "src/whois/zone_list.h"
#include "src/whois/is_domain_delete_pending.h"
#include "src/whois/domain_expiration_datetime.h"
#include "src/whois/nameserver_exists.h"
#include "src/whois/registrar_group.h"
#include "src/whois/registrar_certification.h"

#include "src/fredlib/registrar/info_registrar_output.h"
#include "src/fredlib/registrar/info_registrar.h"
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
#include "util/log/context.h"
#include "util/random.h"

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

namespace Registry
{
namespace WhoisImpl
{

static void log_and_rethrow_exception_handler(Fred::OperationContext& ctx)
{
    try
    {
        throw;
    }
    catch (const Fred::OperationException& ex)
    {
        ctx.get_log().warning(ex.what());
        ctx.get_log().warning(
                boost::algorithm::replace_all_copy(boost::diagnostic_information(ex),"\n", " "));
        throw;
    }
    catch (const Fred::InternalError& ex)
    {
        ctx.get_log().error(boost::algorithm::replace_all_copy(ex.get_exception_stack_info(),"\n", " "));
        ctx.get_log().error(boost::algorithm::replace_all_copy(boost::diagnostic_information(ex),"\n", " "));
        ctx.get_log().error(ex.what());
        throw;
    }
    catch (const std::exception& ex)
    {
        ctx.get_log().error(ex.what());
        throw;
    }
    catch (...)
    {
        ctx.get_log().error("unknown exception");
        throw;
    }
}

namespace
{
std::string get_output_timezone() { static const std::string timezone("UTC"); return timezone; }

std::string create_ctx_name(const std::string &_name)
{
    return boost::str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
}

std::string create_ctx_function_name(const char *fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const Server_impl& _impl, const std::string& _op_name)
    : ctx_server_(create_ctx_name(_impl.get_server_name())),
      ctx_operation_(_op_name)
    {
    }
private:
    Logging::Context ctx_server_;
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) LogContext CTX_VAR((IMPL_OBJ), create_ctx_function_name(__FUNCTION__))
} // anonymous namespace

Registrar make_registrar_from_info_data(const Fred::InfoRegistrarData& ird)
{
    Registrar reg;
    reg.address.city            = ird.city.get_value_or_default();
    reg.address.country_code    = ird.country.get_value_or_default();
    reg.address.postal_code     = ird.postalcode.get_value_or_default();
    reg.address.stateorprovince = ird.stateorprovince.get_value_or_default();
    reg.address.street1         = ird.street1.get_value_or_default();
    reg.address.street2         = ird.street2.get_value_or_default();
    reg.address.street3         = ird.street3.get_value_or_default();
    reg.fax                     = ird.fax.get_value_or_default();
    reg.handle                  = ird.handle;
    reg.id                      = ird.id;
    reg.name                    = ird.name.get_value_or_default();
    reg.organization            = ird.organization.get_value_or_default();
    reg.phone                   = ird.telephone.get_value_or_default();
    reg.url                     = ird.url.get_value_or_default();
    return reg;
}

Registrar Server_impl::get_registrar_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        try
        {
            return make_registrar_from_info_data(
                    Fred::InfoRegistrarByHandle(handle)
                       .exec(ctx, get_output_timezone())
                       .info_registrar_data);
        }
        catch (const Fred::InfoRegistrarByHandle::Exception& e)
        {
            if (e.is_set_unknown_registrar_handle())
            {
                if (Fred::CheckRegistrar(handle).is_invalid_handle())
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
    return Registrar();
}

Server_impl::Server_impl(const std::string& _server_name)
: server_name(_server_name)
{
    LogContext log_ctx(*this, "init");
}

const std::string& Server_impl::get_server_name() const
{
    return server_name;
}

std::vector<Registrar> Server_impl::get_registrars()
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoRegistrarOutput> v =
                Fred::InfoRegistrarAllExceptSystem().exec(ctx, get_output_timezone());
        std::vector<Registrar> result;
        result.reserve(v.size());
        BOOST_FOREACH(Fred::InfoRegistrarOutput it, v)
        {
            result.push_back(make_registrar_from_info_data(it.info_registrar_data));
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
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::map<std::string, std::vector<std::string> > groups = ::Whois::get_registrar_groups(ctx);
        std::vector<RegistrarGroup> reg_grp_seq;
        reg_grp_seq.reserve(groups.size());
        for (std::map<std::string, std::vector<std::string> >::const_iterator it = groups.begin();
                it != groups.end(); ++it)
        {
             reg_grp_seq.push_back(RegistrarGroup(it->first, it->second));
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
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        typedef std::vector<Whois::RegistrarCertificationData> CertificateList;
        const CertificateList v_rcd = ::Whois::get_registrar_certifications(ctx);
        std::vector<RegistrarCertification> result;
        result.reserve(v_rcd.size());
        BOOST_FOREACH(const Whois::RegistrarCertificationData& it, v_rcd)
        {
            result.push_back(RegistrarCertification(
                    it.get_registrar_handle(),
                    it.get_registrar_score(),
                    it.get_registrar_evaluation_file_id()));
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
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        return ::Whois::get_managed_zone_list(ctx);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<std::string>();
}

Contact Server_impl::get_contact_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        try
        {
            const Fred::InfoContactData icd =
                    Fred::InfoContactByHandle(handle).exec(ctx, get_output_timezone()).info_contact_data;
            Contact con;
            con.handle                             = icd.handle;
            con.organization                       = icd.organization.get_value_or_default();
            con.name                               = icd.name.get_value_or_default();
            con.phone                              = icd.telephone.get_value_or_default();
            con.fax                                = icd.fax.get_value_or_default();
            con.email                              = icd.email.get_value_or_default();
            con.notify_email                       = icd.notifyemail.get_value_or_default();
            con.vat_number                         = icd.vat.get_value_or_default();
            con.creating_registrar                 = icd.create_registrar_handle;
            con.sponsoring_registrar               = icd.sponsoring_registrar_handle;
            con.created                            = icd.creation_time;
            con.changed                            = icd.update_time;
            con.last_transfer                      = icd.transfer_time;
            con.identification.identification_type = icd.ssntype.get_value_or_default();
            con.identification.identification_data = icd.ssn.get_value_or_default();
            con.address.city                       = icd.place.get_value_or_default().city;
            con.address.country_code               = icd.place.get_value_or_default().country;
            con.address.postal_code                = icd.place.get_value_or_default().postalcode;
            con.address.stateorprovince =
                    icd.place.get_value_or_default().stateorprovince.get_value_or_default();
            con.address.street1 = icd.place.get_value_or_default().street1;
            con.address.street2 = icd.place.get_value_or_default().street2.get_value_or_default();
            con.address.street3 = icd.place.get_value_or_default().street3.get_value_or_default();

            const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(icd.id).exec(ctx);
            con.statuses.reserve(v_osd.size());
            BOOST_FOREACH(Fred::ObjectStateData it, v_osd)
            {
                if (it.is_external)
                {
                    con.statuses.push_back(it.state_name);
                }
            }

            con.disclose_organization = icd.discloseorganization;
            con.disclose_name = icd.disclosename;
            con.disclose_address = icd.discloseaddress;
            con.disclose_phone = icd.disclosetelephone;
            con.disclose_fax = icd.disclosefax;
            con.disclose_email = icd.discloseemail;
            con.disclose_notify_email = icd.disclosenotifyemail;
            con.disclose_identification = icd.discloseident;
            con.disclose_vat_number = icd.disclosevat;

            return con;
        }
        catch (const Fred::InfoContactByHandle::Exception& e)
        {
            if (e.is_set_unknown_contact_handle())
            {
                if (Fred::ContactHandleState::SyntaxValidity::invalid ==
                        Fred::Contact::get_handle_syntax_validity(handle))
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

static WhoisImpl::NSSet make_nsset_from_info_data(const Fred::InfoNssetData& ind, Fred::OperationContext& ctx)
{
    WhoisImpl::NSSet nss;
    nss.changed       = ind.update_time;
    nss.created       = ind.creation_time;
    nss.handle        = ind.handle;
    nss.last_transfer = ind.transfer_time;
    nss.nservers.reserve(ind.dns_hosts.size());
    WhoisImpl::NameServer ns;
    BOOST_FOREACH(Fred::DnsHost it, ind.dns_hosts)
    {
        ns.fqdn = it.get_fqdn();
        const std::vector<boost::asio::ip::address> ip_addresses = it.get_inet_addr();

        ns.ip_addresses.reserve(ip_addresses.size());
        BOOST_FOREACH(boost::asio::ip::address addr_it, ip_addresses)
        {
            ns.ip_addresses.push_back(addr_it);
        }

        nss.nservers.push_back(ns);
    }
    nss.sponsoring_registrar = ind.sponsoring_registrar_handle;
    BOOST_FOREACH(Fred::ObjectIdHandlePair it, ind.tech_contacts)
    {
        nss.tech_contacts.push_back(it.handle);
    }
    const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(ind.id).exec(ctx);
    nss.statuses.reserve(v_osd.size());
    BOOST_FOREACH(Fred::ObjectStateData it, v_osd)
    {
        if (it.is_external)
        {
            nss.statuses.push_back(it.state_name);
        }
    }
    return nss;
}

WhoisImpl::NSSet Server_impl::get_nsset_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        try
        {
            return make_nsset_from_info_data(
                    Fred::InfoNssetByHandle(handle).exec(ctx, get_output_timezone()).info_nsset_data,
                    ctx);
        }
        catch (const Fred::InfoNssetByHandle::Exception& e)
        {
            if (e.is_set_unknown_handle())
            {
                if (Fred::CheckNsset(handle).is_invalid_handle())
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
    return WhoisImpl::NSSet();
}

static NSSetSeq get_nssets_by_(
    Fred::OperationContext& ctx,
    const std::vector<Fred::InfoNssetOutput>& nss_info,
    const std::string& handle,
    unsigned long limit)
{
    NSSetSeq nss_seq;
    std::vector<Fred::InfoNssetOutput>::const_iterator it = nss_info.begin(), end;
    if (nss_info.size() > limit)
    {
        nss_seq.limit_exceeded = true;
        nss_seq.content.reserve(limit);
        end = nss_info.begin() + limit;
    }
    else
    {
        nss_seq.limit_exceeded = false;
        nss_seq.content.reserve(nss_info.size());
        end = nss_info.end();
    }
    for (; it != end; ++it)
    {
        nss_seq.content.push_back(make_nsset_from_info_data(it->info_nsset_data, ctx));
    }
    return nss_seq;
}

NSSetSeq Server_impl::get_nssets_by_ns(
    const std::string& handle,
    unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoNssetOutput> nss_info =
                Fred::InfoNssetByDNSFqdn(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (nss_info.empty())
        {
            if (Fred::CheckDomain(handle).is_invalid_syntax())
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_nssets_by_(ctx, nss_info, handle, limit);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return NSSetSeq();
}

NSSetSeq Server_impl::get_nssets_by_tech_c(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoNssetOutput> nss_info =
                Fred::InfoNssetByTechContactHandle(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (nss_info.empty())
        {
            if (Fred::ContactHandleState::SyntaxValidity::invalid ==
                    Fred::Contact::get_handle_syntax_validity(handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_nssets_by_(ctx, nss_info, handle, limit);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return NSSetSeq();
}

NameServer Server_impl::get_nameserver_by_fqdn(const std::string& fqdn)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        if (::Whois::nameserver_exists(fqdn,ctx))
        {
            NameServer temp;
            temp.fqdn = fqdn;
            /*
             * Because of grouping nameservers in NSSet we don't include
             * IP address in output (given nameserver can be in different
             * NSSets with different IP addresses)
             *
             * temp.ip_addresses;
             */
            return temp;
        }
        else
        {
            if (Fred::CheckDomain(fqdn).is_invalid_syntax())
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return NameServer();
}

WhoisImpl::KeySet Server_impl::get_keyset_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        try
        {
            const Fred::InfoKeysetData ikd =
                    Fred::InfoKeysetByHandle(handle).exec(ctx, get_output_timezone()).info_keyset_data;
            WhoisImpl::KeySet ks;
            ks.handle               = ikd.handle;
            ks.changed              = ikd.update_time;
            ks.created              = ikd.creation_time;
            ks.sponsoring_registrar = ikd.sponsoring_registrar_handle;
            ks.last_transfer        = ikd.transfer_time;
            ks.dns_keys.reserve(ikd.dns_keys.size());
            DNSKey dns_k;
            BOOST_FOREACH(Fred::DnsKey it, ikd.dns_keys)
            {
                dns_k.alg        = it.get_alg();
                dns_k.flags      = it.get_flags();
                dns_k.protocol   = it.get_protocol();
                dns_k.public_key = it.get_key();
                ks.dns_keys.push_back(dns_k);
            }
            BOOST_FOREACH(Fred::ObjectIdHandlePair it, ikd.tech_contacts)
            {
                ks.tech_contacts.push_back(it.handle);
            }
            return ks;
        }
        catch (const Fred::InfoKeysetByHandle::Exception& e)
        {
            if (e.is_set_unknown_handle())
            {
                if (Fred::CheckKeyset(handle).is_invalid_handle())
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
    return KeySet();
}

KeySetSeq Server_impl::get_keysets_by_tech_c(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoKeysetOutput> ks_info =
                Fred::InfoKeysetByTechContactHandle(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        KeySetSeq ks_seq;
        std::vector<Fred::InfoKeysetOutput>::const_iterator it = ks_info.begin(), end;
        if (ks_info.empty())
        {
            if (Fred::ContactHandleState::SyntaxValidity::invalid ==
                    Fred::Contact::get_handle_syntax_validity(handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        if (ks_info.size() > limit)
        {
            ks_seq.limit_exceeded = true;
            ks_seq.content.reserve(limit);
            end = ks_info.begin() + limit;
        }
        else
        {
            ks_seq.limit_exceeded = false;
            ks_seq.content.reserve(ks_info.size());
            end = ks_info.end();
        }
        for (; it != end; ++it)
        {
            WhoisImpl::KeySet temp;
            temp.changed = it->info_keyset_data.update_time;
            temp.created = it->info_keyset_data.creation_time;

            temp.dns_keys.reserve(it->info_keyset_data.dns_keys.size());
            DNSKey tmp_dns;
            BOOST_FOREACH(Fred::DnsKey it_dns, it->info_keyset_data.dns_keys)
            {
                tmp_dns.alg        = it_dns.get_alg();
                tmp_dns.flags      = it_dns.get_flags();
                tmp_dns.protocol   = it_dns.get_protocol();
                tmp_dns.public_key = it_dns.get_key();
                temp.dns_keys.push_back(tmp_dns);
            }

            temp.handle = it->info_keyset_data.handle;
            temp.last_transfer = it->info_keyset_data.transfer_time;
            temp.sponsoring_registrar = it->info_keyset_data.sponsoring_registrar_handle;

            const std::vector<Fred::ObjectStateData> v_osd =
                    Fred::GetObjectStates(it->info_keyset_data.id).exec(ctx);
            temp.statuses.reserve(v_osd.size());
            BOOST_FOREACH(Fred::ObjectStateData it_osd, v_osd)
            {
                if (it_osd.is_external)
                {
                    temp.statuses.push_back(it_osd.state_name);
                }
            }

            temp.tech_contacts.reserve(it->info_keyset_data.tech_contacts.size());
            BOOST_FOREACH(Fred::ObjectIdHandlePair it_oihp, it->info_keyset_data.tech_contacts)
            {
                temp.tech_contacts.push_back(it_oihp.handle);
            }
            ks_seq.content.push_back(temp);
        }
        return ks_seq;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return KeySetSeq();
}

WhoisImpl::Domain generate_obfuscate_domain_delete_candidate(const std::string& _handle)
{
    WhoisImpl::Domain temp;
    temp.fqdn = _handle;
    temp.statuses.push_back("deleteCandidate");
    //all the rest is default constructed
    return temp;
}

static WhoisImpl::Domain make_domain_from_info_data(
    const Fred::InfoDomainData& idd,
    Fred::OperationContext& ctx)
{
    WhoisImpl::Domain result;
    result.admin_contacts.reserve(idd.admin_contacts.size());
    BOOST_FOREACH(Fred::ObjectIdHandlePair it, idd.admin_contacts)
    {
        result.admin_contacts.push_back(it.handle);
    }
    result.changed              = idd.update_time;
    result.expire               = idd.expiration_date;
    result.fqdn                 = idd.fqdn;
    result.keyset               = idd.keyset.get_value_or_default().handle;
    result.last_transfer        = idd.transfer_time;
    result.nsset                = idd.nsset.get_value_or_default().handle;
    result.registered           = idd.creation_time;
    result.registrant           = idd.registrant.handle;
    result.sponsoring_registrar = idd.sponsoring_registrar_handle;
    const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(idd.id).exec(ctx);
    result.statuses.reserve(v_osd.size());
    BOOST_FOREACH(Fred::ObjectStateData it, v_osd)
    {
        if (it.is_external)
        {
            result.statuses.push_back(it.state_name);
        }
    }
    if (! idd.enum_domain_validation.isnull())
    {
        result.validated_to = idd.enum_domain_validation.get_value().validation_expiration;
        result.validated_to_time_estimate =
            ::Whois::domain_validation_expiration_datetime_estimate(
                ctx,
                idd.enum_domain_validation.get_value().validation_expiration);
        Optional<boost::posix_time::ptime> vtta =
                ::Whois::domain_validation_expiration_datetime_actual(ctx, idd.id);
        if (vtta.isset())
        {
            result.validated_to_time_actual = vtta.get_value();
        }
    }
    result.expire_time_estimate = ::Whois::domain_expiration_datetime_estimate(ctx, idd.expiration_date);

    Optional<boost::posix_time::ptime> eta = ::Whois::domain_expiration_datetime_actual(ctx, idd.id);
    if (eta.isset())
    {
        result.expire_time_actual = eta.get_value();
    }
    return result;
}

WhoisImpl::Domain Server_impl::get_domain_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    const Fred::CheckDomain check_domain(handle);
    try
    {
        try
        {
            if (check_domain.is_invalid_syntax())
            {
                throw InvalidLabel();
            }
            if (check_domain.is_bad_zone(ctx))
            {
                throw UnmanagedZone();
            }
            if (check_domain.is_bad_length(ctx))
            {
                throw TooManyLabels();
            }
            if (::Whois::is_domain_delete_pending(handle, ctx, "Europe/Prague"))
            {
                return Domain(generate_obfuscate_domain_delete_candidate(handle));
            }
            return make_domain_from_info_data(
                    Fred::InfoDomainByHandle(handle)
                        .exec( ctx, get_output_timezone() )
                        .info_domain_data,
                    ctx);
        }
        catch (const Fred::InfoDomainByHandle::Exception& e)
        {
            if (e.is_set_unknown_fqdn())
            {
                std::string conflicting_enum_domain;
                if (check_domain.is_registered(ctx, conflicting_enum_domain))
                {
                    //returns info of conflicting domain instead of requested domain
                    if (::Whois::is_domain_delete_pending(conflicting_enum_domain, ctx, "Europe/Prague"))
                    {
                        return Domain(generate_obfuscate_domain_delete_candidate(conflicting_enum_domain));
                    }
                    return make_domain_from_info_data(
                            Fred::InfoDomainByHandle(conflicting_enum_domain)
                                .exec( ctx, get_output_timezone() )
                                .info_domain_data,
                            ctx);
                }
                if (Fred::CheckDomain(handle).is_invalid_handle(ctx))
                {
                    throw InvalidLabel();
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
    return Domain();
}

static DomainSeq get_domains_by_(
    Fred::OperationContext& ctx,
    unsigned long limit,
    const std::vector<Fred::InfoDomainOutput>& domain_info)
{
    DomainSeq domain_seq;
    unsigned int non_del_pending_domains = 0;
    domain_seq.content.reserve( (domain_info.size() > limit) ? limit : domain_info.size());
    BOOST_FOREACH(const Fred::InfoDomainOutput& it, domain_info)
    {
        if (! ::Whois::is_domain_delete_pending(it.info_domain_data.fqdn, ctx, "Europe/Prague"))
        {
            if (++non_del_pending_domains > limit)
            {
                domain_seq.limit_exceeded = true;
                break;
            }
            domain_seq.content.push_back(make_domain_from_info_data(it.info_domain_data, ctx));
        }
    }
    return domain_seq;
}

DomainSeq Server_impl::get_domains_by_registrant(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByRegistrantHandle(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (Fred::ContactHandleState::SyntaxValidity::invalid ==
                    Fred::Contact::get_handle_syntax_validity(handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_admin_contact(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByAdminContactHandle(handle).set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (Fred::ContactHandleState::SyntaxValidity::invalid ==
                    Fred::Contact::get_handle_syntax_validity(handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_nsset(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByNssetHandle(handle).set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (Fred::CheckNsset(handle).is_invalid_handle())
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_keyset(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::InfoDomainOutput> domain_info =
                Fred::InfoDomainByKeysetHandle(handle).set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (Fred::CheckKeyset(handle).is_invalid_handle())
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

static std::vector<ObjectStatusDesc> get_object_status_descriptions(
    const std::string& lang,
    const std::string& type,
    const Server_impl& impl)
{
    LOGGING_CONTEXT(log_ctx, impl);

    Fred::OperationContextCreator ctx;
    try
    {
        const std::vector<Fred::ObjectStateDescription> states =
                Fred::GetObjectStateDescriptions(lang)
                    .set_object_type(type)
                    .set_external()
                    .exec(ctx);
        if (states.empty())
        {
            throw MissingLocalization();
        }

        std::vector<ObjectStatusDesc> state_seq;
        state_seq.reserve(states.size());
        ObjectStatusDesc tmp;
        BOOST_FOREACH(Fred::ObjectStateDescription it, states)
        {
            tmp.handle = it.handle;
            tmp.name = it.description;
            state_seq.push_back(tmp);
        }
        return state_seq;
    }
    catch (const MissingLocalization&)
    {
        throw;
    }
    catch (...)
    {
        log_and_rethrow_exception_handler(ctx);
    }
    return std::vector<ObjectStatusDesc>();
}

std::vector<ObjectStatusDesc> Server_impl::get_domain_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "domain", *this);
}

std::vector<ObjectStatusDesc> Server_impl::get_contact_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "contact", *this);
}

std::vector<ObjectStatusDesc> Server_impl::get_nsset_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "nsset", *this);
}

std::vector<ObjectStatusDesc> Server_impl::get_keyset_status_descriptions(const std::string& lang)
{
    return get_object_status_descriptions(lang, "keyset", *this);
}

}//WhoisImpl
}//Registry
