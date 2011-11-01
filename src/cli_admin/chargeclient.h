/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CHARGECLIENT_H_
#define _CHARGECLIENT_H_

#include "charge_params.h"

namespace Admin {
class ChargeClient {
private:
    ChargeRequestFeeArgs params;

public:
    ChargeClient(const ChargeRequestFeeArgs &p) : params(p)
    { }

    void runMethod();
    void chargeRequestFeeOneReg(const std::string &handle, const boost::gregorian::date &poll_msg_period_to);
    void chargeRequestFeeAllRegs(const std::string &except_handles, const boost::gregorian::date &poll_msg_period_to);

private:
    unsigned long long getRegistrarID(const std::string &handle);

};
}; // namespace Admin

#endif /*_CHARGECLIENT_H_*/
