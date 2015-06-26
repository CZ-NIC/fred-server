#ifndef WHOIS2_IMPL_H_6545489794
#define WHOIS2_IMPL_H_6545489794

#include "src/corba/Whois2.hh"
#include <fredlib/fredlib.h>
#include "src/fredlib/registrar/info_registrar_data.h"

#include <string>

namespace Registry {
namespace Whois {

    NullableRegistrar*  wrap_registrar( const Fred::InfoRegistrarData& in);
    NullableContact*    wrap_contact(   const Fred::InfoContactData& in);
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

            virtual NullableRegistrar* get_registrar_by_handle(const char* handle);

            virtual NullableContact* get_contact_by_handle(const char* handle);

            virtual NullableNSSet* get_nsset_by_handle(const char* handle);
            virtual NSSetSeq* get_nssets_by_ns(const char* handle);
            virtual NSSetSeq* get_nssets_by_tech_c(const char* handle);

            virtual NullableNameServer* get_nameserver_by_fqdn(const char* handle);

            virtual NullableKeySet* get_keyset_by_handle(const char* handle);
            virtual KeySetSeq* get_keysets_by_tech_c(const char* handle);

            virtual NullableDomain* get_domain_by_handle(const char* handle);
            virtual DomainSeq* get_domains_by_registrant(const char* handle);
            virtual DomainSeq* get_domains_by_admin_contact(const char* handle);
            virtual DomainSeq* get_domains_by_nsset(const char* handle);
            virtual DomainSeq* get_domains_by_keyset(const char* handle);

            virtual ObjectStatusDescSeq* get_domain_status_descriptions(const char* lang);
            virtual ObjectStatusDescSeq* get_contact_status_descriptions(const char* lang);
            virtual ObjectStatusDescSeq* get_nsset_status_descriptions(const char* lang);
            virtual ObjectStatusDescSeq* get_keyset_status_descriptions(const char* lang);
    };

}
}
#endif
