#ifndef WHOIS2_IMPL_H_6545489794
#define WHOIS2_IMPL_H_6545489794

#include "src/corba/Whois2.hh"
#include <fredlib/fredlib.h>
#include "src/fredlib/registrar/info_registrar_data.h"

#include <string>

namespace Registry {
namespace Whois {

    Registrar wrap_registrar( const Fred::InfoRegistrarData& in);
    Contact wrap_contact(   const Fred::InfoContactData& in);
    Domain wrap_domain(const Fred::InfoDomainData& in);
    KeySet wrap_keyset(const Fred::InfoKeysetData& in);
    NSSet wrap_nsset(const Fred::InfoNssetData& in);

    class Server_impl :
        public POA_Registry::Whois::WhoisIntf
    {
        private:
            static const std::string output_timezone;
        public:
            virtual ~Server_impl() {};

            Registrar* get_registrar_by_handle(const char* handle);

            RegistrarSeq* get_registrars();

            RegistrarGroupList* get_registrar_groups();

            RegistrarCertificationList* get_registrar_certification_list();

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
    };

}
}
#endif
