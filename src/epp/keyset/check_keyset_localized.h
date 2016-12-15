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
 *  @file
 */

#ifndef CHECK_KEYSET_LOCALIZED_H_231A7D4B89FC4368A15ED984120F7488
#define CHECK_KEYSET_LOCALIZED_H_231A7D4B89FC4368A15ED984120F7488

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/keyset/impl/keyset_handle_registration_obstruction.h"

#include "util/db/nullable.h"

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Keyset {
namespace Localized {

struct CheckKeysetLocalizedResponse
{
    struct Result
    {
        KeysetHandleRegistrationObstruction::Enum state;
        std::string description;

        Result()
        { }

        Result(
            const KeysetHandleRegistrationObstruction::Enum _state,
            const std::string& _description)
        :   state(_state),
            description(_description)
        { }
    };

    typedef std::map<std::string, Nullable<Result> > Results;

    CheckKeysetLocalizedResponse(
        const LocalizedSuccessResponse& _response,
        const Results& _results)
    :   ok_response(_response),
        results(_results)
    { }

    LocalizedSuccessResponse ok_response;
    Results results;
};

CheckKeysetLocalizedResponse check_keyset_localized(
        const std::set<std::string>& _keyset_handles,
        unsigned long long _registrar_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle);

} // namespace Epp::Keyset::Localized
} // namespace Epp::Keyset
} // namespace Epp


#endif
