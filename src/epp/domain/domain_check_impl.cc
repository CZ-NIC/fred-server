#include "src/epp/domain/domain_check_impl.h"

#include "src/epp/domain/domain_check_localization.h"
#include "src/epp/domain/domain_registration_obstruction.h"
#include "src/fredlib/domain/domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/idn_utils.h"

#include <boost/foreach.hpp>

#include <idna.h>

namespace Epp {

namespace Domain {

namespace {

Nullable<DomainRegistrationObstruction::Enum> domain_get_registration_obstruction_by_fqdn(
    Fred::OperationContext& _ctx,
    const std::string _domain_fqdn)
{
    switch (Fred::Domain::get_domain_registrability_by_domain_fqdn(_ctx, _domain_fqdn)) {

        case Fred::Domain::DomainRegistrability::registered:
            return DomainRegistrationObstruction::registered;

        case Fred::Domain::DomainRegistrability::blacklisted:
            return DomainRegistrationObstruction::blacklisted;

        case Fred::Domain::DomainRegistrability::zone_not_in_registry:
            return DomainRegistrationObstruction::zone_not_in_registry;

        case Fred::Domain::DomainRegistrability::available:

            switch (Fred::Domain::get_domain_fqdn_syntax_validity(_ctx, _domain_fqdn)) {
                case Fred::Domain::DomainFqdnSyntaxValidity::invalid:
                    return DomainRegistrationObstruction::invalid_fqdn;
                case Fred::Domain::DomainFqdnSyntaxValidity::valid:
                    return Nullable<DomainRegistrationObstruction::Enum>();
            }
            throw std::logic_error("Unexpected Fred::Domain::DomainFqdnSyntaxValidity::Enum value.");

    }
    throw std::logic_error("Unexpected Fred::Domain::DomainRegistrability::Enum value.");
}

} // namespace Epp::Domain::{anonymous}

DomainFqdnToDomainRegistrationObstruction domain_check_impl(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _domain_fqdns,
    unsigned long long _registrar_id
) {

    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    DomainFqdnToDomainRegistrationObstruction domain_fqdn_to_domain_registration_obstruction;

    BOOST_FOREACH(const std::string& domain_fqdn, _domain_fqdns) {
        domain_fqdn_to_domain_registration_obstruction[domain_fqdn] = domain_get_registration_obstruction_by_fqdn(_ctx, domain_fqdn);
    }

    return domain_fqdn_to_domain_registration_obstruction;
}

}

}
