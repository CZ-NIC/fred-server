/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
#ifndef CREDIT_CORBA_CONVERSIONS_HH_80A5B0A9DBF24992B74E0F2BD2B27268
#define CREDIT_CORBA_CONVERSIONS_HH_80A5B0A9DBF24992B74E0F2BD2B27268

#include "corba/EPP.hh"

#include "src/backend/epp/credit/client_credit_localized.hh"

namespace LibFred {
namespace Corba {

void wrap_ClientCreditOutputData(
        const Epp::Credit::ClientCreditOutputData& src,
        ccReg::ZoneCredit& dst);

} // namespace LibFred::Corba
} // namespace LibFred

#endif
