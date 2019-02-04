/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/whois/whois.hh"

#include "src/backend/whois/domain_expiration_datetime.hh"
#include "src/backend/whois/is_domain_delete_pending.hh"
#include "src/backend/whois/nameserver_exists.hh"
#include "src/backend/whois/registrar_certification.hh"
#include "src/backend/whois/registrar_group.hh"
#include "src/backend/whois/zone_list.hh"
#include "libfred/object/check_handle.hh"
#include "libfred/object_state/get_object_state_descriptions.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/domain_name.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_data.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_data.hh"
#include "libfred/registrable_object/keyset/keyset_dns_key.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_data.hh"
#include "libfred/registrable_object/nsset/nsset_dns_host.hh"
#include "libfred/registrar/check_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_output.hh"
#include "util/log/context.hh"
#include "util/random.hh"

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <algorithm>

namespace Fred {
namespace Backend {
namespace Whois {

static void log_and_rethrow_exception_handler(LibFred::OperationContext& ctx)
{
    try
    {
        throw;
    }
    catch (const LibFred::OperationException& ex)
    {
        ctx.get_log().warning(ex.what());
        ctx.get_log().warning(
                boost::algorithm::replace_all_copy(boost::diagnostic_information(ex),"\n", " "));
        throw;
    }
    catch (const LibFred::InternalError& ex)
    {
        ctx.get_log().error(boost::algorithm::replace_all_copy(ex.get_exception_stack_info(),"\n", " "));
        ctx.get_log().error(boost::algorithm::replace_all_copy(boost::diagnostic_information(ex),"\n", " "));
        ctx.get_log().error(ex.what());
        throw;
    }
    catch (const FatalException& ex)
    {
        ctx.get_log().error(ex.what());
        throw;
    }
    catch (const Exception& ex)
    {
        ctx.get_log().info(ex.what());
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

namespace {

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

} // namespace Fred::Backend::Whois::{anonymous}

Registrar make_registrar_from_info_data(const LibFred::InfoRegistrarData& ird)
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

    LibFred::OperationContextCreator ctx;
    try
    {
        try
        {
            return make_registrar_from_info_data(
                    LibFred::InfoRegistrarByHandle(handle)
                       .exec(ctx, get_output_timezone())
                       .info_registrar_data);
        }
        catch (const LibFred::InfoRegistrarByHandle::Exception& e)
        {
            if (e.is_set_unknown_registrar_handle())
            {
                if (LibFred::CheckRegistrar(handle).is_invalid_handle())
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

    LibFred::OperationContextCreator ctx;
    try
    {
        const std::vector<LibFred::InfoRegistrarOutput> v =
                LibFred::InfoRegistrarAllExceptSystem().exec(ctx, get_output_timezone());
        std::vector<Registrar> result;
        result.reserve(v.size());
        BOOST_FOREACH(LibFred::InfoRegistrarOutput it, v)
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

    LibFred::OperationContextCreator ctx;
    try
    {
        const std::map<std::string, std::vector<std::string> > groups = Fred::Backend::Whois::get_registrar_groups(ctx);
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

    LibFred::OperationContextCreator ctx;
    try
    {
        typedef std::vector<Fred::Backend::Whois::RegistrarCertificationData> CertificateList;
        const CertificateList v_rcd = get_registrar_certifications(ctx);
        std::vector<RegistrarCertification> result;
        result.reserve(v_rcd.size());
        BOOST_FOREACH(const RegistrarCertificationData& it, v_rcd)
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

    LibFred::OperationContextCreator ctx;
    try
    {
        return ::Fred::Backend::Whois::get_managed_zone_list(ctx);
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

    try
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            const LibFred::InfoContactData icd =
                    LibFred::InfoContactByHandle(handle).exec(ctx, get_output_timezone()).info_contact_data;
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

            const std::vector<LibFred::ObjectStateData> v_osd = LibFred::GetObjectStates(icd.id).exec(ctx);
            con.statuses.reserve(v_osd.size());
            BOOST_FOREACH(LibFred::ObjectStateData it, v_osd)
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
        catch (const LibFred::InfoContactByHandle::Exception& e)
        {
            LibFred::OperationContextCreator ctx;
            if (e.is_set_unknown_contact_handle())
            {
                if (LibFred::ContactHandleState::SyntaxValidity::invalid ==
                    LibFred::Contact::get_handle_syntax_validity(ctx, handle))
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
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return Contact();
}

static Whois::NSSet make_nsset_from_info_data(const LibFred::InfoNssetData& ind, LibFred::OperationContext& ctx)
{
    Whois::NSSet nss;
    nss.changed       = ind.update_time;
    nss.created       = ind.creation_time;
    nss.handle        = ind.handle;
    nss.last_transfer = ind.transfer_time;
    nss.nservers.reserve(ind.dns_hosts.size());
    BOOST_FOREACH(LibFred::DnsHost dns_host, ind.dns_hosts)
    {
        nss.nservers.push_back(Whois::NameServer(dns_host.get_fqdn(), dns_host.get_inet_addr()));
    }
    nss.sponsoring_registrar = ind.sponsoring_registrar_handle;
    BOOST_FOREACH(LibFred::ObjectIdHandlePair id_handle_pair, ind.tech_contacts)
    {
        nss.tech_contacts.push_back(id_handle_pair.handle);
    }
    const std::vector<LibFred::ObjectStateData> v_osd = LibFred::GetObjectStates(ind.id).exec(ctx);
    nss.statuses.reserve(v_osd.size());
    BOOST_FOREACH(LibFred::ObjectStateData nsset_state_data, v_osd)
    {
        if (nsset_state_data.is_external)
        {
            nss.statuses.push_back(nsset_state_data.state_name);
        }
    }
    return nss;
}

Whois::NSSet Server_impl::get_nsset_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            return make_nsset_from_info_data(
                    LibFred::InfoNssetByHandle(handle).exec(ctx, get_output_timezone()).info_nsset_data,
                    ctx);
        }
        catch (const LibFred::InfoNssetByHandle::Exception& e)
        {
            LibFred::OperationContextCreator ctx;
            if (e.is_set_unknown_handle())
            {
                if (LibFred::TestHandleOf< LibFred::Object_Type::nsset >(handle).is_invalid_handle(ctx))
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
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return Whois::NSSet();
}

static NSSetSeq get_nssets_by_(
    LibFred::OperationContext& ctx,
    const std::vector<LibFred::InfoNssetOutput>& nss_info,
    const std::string& handle,
    unsigned long limit)
{
    NSSetSeq nss_seq;
    std::vector<LibFred::InfoNssetOutput>::const_iterator it = nss_info.begin(), end;
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

    LibFred::OperationContextCreator ctx;
    try
    {
        const std::vector<LibFred::InfoNssetOutput> nss_info =
                LibFred::InfoNssetByDNSFqdn(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (nss_info.empty())
        {
            if (LibFred::CheckDomain(handle).is_invalid_syntax(ctx))
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

    try
    {
        LibFred::OperationContextCreator ctx;
        const std::vector<LibFred::InfoNssetOutput> nss_info =
                LibFred::InfoNssetByTechContactHandle(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (nss_info.empty())
        {
            if (LibFred::ContactHandleState::SyntaxValidity::invalid ==
                LibFred::Contact::get_handle_syntax_validity(ctx, handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_nssets_by_(ctx, nss_info, handle, limit);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return NSSetSeq();
}

NameServer Server_impl::get_nameserver_by_fqdn(const std::string& fqdn)
{
    LOGGING_CONTEXT(log_ctx, *this);

    const std::string no_root_dot_fqdn = LibFred::Zone::rem_trailing_dot(fqdn);

    LibFred::OperationContextCreator ctx;
    try
    {
        if (nameserver_exists(no_root_dot_fqdn, ctx))
        {
            NameServer temp;
            temp.fqdn = no_root_dot_fqdn;
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
            if (!LibFred::Domain::is_rfc1123_compliant_host_name(no_root_dot_fqdn))
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

Whois::KeySet Server_impl::get_keyset_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            const LibFred::InfoKeysetData ikd =
                    LibFred::InfoKeysetByHandle(handle).exec(ctx, get_output_timezone()).info_keyset_data;
            Whois::KeySet ks;
            ks.handle               = ikd.handle;
            ks.changed              = ikd.update_time;
            ks.created              = ikd.creation_time;
            ks.sponsoring_registrar = ikd.sponsoring_registrar_handle;
            ks.last_transfer        = ikd.transfer_time;
            ks.dns_keys.reserve(ikd.dns_keys.size());
            BOOST_FOREACH(LibFred::DnsKey dns_key, ikd.dns_keys)
            {
                ks.dns_keys.push_back(DNSKey(dns_key.get_flags(),
                                             dns_key.get_protocol(),
                                             dns_key.get_alg(),
                                             dns_key.get_key()));
            }

            const std::vector< LibFred::ObjectStateData > keyset_states = LibFred::GetObjectStates(ikd.id).exec(ctx);
            ks.statuses.reserve(keyset_states.size());
            for (std::vector< LibFred::ObjectStateData >::const_iterator state_ptr = keyset_states.begin();
                 state_ptr != keyset_states.end(); ++state_ptr)
            {
                if (state_ptr->is_external)
                {
                    ks.statuses.push_back(state_ptr->state_name);
                }
            }

            BOOST_FOREACH(LibFred::ObjectIdHandlePair it, ikd.tech_contacts)
            {
                ks.tech_contacts.push_back(it.handle);
            }
            return ks;
        }
        catch (const LibFred::InfoKeysetByHandle::Exception& e)
        {
            LibFred::OperationContextCreator ctx;
            if (e.is_set_unknown_handle())
            {
                if (LibFred::Keyset::get_handle_syntax_validity(ctx, handle) == LibFred::Keyset::HandleState::invalid)
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
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return KeySet();
}

KeySetSeq Server_impl::get_keysets_by_tech_c(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const std::vector<LibFred::InfoKeysetOutput> ks_info =
                LibFred::InfoKeysetByTechContactHandle(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        KeySetSeq ks_seq;
        if (ks_info.empty())
        {
            if (LibFred::ContactHandleState::SyntaxValidity::invalid ==
                LibFred::Contact::get_handle_syntax_validity(ctx, handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        ks_seq.limit_exceeded = limit < ks_info.size();
        const std::size_t ks_seq_size = ks_seq.limit_exceeded ? limit : ks_info.size();
        ks_seq.content.reserve(ks_seq_size);
        for (std::size_t idx = 0; idx < ks_seq_size; ++idx)
        {
            Whois::KeySet keyset;
            const LibFred::InfoKeysetData &info_keyset_data = ks_info[idx].info_keyset_data;
            keyset.changed = info_keyset_data.update_time;
            keyset.created = info_keyset_data.creation_time;

            keyset.dns_keys.reserve(info_keyset_data.dns_keys.size());
            BOOST_FOREACH(LibFred::DnsKey dns_key, info_keyset_data.dns_keys)
            {
                keyset.dns_keys.push_back(DNSKey(dns_key.get_flags(),
                                                 dns_key.get_protocol(),
                                                 dns_key.get_alg(),
                                                 dns_key.get_key()));
            }

            keyset.handle = info_keyset_data.handle;
            keyset.last_transfer = info_keyset_data.transfer_time;
            keyset.sponsoring_registrar = info_keyset_data.sponsoring_registrar_handle;

            const std::vector<LibFred::ObjectStateData> v_osd =
                    LibFred::GetObjectStates(info_keyset_data.id).exec(ctx);
            keyset.statuses.reserve(v_osd.size());
            BOOST_FOREACH(LibFred::ObjectStateData it_osd, v_osd)
            {
                if (it_osd.is_external)
                {
                    keyset.statuses.push_back(it_osd.state_name);
                }
            }

            keyset.tech_contacts.reserve(info_keyset_data.tech_contacts.size());
            BOOST_FOREACH(LibFred::ObjectIdHandlePair it_oihp, info_keyset_data.tech_contacts)
            {
                keyset.tech_contacts.push_back(it_oihp.handle);
            }
            ks_seq.content.push_back(keyset);
        }
        return ks_seq;
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return KeySetSeq();
}

static Whois::Domain make_domain_from_info_data(
    const LibFred::InfoDomainData& idd,
    LibFred::OperationContext& ctx)
{
    Whois::Domain result;
    result.admin_contacts.reserve(idd.admin_contacts.size());
    BOOST_FOREACH(LibFred::ObjectIdHandlePair it, idd.admin_contacts)
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
    const std::vector<LibFred::ObjectStateData> v_osd = LibFred::GetObjectStates(idd.id).exec(ctx);
    result.statuses.reserve(v_osd.size());
    BOOST_FOREACH(LibFred::ObjectStateData it, v_osd)
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
            domain_validation_expiration_datetime_estimate(
                ctx,
                idd.enum_domain_validation.get_value().validation_expiration);
        Optional<boost::posix_time::ptime> vtta =
                domain_validation_expiration_datetime_actual(ctx, idd.id);
        if (vtta.isset())
        {
            result.validated_to_time_actual = vtta.get_value();
        }
    }
    result.expire_time_estimate = domain_expiration_datetime_estimate(ctx, idd.expiration_date);

    Optional<boost::posix_time::ptime> eta = domain_expiration_datetime_actual(ctx, idd.id);
    if (eta.isset())
    {
        result.expire_time_actual = eta.get_value();
    }
    return result;
}

Whois::Domain Server_impl::get_domain_by_handle(const std::string& handle)
{
    LOGGING_CONTEXT(log_ctx, *this);

    const std::string no_root_dot_fqdn = LibFred::Zone::rem_trailing_dot(handle);

    LibFred::OperationContextCreator ctx;
    const LibFred::CheckDomain check_domain(no_root_dot_fqdn);
    try
    {
        try
        {
            if (handle.empty() || (handle.length() > 255))
            {
                throw InvalidLabel();
            }
            if (check_domain.is_bad_zone(ctx))
            {
                throw UnmanagedZone();
            }
            if (is_domain_delete_pending(handle, ctx, "Europe/Prague"))
            {
                throw ObjectDeleteCandidate();
            }
            return make_domain_from_info_data(
                    LibFred::InfoDomainByFqdn(no_root_dot_fqdn)
                        .exec( ctx, get_output_timezone() )
                        .info_domain_data,
                    ctx);
        }
        catch (const LibFred::InfoDomainByFqdn::Exception& e)
        {
            if (e.is_set_unknown_fqdn())
            {
                std::string conflicting_enum_domain;
                if (check_domain.is_registered(ctx, conflicting_enum_domain))
                {
                    //returns info of conflicting domain instead of requested domain
                    if (is_domain_delete_pending(conflicting_enum_domain, ctx, "Europe/Prague"))
                    {
                        throw ObjectDeleteCandidate();
                    }
                    return make_domain_from_info_data(
                            LibFred::InfoDomainByFqdn(conflicting_enum_domain)
                                .exec( ctx, get_output_timezone() )
                                .info_domain_data,
                            ctx);
                }
                if (check_domain.is_bad_length(ctx))
                {
                    throw TooManyLabels();
                }
                if (LibFred::CheckDomain(no_root_dot_fqdn).is_invalid_syntax(ctx))
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
    LibFred::OperationContext& ctx,
    unsigned long limit,
    const std::vector<LibFred::InfoDomainOutput>& domain_info)
{
    DomainSeq domain_seq;
    unsigned int non_del_pending_domains = 0;
    domain_seq.content.reserve( (domain_info.size() > limit) ? limit : domain_info.size());
    BOOST_FOREACH(const LibFred::InfoDomainOutput& it, domain_info)
    {
        if (!is_domain_delete_pending(it.info_domain_data.fqdn, ctx, "Europe/Prague"))
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

    try
    {
        LibFred::OperationContextCreator ctx;
        const std::vector<LibFred::InfoDomainOutput> domain_info =
                LibFred::InfoDomainByRegistrantHandle(handle)
                    .set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (LibFred::ContactHandleState::SyntaxValidity::invalid ==
                LibFred::Contact::get_handle_syntax_validity(ctx, handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_admin_contact(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const std::vector<LibFred::InfoDomainOutput> domain_info =
                LibFred::InfoDomainByAdminContactHandle(handle).set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (LibFred::ContactHandleState::SyntaxValidity::invalid ==
                LibFred::Contact::get_handle_syntax_validity(ctx, handle))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_nsset(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const std::vector<LibFred::InfoDomainOutput> domain_info =
                LibFred::InfoDomainByNssetHandle(handle).set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (LibFred::TestHandleOf< LibFred::Object_Type::nsset >(handle).is_invalid_handle(ctx))
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        log_and_rethrow_exception_handler(ctx);
    }
    return DomainSeq();
}

DomainSeq Server_impl::get_domains_by_keyset(const std::string& handle, unsigned long limit)
{
    LOGGING_CONTEXT(log_ctx, *this);

    try
    {
        LibFred::OperationContextCreator ctx;
        const std::vector<LibFred::InfoDomainOutput> domain_info =
                LibFred::InfoDomainByKeysetHandle(handle).set_limit(limit + 1)
                    .exec(ctx, get_output_timezone());
        if (domain_info.empty())
        {
            if (LibFred::Keyset::get_handle_syntax_validity(ctx, handle) == LibFred::Keyset::HandleState::invalid)
            {
                throw InvalidHandle();
            }
            throw ObjectNotExists();
        }
        return get_domains_by_(ctx, limit, domain_info);
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
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

    LibFred::OperationContextCreator ctx;
    try
    {
        const std::vector<LibFred::ObjectStateDescription> states =
                LibFred::GetObjectStateDescriptions(lang)
                    .set_object_type(type)
                    .set_external()
                    .exec(ctx);
        if (states.empty())
        {
            throw MissingLocalization();
        }

        std::vector<ObjectStatusDesc> state_seq;
        state_seq.reserve(states.size());
        BOOST_FOREACH(LibFred::ObjectStateDescription state, states)
        {
            state_seq.push_back(ObjectStatusDesc(state.handle, state.description));
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

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
