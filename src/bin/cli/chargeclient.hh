/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CHARGECLIENT_HH_40EFC56FBE424C79BD0719D5E3524610
#define CHARGECLIENT_HH_40EFC56FBE424C79BD0719D5E3524610

#include "src/bin/cli/charge_params.hh"

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
