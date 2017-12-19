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

#ifndef CREATE_LOW_CREDIT_MESSAGES_H_5AB9158286F848FDAE0FF93BD0D498E3
#define CREATE_LOW_CREDIT_MESSAGES_H_5AB9158286F848FDAE0FF93BD0D498E3

#include "src/fredlib/opcontext.h"

namespace Fred {
namespace Poll {

struct CreateLowCreditMessages
{
    unsigned long long exec(OperationContext& _ctx) const;
};

} // namespace Fred::Poll
} // namespace Fred

#endif
