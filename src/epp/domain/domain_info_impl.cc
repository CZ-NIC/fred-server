#include "src/epp/domain/domain_info_impl.h"
#include "src/fredlib/domain/info_domain.h"

namespace Epp {

namespace Domain {

namespace {


} // namespace Epp::Domain::{anonymous}

DomainInfoOutputData domain_info_impl(
    Fred::OperationContext& ctx,
    const std::string& domain_fqdn,
    unsigned long long registrar_id
) {

    const bool registrar_is_authenticated = registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    DomainInfoOutputData domain_info_output_data;

    try {

        const Fred::InfoDomainData info_domain_data = Fred::InfoDomainByHandle(domain_fqdn).exec(ctx, "UTC").info_domain_data;

    } catch (const Fred::InfoDomainByHandle::Exception& e) {

        throw; // TODO

    }

    return domain_info_output_data;
}

}

}
