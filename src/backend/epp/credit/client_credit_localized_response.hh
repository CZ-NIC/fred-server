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
#ifndef CLIENT_CREDIT_LOCALIZED_RESPONSE_HH_C901948D489940458E827561535A7691
#define CLIENT_CREDIT_LOCALIZED_RESPONSE_HH_C901948D489940458E827561535A7691

#include "src/backend/epp/credit/client_credit_output_data.hh"
#include "src/backend/epp/epp_response_success_localized.hh"

namespace Epp {
namespace Credit {

struct ClientCreditLocalizedResponse
{
    ClientCreditLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const ClientCreditOutputData& _data)
        : epp_response_success_localized(_epp_response_success_localized),
          data(_data)
    { }
    EppResponseSuccessLocalized epp_response_success_localized;
    ClientCreditOutputData data;
};

} // namespace Epp::Contact
} // namespace Epp

#endif
