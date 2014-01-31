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
 *  object state request locking
 */

#ifndef LOCK_OBJECT_STATE_REQUEST_LOCK_H_
#define LOCK_OBJECT_STATE_REQUEST_LOCK_H_

#include <string>

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Fred
{

    class LockObjectStateRequestLock
    {
    public:
        LockObjectStateRequestLock(unsigned long long state_id, unsigned long long object_id);
        LockObjectStateRequestLock(const std::string& state_name, unsigned long long object_id);
        void exec(OperationContext &_ctx);
    private:
        unsigned long long state_id_;
        Optional<std::string> state_name_;
        const unsigned long long object_id_;
    };

}//namespace Fred

#endif // LOCK_OBJECT_STATE_REQUEST_LOCK_H_
