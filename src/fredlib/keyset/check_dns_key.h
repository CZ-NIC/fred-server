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

#ifndef CHECK_DNS_KEY_H_F75DC6271204DFF2C7678D1565A10144//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CHECK_DNS_KEY_H_F75DC6271204DFF2C7678D1565A10144

#include "src/fredlib/opcontext.h"

namespace Fred {
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

}//namespace Fred::DnsSec
}//namespace Fred

#endif//CHECK_DNS_KEY_H_F75DC6271204DFF2C7678D1565A10144
