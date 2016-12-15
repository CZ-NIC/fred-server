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

#ifndef SRC_EPP_LOCALIZATION_78673784341_
#define SRC_EPP_LOCALIZATION_78673784341_

#include "src/epp/error.h"
#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/object_states_localized.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/session_lang.h"

#include "src/fredlib/opcontext.h"

#include <set>

namespace Epp {

LocalizedSuccessResponse create_localized_success_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    SessionLang::Enum _lang
);

LocalizedFailResponse create_localized_fail_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    const std::set<Error>& _errors,
    SessionLang::Enum _lang
);

LocalizedFailResponse create_localized_fail_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    const Error& _error,
    SessionLang::Enum _lang
);

std::map<std::string, std::string> localize_object_states_deprecated(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _state_handles,
    SessionLang::Enum _lang
);

ObjectStatesLocalized localize_object_states(
        Fred::OperationContext& _ctx,
        const std::set<Fred::Object_State::Enum>& _states,
        SessionLang::Enum _lang);

}

#endif
