/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#ifndef EPP_GENERATE_AUTHINFOPW_6874612641200
#define EPP_GENERATE_AUTHINFOPW_6874612641200

#include <string>

namespace Epp {

    unsigned default_authinfopw_length() { return 8; }

    inline std::string generate_authinfopw(unsigned length) {
        return rdg.xnstring(length);
    }

}

#endif
