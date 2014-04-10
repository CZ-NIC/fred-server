/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  delete all contact states related to admin contact verification
 */

#ifndef CONTACT_VERIFICATION_CONTACT_STATES_DELETE_ALL_38972124897_
#define CONTACT_VERIFICATION_CONTACT_STATES_DELETE_ALL_38972124897_


#include "src/fredlib/opcontext.h"

namespace Admin
{
namespace AdminContactVerificationObjectStates
{
    void delete_all(Fred::OperationContext& ctx, unsigned long long contact_id);
}
}
#endif // #include guard end

