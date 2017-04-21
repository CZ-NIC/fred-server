/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef CONTACT_CORBA_CONVERSIONS_H_366458F9F5E84E6F994E130D1ADAB195
#define CONTACT_CORBA_CONVERSIONS_H_366458F9F5E84E6F994E130D1ADAB195

#include "src/corba/EPP.hh"

#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/contact_handle_registration_obstruction_localized.h"
#include "src/epp/contact/info_contact_localized.h"

#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace Fred {
namespace Corba {

void
unwrap_ContactChange(
        const ccReg::ContactChange& src,
        Epp::Contact::ContactChange& dst);


/**
 * @returns data ordered the same way as input contact_handles
 */
ccReg::CheckResp
wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map<std::string,
                boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized> >& contact_handle_check_results);


void
wrap_InfoContactLocalizedOutputData(
        const Epp::Contact::InfoContactLocalizedOutputData& src,
        ccReg::Contact& dst);


} // namespace Fred::Corba
} // namespace Fred

#endif
