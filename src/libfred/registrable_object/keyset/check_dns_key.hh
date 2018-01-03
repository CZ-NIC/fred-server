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

#ifndef CHECK_DNS_KEY_HH_D116922494E547B5B638A21AB084ACEB
#define CHECK_DNS_KEY_HH_D116922494E547B5B638A21AB084ACEB

#include "src/libfred/opcontext.hh"

namespace LibFred {
namespace DnsSec {

struct Algorithm
{
    enum Usability
    {
        usable,
        forbidden,
        invalid_value,
    };
};

Algorithm::Usability get_algorithm_usability(OperationContext &ctx, int algorithm_number);

} // namespace LibFred::DnsSec
} // namespace LibFred

#endif
