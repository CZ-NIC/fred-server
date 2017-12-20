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

#ifndef CREDIT_CORBA_CONVERSIONS_H_BA36AF5DA2D75E09E18C971A77551277//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREDIT_CORBA_CONVERSIONS_H_BA36AF5DA2D75E09E18C971A77551277

#include "src/bin/corba/EPP.hh"

#include "src/backend/epp/credit/client_credit_localized.hh"

namespace LibFred {
namespace Corba {

void wrap_ClientCreditOutputData(
        const Epp::Credit::ClientCreditOutputData& src,
        ccReg::ZoneCredit& dst);

} // namespace LibFred::Corba
} // namespace LibFred

#endif//CREDIT_CORBA_CONVERSIONS_H_BA36AF5DA2D75E09E18C971A77551277
