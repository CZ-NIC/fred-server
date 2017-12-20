/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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

#include <string>

namespace LibFred {
namespace Contact {
namespace Verification {

//check conditions for cancel verification
bool check_contact_change_for_cancel_verification(
                    const std::string & contact_handle);

//cancel contact state
//conditionaly_identified if state conditionaly_identified is set
//and state identified if state identified is set
//throw if error
void contact_cancel_verification(
        const std::string & contact_handle);
}}}
