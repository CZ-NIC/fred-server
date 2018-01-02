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

#ifndef CREATE_LOW_CREDIT_MESSAGES_HH_473AF17B66214F1CADD7C676206E0C76
#define CREATE_LOW_CREDIT_MESSAGES_HH_473AF17B66214F1CADD7C676206E0C76

#include "src/libfred/opcontext.hh"

namespace LibFred {
namespace Poll {

struct CreateLowCreditMessages
{
    unsigned long long exec(OperationContext& _ctx) const;
};

} // namespace LibFred::Poll
} // namespace LibFred

#endif
