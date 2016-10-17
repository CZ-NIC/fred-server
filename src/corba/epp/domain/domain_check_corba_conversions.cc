#include "src/corba/epp/corba_conversions.h"

#include "src/corba/EPP.hh"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/domain/domain_check.h"
#include "src/epp/domain/domain_registration_obstruction.h"

#include "util/map_at.h"

namespace CorbaConversion {

namespace {

ccReg::CheckAvail wrap_Epp_Domain_DomainLocalizedRegistrationObstruction(
    const boost::optional<Epp::Domain::DomainLocalizedRegistrationObstruction>& obstruction
) {

    if (!obstruction) {
        return ccReg::NotExist;
    }

    switch (obstruction.get().state) {
        case Epp::Domain::DomainRegistrationObstruction::registered           : return ccReg::Exist;
        case Epp::Domain::DomainRegistrationObstruction::blacklisted          : return ccReg::BlackList;
        case Epp::Domain::DomainRegistrationObstruction::zone_not_in_registry : return ccReg::NotApplicable;
        case Epp::Domain::DomainRegistrationObstruction::invalid_fqdn         : return ccReg::BadFormat;
    }

    throw std::logic_error("Unexpected Epp::Domain::DomainRegistrationObstruction::Enum value.");
}

} // namespace CorbaConversion::{anonymous}

/**
 * @returns check results in the same order as input handles
 */
ccReg::CheckResp wrap_Epp_Domain_DomainFqdnToDomainLocalizedRegistrationObstruction(
    const std::vector<std::string>& _domain_fqdns,
    const Epp::Domain::DomainFqdnToDomainLocalizedRegistrationObstruction& _domain_fqdn_to_domain_localized_registration_obstruction
) {
    ccReg::CheckResp result;
    result.length(_domain_fqdns.size());

    CORBA::ULong result_idx = 0;
    for(
        std::vector<std::string>::const_iterator domain_fqdn_ptr = _domain_fqdns.begin();
        domain_fqdn_ptr != _domain_fqdns.end();
        ++domain_fqdn_ptr, ++result_idx
    ) {
        const boost::optional<Epp::Domain::DomainLocalizedRegistrationObstruction> domain_localized_registration_obstruction =
            map_at(_domain_fqdn_to_domain_localized_registration_obstruction, *domain_fqdn_ptr);

        result[result_idx].avail =
            wrap_Epp_Domain_DomainLocalizedRegistrationObstruction(domain_localized_registration_obstruction);

        result[result_idx].reason =
            Corba::wrap_string_to_corba_string(
                domain_localized_registration_obstruction ? domain_localized_registration_obstruction.get().description : ""
            );
    }

    return result;
}

}
