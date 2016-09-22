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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <idna.h>

namespace Epp {

namespace Domain {

namespace {

Nullable<DomainRegistrationObstruction::Enum> domain_get_registration_obstruction_by_fqdn(
    Fred::OperationContext& ctx,
    const unsigned long long registrar_id,
    const std::string domain_fqdn)
{
    try {
        Fred::Domain::DomainRegistrability::Enum domain_registrability
            = Fred::Domain::get_domain_registrability_by_domain_fqdn(ctx, domain_fqdn);

        switch (domain_registrability) {

            case Fred::Domain::DomainRegistrability::registered:
                return DomainRegistrationObstruction::registered;

            case Fred::Domain::DomainRegistrability::blacklisted:
                return DomainRegistrationObstruction::blacklisted;

            case Fred::Domain::DomainRegistrability::zone_not_in_registry:
                return DomainRegistrationObstruction::zone_not_in_registry;

            case Fred::Domain::DomainRegistrability::available:
                Fred::Domain::DomainFqdnSyntaxValidity::Enum domain_fqdn_syntax_validity
                     = Fred::Domain::get_domain_fqdn_syntax_validity(ctx, domain_fqdn);

                switch (domain_fqdn_syntax_validity) {
                    case Fred::Domain::DomainFqdnSyntaxValidity::invalid:
                        return DomainRegistrationObstruction::invalid_fqdn;
                    case Fred::Domain::DomainFqdnSyntaxValidity::valid:
                        return Nullable<DomainRegistrationObstruction::Enum>();
                }
                throw std::logic_error("Unexpected Fred::Domain::DomainFqdnSyntaxValidity::Enum value.");

        }
        throw std::logic_error("Unexpected Fred::Domain::DomainRegistrability::Enum value.");

    }
    catch (const Fred::Domain::ExceptionInvalidFqdn&) {
        return DomainRegistrationObstruction::invalid_fqdn;
    }
}

} // namespace Epp::Domain::{anonymous}

DomainFqdnToDomainRegistrationObstruction domain_check_impl(
    Fred::OperationContext& ctx,
    const std::set<std::string>& domain_fqdns,
    unsigned long long registrar_id
) {

    const bool registrar_is_authenticated = registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    DomainFqdnToDomainRegistrationObstruction domain_fqdn_to_domain_registration_obstruction;

    BOOST_FOREACH(const std::string &domain_fqdn, domain_fqdns) {
        domain_fqdn_to_domain_registration_obstruction[domain_fqdn] = domain_get_registration_obstruction_by_fqdn(ctx, registrar_id, domain_fqdn);
    }

    return domain_fqdn_to_domain_registration_obstruction;
}

}

}
