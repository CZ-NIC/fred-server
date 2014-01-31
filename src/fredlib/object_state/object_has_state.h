/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  object state check
 */

#ifndef OBJECT_HAS_STATE_H_
#define OBJECT_HAS_STATE_H_

#include "src/fredlib/opexception.h"

namespace Fred
{

    class ObjectHasState
    {
    public:
        ObjectHasState(unsigned long long object_id, const std::string& state_name);
        bool exec(OperationContext& ctx);
    private:
        const unsigned long long object_id_;
        const std::string state_name_;
    };

}//namespace Fred

#endif // OBJECT_HAS_STATE_H_
