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
 *  perform object state request
 */

#ifndef PERFORM_OBJECT_STATE_REQUEST_H_
#define PERFORM_OBJECT_STATE_REQUEST_H_

#include "src/fredlib/opexception.h"

namespace Fred
{
    class PerformObjectStateRequest
    {
    public:
        PerformObjectStateRequest();
        PerformObjectStateRequest(const Optional< unsigned long long > &_object_id);
        PerformObjectStateRequest& set_object_id(unsigned long long _object_id);
        void exec(OperationContext &_ctx);

    private:
        Optional< unsigned long long > object_id_;
    };//class PerformObjectStateRequest

}//namespace Fred

#endif // PERFORM_OBJECT_STATE_REQUEST_H_
