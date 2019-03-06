/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CONTACT_CORBA_CONVERSIONS_HH_377DC2C828B7482897ED1EBE1A1AC7D1
#define CONTACT_CORBA_CONVERSIONS_HH_377DC2C828B7482897ED1EBE1A1AC7D1

#include "src/bin/corba/EPP.hh"

#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/contact_handle_registration_obstruction_localized.hh"
#include "src/backend/epp/contact/info_contact_localized.hh"

#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace LibFred {
namespace Corba {

void unwrap_ContactChange(const ccReg::ContactChange& src, Epp::Contact::ContactChange& dst);

void unwrap_ContactData(const ccReg::ContactData& src, Epp::Contact::ContactData& dst);

/**
 * @returns data ordered the same way as input contact_handles
 */
ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map< std::string,
                boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized> >& contact_handle_check_results);

void wrap_InfoContactLocalizedOutputData(
        const Epp::Contact::InfoContactLocalizedOutputData& src,
        ccReg::Contact& dst);

} // namespace LibFred::Corba
} // namespace LibFred

#endif
