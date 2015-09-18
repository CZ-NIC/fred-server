/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  declaration of Fred::Messages::Generate class
 */

#ifndef GENERATE_H_919490122FEE4648D5B94BDAC5299EAC//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define GENERATE_H_919490122FEE4648D5B94BDAC5299EAC

#include "src/fredlib/opcontext.h"

namespace Fred {
namespace Messages {

struct CommChannel
{
    enum Value
    {
        SMS,
        EMAIL,
        LETTER,
    };
};

class Generate
{
public:
    template < CommChannel::Value COMM_CHANNEL >
    struct Into
    {
        static void exec(OperationContext &_ctx);
    };
};

}//namespace Fred::Messages
}//namespace Fred

#endif//GENERATE_H_919490122FEE4648D5B94BDAC5299EAC
