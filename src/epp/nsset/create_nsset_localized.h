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

#ifndef CREATE_NSSET_LOCALIZED_H_E012D98CCFF7446FB305B72D2F4B8417
#define CREATE_NSSET_LOCALIZED_H_E012D98CCFF7446FB305B72D2F4B8417

#include "src/epp/nsset/create_nsset_config_data.h"
#include "src/epp/nsset/create_nsset_input_data.h"
#include "src/epp/nsset/create_nsset_localized_response.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "util/optional_value.h"

namespace Epp {
namespace Nsset {

CreateNssetLocalizedResponse create_nsset_localized(
        const CreateNssetInputData& _create_nsset_input_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const CreateNssetConfigData& _config_data,
        const Optional<unsigned long long>& _logd_request_id);

} // namespace Epp::Nsset
} // namespace Epp

#endif
