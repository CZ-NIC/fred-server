/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef EPP_CONTACT_CHECK_H_63287364330
#define EPP_CONTACT_CHECK_H_63287364330

#include "src/epp/contact/ident_type.h"
#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/contact/contact_handle_registration_obstruction.h"
#include "util/lazy_nullable_type.h"
#include "util/db/nullable.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct LocalizedContactHandleRegistrationObstruction {
    const ContactHandleRegistrationObstruction::Enum state;
    const std::string description;

    LocalizedContactHandleRegistrationObstruction(
        const ContactHandleRegistrationObstruction::Enum _state,
        const std::string& _description
    ) :
       state(_state),
       description(_description)
    { }
};

struct LocalizedCheckContactResponse {
    const LocalizedSuccessResponse ok_response;
    const std::map<std::string, LazyNullable<LocalizedContactHandleRegistrationObstruction> > contact_statuses;

    LocalizedCheckContactResponse(
        const LocalizedSuccessResponse& _ok_response,
        const std::map<std::string, LazyNullable<LocalizedContactHandleRegistrationObstruction> >& _contact_statuses
    ) :
        ok_response(_ok_response),
        contact_statuses(_contact_statuses)
    { }
};

LocalizedCheckContactResponse contact_check(
    const std::set<std::string>& _contact_handles,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
);

}

#endif
