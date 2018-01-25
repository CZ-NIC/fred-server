#ifndef WHOIS2_IMPL_HH_4FB0D9C852264D3F8B9706DFA7E2873E
#define WHOIS2_IMPL_HH_4FB0D9C852264D3F8B9706DFA7E2873E

#include "src/bin/corba/Whois2.hh"
#include "src/backend/whois/whois.hh"

#include <string>

namespace Registry
{
namespace Whois
{

class Server_impl : public POA_Registry::Whois::WhoisIntf
{
public:
    Server_impl(const std::string& server_name_)
    : pimpl_(new Registry::WhoisImpl::Server_impl(server_name_))
    {}

    virtual ~Server_impl() {}

    Registrar* get_registrar_by_handle(const char* handle);

    RegistrarSeq* get_registrars();

    RegistrarGroupList* get_registrar_groups();

    RegistrarCertificationList* get_registrar_certification_list();

    ZoneFqdnList* get_managed_zone_list();

    Contact* get_contact_by_handle(const char* handle);

    NSSet* get_nsset_by_handle(const char* handle);

    NSSetSeq* get_nssets_by_ns(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded);

    NSSetSeq* get_nssets_by_tech_c(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded);

    NameServer* get_nameserver_by_fqdn(const char* handle);

    KeySet* get_keyset_by_handle(const char* handle);

    KeySetSeq* get_keysets_by_tech_c(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded);

    Domain* get_domain_by_handle(const char* handle);

    DomainSeq* get_domains_by_registrant(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded);

    DomainSeq* get_domains_by_admin_contact(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded);

    DomainSeq* get_domains_by_nsset(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded);

    DomainSeq* get_domains_by_keyset(
        const char* handle,
        ::CORBA::ULong limit,
        ::CORBA::Boolean& limit_exceeded);

    ObjectStatusDescSeq* get_domain_status_descriptions(const char* lang);
    ObjectStatusDescSeq* get_contact_status_descriptions(const char* lang);
    ObjectStatusDescSeq* get_nsset_status_descriptions(const char* lang);
    ObjectStatusDescSeq* get_keyset_status_descriptions(const char* lang);

private:
    const std::unique_ptr<Registry::WhoisImpl::Server_impl> pimpl_;
};

} // namespace Whois
} // namespace Registry
#endif