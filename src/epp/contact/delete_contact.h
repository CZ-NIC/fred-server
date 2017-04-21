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

#ifndef DELETE_CONTACT_H_453EB59462824558BD5077709DA2806D
#define DELETE_CONTACT_H_453EB59462824558BD5077709DA2806D

#include "src/epp/contact/delete_contact_config_data.h"
#include "src/epp/session_data.h"
#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {
namespace Contact {

/**
 * If successful (no exception thrown) state requests of conact are performed.
 * In case of exception behaviour is undefined and transaction should be rolled back.
 *
 * @returns last contact history id before delete
 */
unsigned long long delete_contact(
        Fred::OperationContext& _ctx,
        const std::string& _handle,
        const DeleteContactConfigData& _delete_contact_config_data,
        const SessionData& _session_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
