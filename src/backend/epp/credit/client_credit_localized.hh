/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef CLIENT_CREDIT_LOCALIZED_H_70DB3AA27BA936DA925FEB4F5E89D01B//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CLIENT_CREDIT_LOCALIZED_H_70DB3AA27BA936DA925FEB4F5E89D01B

#include "src/backend/epp/credit/client_credit_localized_response.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Credit {

ClientCreditLocalizedResponse client_credit_localized(const SessionData& _session_data);


} // namespace Epp::Credit
} // namespace Epp

#endif//CLIENT_CREDIT_LOCALIZED_H_70DB3AA27BA936DA925FEB4F5E89D01B
