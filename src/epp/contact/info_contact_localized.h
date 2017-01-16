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

#ifndef INFO_CONTACT_LOCALIZED_H_1AA5F0561FEF4660B5392F22BD64435C
#define INFO_CONTACT_LOCALIZED_H_1AA5F0561FEF4660B5392F22BD64435C

#include "src/epp/contact/impl/info_contact_localized_response.h"
#include "src/epp/contact/contact_disclose.h"
#include "src/epp/impl/notification_data.h"
#include "src/epp/impl/session_data.h"
#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {
namespace Contact {

InfoContactLocalizedResponse info_contact_localized(
        const std::string& _contact_handle,
        const SessionData& _session_data);

} // namespace Epp::Contact
} // namespace Epp

#endif
