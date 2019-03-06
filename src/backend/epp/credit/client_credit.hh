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
#ifndef CLIENT_CREDIT_HH_25CA480C60E84583A20F3B5FB1D4B650
#define CLIENT_CREDIT_HH_25CA480C60E84583A20F3B5FB1D4B650

#include "src/backend/epp/credit/client_credit_output_data.hh"
#include "libfred/opcontext.hh"

namespace Epp {
namespace Credit {

/**
 * @throws ExceptionAuthErrorServerClosingConnection
 */
ClientCreditOutputData client_credit(
        LibFred::OperationContext& _ctx,
        unsigned long long _registrar_id);

} // namespace Epp::Credit
} // namespace Epp

#endif
