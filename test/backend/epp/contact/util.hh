/*
 * Copyright (C) 2020  CZ.NIC, z. s. p. o.
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

#ifndef UTIL_HH_7D6C14B0C75543FCB3B5AB6CA91D2971
#define UTIL_HH_7D6C14B0C75543FCB3B5AB6CA91D2971

#include "src/backend/epp/contact/info_contact.hh"

#include "libfred/registrable_object/contact/info_contact_data.hh"

namespace Test {
namespace Backend {
namespace Epp {
namespace Contact {

boost::optional<::LibFred::PersonalIdUnion> get_ident(const boost::optional<::Epp::Contact::ContactIdent>& ident);

std::string get_ident_type(const ::Epp::Contact::HideableOptional<::Epp::Contact::ContactIdent>& ident);

std::string get_ident_value(const ::Epp::Contact::HideableOptional<::Epp::Contact::ContactIdent>& ident);

void check_equal(::Epp::Contact::InfoContactOutputData epp_data, const ::LibFred::InfoContactData& fred_data);

void check_equal_but_no_authinfopw(::Epp::Contact::InfoContactOutputData epp_data, const ::LibFred::InfoContactData& fred_data);

} // namespace Test::Backend::Epp::Contact
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
