/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef PROCESS_PUBLIC_REQUEST_PERSONAL_INFO_HH_9CC977C4F3F344748AB821941D44B2F7
#define PROCESS_PUBLIC_REQUEST_PERSONAL_INFO_HH_9CC977C4F3F344748AB821941D44B2F7

#include "src/libfred/mailer.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/corba/file_manager_client.hh"
#include "src/libfred/file_transferer.hh"
#include "src/libfred/public_request/public_request_type_iface.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

void process_public_request_personal_info_resolved(
        unsigned long long _public_request_id,
        const LibFred::PublicRequestTypeIface& _public_request_type,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager,
        std::shared_ptr<LibFred::File::Transferer> _file_manager_client);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
