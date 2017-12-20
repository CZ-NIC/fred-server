/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file clear_admin_object_state_request_id.h
 *  clear all administrative object state requests
 */

/*
administrativni zruseni vsech stavu blokovani objektu, (update do object_state_request) ClearAdminObjectStateRequestId
  M id objektu,
  M typ objektu,
  duvod
*/

#ifndef CLEAR_ADMIN_OBJECT_STATE_REQUEST_ID_H_
#define CLEAR_ADMIN_OBJECT_STATE_REQUEST_ID_H_

#include <string>

#include "src/libfred/opexception.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/object_state/typedefs.hh"
#include "src/util/optional_value.hh"

namespace LibFred
{

    class ClearAdminObjectStateRequestId
    {
    public:
        ClearAdminObjectStateRequestId(ObjectId _object_id);
        ClearAdminObjectStateRequestId(ObjectId _object_id,
            const std::string &_reason
            );
        ClearAdminObjectStateRequestId& set_reason(const std::string &_reason);
        void exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(object_id_not_found, ObjectId);
        DECLARE_EXCEPTION_DATA(server_blocked_absent, ObjectId);

        struct Exception
        :   virtual LibFred::OperationException,
            ExceptionData_object_id_not_found<Exception>,
            ExceptionData_server_blocked_absent<Exception>
        {};

    private:
        const ObjectId object_id_;
        Optional< std::string > reason_;
    };//class ClearAdminObjectStateRequestId

} // namespace LibFred

#endif//CLEAR_ADMINISTRATIVE_OBJECT_STATE_REQUEST_ID_H_
