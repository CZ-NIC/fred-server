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

#ifndef CHECK_DOMAIN_CORBA_CONVERSIONS_H_23FE85CDCEFF48A581E649EB35C36B79
#define CHECK_DOMAIN_CORBA_CONVERSIONS_H_23FE85CDCEFF48A581E649EB35C36B79

#include "util/corba_conversion.h"

#include "src/corba/EPP.hh"

#include "src/epp/domain/check_domain_localized.h"

#include <map>
#include <string>
#include <vector>

namespace Corba {

/**
 * @returns data ordered the same way as input fqdns
 */
ccReg::CheckResp wrap_Epp_Domain_CheckDomainLocalizedResponse(
        const std::vector<std::string>& _domain_fqdns,
        const std::map<std::string, boost::optional<Epp::Domain::DomainLocalizedRegistrationObstruction> >& _domain_fqdn_to_domain_localized_registration_obstruction);

} // namespace Corba

#endif
