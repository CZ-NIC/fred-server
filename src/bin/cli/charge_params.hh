/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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

#ifndef CHARGE_PARAMS_HH_19B07055E7ED4C319CEAF9E34F7CCD28
#define CHARGE_PARAMS_HH_19B07055E7ED4C319CEAF9E34F7CCD28

#include "src/util/types/optional.hh"


/**
 *  @registrar_params.h
 *  header of charging client implementation
 */

struct ChargeRequestFeeArgs {
    optional_string only_registrar;
    optional_string except_registrars;
    optional_string poll_msg_period_to;

    ChargeRequestFeeArgs()
    { }

    ChargeRequestFeeArgs(
            const optional_string& _only_registrar,
            const optional_string& _except_registrars,
            const optional_string& _poll_msg_period_to
            ) :
        only_registrar(_only_registrar),
        except_registrars(_except_registrars),
        poll_msg_period_to(_poll_msg_period_to)
    { }
};

#endif /*_CHARGE_PARAMS_*/
