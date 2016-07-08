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

#ifndef EPP_NSSET_CHECK_H_abaeee5023964a5a953d02d288c1ec33
#define EPP_NSSET_CHECK_H_abaeee5023964a5a953d02d288c1ec33


#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/nsset/nsset_handle_registration_obstruction.h"
#include <boost/optional.hpp>
#include "util/db/nullable.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct LocalizedNssetHandleRegistrationObstruction {
    const NssetHandleRegistrationObstruction::Enum state;
    const std::string description;

    LocalizedNssetHandleRegistrationObstruction(
        const NssetHandleRegistrationObstruction::Enum _state,
        const std::string& _description
    ) :
       state(_state),
       description(_description)
    { }
};

struct LocalizedCheckNssetResponse {
    const LocalizedSuccessResponse ok_response;
    const std::map<std::string, boost::optional<LocalizedNssetHandleRegistrationObstruction> > nsset_statuses;

    LocalizedCheckNssetResponse(
        const LocalizedSuccessResponse& _ok_response,
        const std::map<std::string, boost::optional<LocalizedNssetHandleRegistrationObstruction> >& _nsset_statuses
    ) :
        ok_response(_ok_response),
        nsset_statuses(_nsset_statuses)
    { }
};

LocalizedCheckNssetResponse nsset_check(
    const std::set<std::string>& _nsset_handles,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
);

}

#endif
