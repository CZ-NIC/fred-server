/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file domain_check_corba_conversions.h
 *  <++>
 */

#ifndef DOMAIN_CHECK_CORBA_CONVERSIONS_H_AF87AFA258D34CB79CA9D67847D16BCF
#define DOMAIN_CHECK_CORBA_CONVERSIONS_H_AF87AFA258D34CB79CA9D67847D16BCF

#include "util/corba_conversion.h"

#include "src/corba/EPP.hh"

#include "src/epp/domain/check_domain_localized.h"
#include "src/epp/domain/impl/domain_check_localization.h"

namespace Corba {

    /**
     * @returns data ordered the same way as input fqdns
     */
    ccReg::CheckResp wrap_Epp_Domain_DomainFqdnToDomainLocalizedRegistrationObstruction(
        const std::vector<std::string>& domain_fqdns,
        const Epp::Domain::DomainFqdnToDomainLocalizedRegistrationObstruction& _domain_fqdn_to_domain_localized_registration_obstruction
    );

}

#endif
