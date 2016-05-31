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

#ifndef LOCALIZED_CHECK_H_1797E6168918A3D95558AB336DF4C982//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define LOCALIZED_CHECK_H_1797E6168918A3D95558AB336DF4C982

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/keyset/handle_check_result.h"
#include "util/db/nullable.h"

namespace Epp {
namespace Keyset {

struct LocalizedHandleCheckResult
{
    HandleCheckResult::Enum state;
    std::string description;
};

struct LocalizedHandleCheckResponse
{
    LocalizedHandleCheckResponse(
        const LocalizedSuccessResponse &_response,
        const std::map< std::string, Nullable< LocalizedHandleCheckResult > > &_results)
    :   ok_response(_response),
        results(_results) { }

    LocalizedSuccessResponse ok_response;
    std::map< std::string, Nullable< LocalizedHandleCheckResult > > results;
};

LocalizedHandleCheckResponse get_localized_check(
    const std::set< std::string > &_keyset_handles,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle);

}//namespace Epp::Keyset
}//namespace Epp


#endif//LOCALIZED_CHECK_H_1797E6168918A3D95558AB336DF4C982
