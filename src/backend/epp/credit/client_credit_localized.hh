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
#ifndef CLIENT_CREDIT_LOCALIZED_HH_59369CE558634FCF83F27F946FE5F9DD
#define CLIENT_CREDIT_LOCALIZED_HH_59369CE558634FCF83F27F946FE5F9DD

#include "src/backend/epp/credit/client_credit_localized_response.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Credit {

ClientCreditLocalizedResponse client_credit_localized(const SessionData& _session_data);


} // namespace Epp::Credit
} // namespace Epp

#endif
