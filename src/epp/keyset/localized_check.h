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

#ifndef LOCALIZED_CHECK_H_5B155FDF407D4AE3BE7089DC48B3C04E
#define LOCALIZED_CHECK_H_5B155FDF407D4AE3BE7089DC48B3C04E

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/keyset/handle_check_result.h"
#include "util/db/nullable.h"

namespace Epp {
namespace Keyset {
namespace Localized {

struct HandlesCheck
{
    struct Result
    {
        HandleCheckResult::Enum state;
        std::string description;
    };

    typedef std::map< std::string, Nullable< Result > > Results;

    HandlesCheck(const LocalizedSuccessResponse &_response, const Results &_results)
    :   ok_response(_response),
        results(_results) { }

    LocalizedSuccessResponse ok_response;
    Results results;
};

HandlesCheck check(
    const std::set< std::string > &_keyset_handles,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle);

}//namespace Epp::Keyset::Localized
}//namespace Epp::Keyset
}//namespace Epp


#endif
