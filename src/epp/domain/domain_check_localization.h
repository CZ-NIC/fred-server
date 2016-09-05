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
 *  @file domain_check_localization.h
 *  <++>
 */

#ifndef SRC_EPP_DOMAIN_DOMAIN_CHECK_LOCALIZATION_H
#define SRC_EPP_DOMAIN_DOMAIN_CHECK_LOCALIZATION_H

#include "src/epp/domain/domain_registration_obstruction.h"
#include "src/epp/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <boost/optional.hpp>
#include <map>
#include <string>

namespace Epp {

namespace Domain {

typedef std::map<std::string, Nullable<DomainRegistrationObstruction::Enum> > DomainFqdnToDomainRegistrationObstruction;
typedef std::map<std::string, boost::optional<DomainLocalizedRegistrationObstruction> > DomainFqdnToDomainLocalizedRegistrationObstruction;

DomainFqdnToDomainLocalizedRegistrationObstruction create_domain_fqdn_to_domain_localized_registration_obstruction(
    Fred::OperationContext &ctx,
    const DomainFqdnToDomainRegistrationObstruction &check_results,
    SessionLang::Enum lang);

}

}

#endif
