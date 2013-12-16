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
 *  @file create_admin_object_state_restore_request.h
 *  create administrative object state restore request
 */

/*
administrativni nastaveni stavu blokovani objektu, (insert do object_state_request) CreateAdminObjectStateRestoreRequest
  M handle objektu,
  M typ objektu,
  M seznam stavu (jmena)
  od
  do
  poznamka
*/

#ifndef CREATE_ADMIN_OBJECT_STATE_RESTORE_REQUEST_H_
#define CREATE_ADMIN_OBJECT_STATE_RESTORE_REQUEST_H_

#include "fredlib/domain/create_object_state_request.h"

namespace Fred
{

    class CreateAdminObjectStateRestoreRequest
    {
    public:
        CreateAdminObjectStateRestoreRequest(const std::string &_object_handle,
            ObjectType _object_type);
        CreateAdminObjectStateRestoreRequest(const std::string &_object_handle,
            ObjectType _object_type,
            const std::string &_notice
            );
        CreateAdminObjectStateRestoreRequest& set_notice(const std::string &_notice);
        ObjectId exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(state_not_found, std::string);
        DECLARE_EXCEPTION_DATA(server_blocked_absent, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_state_not_found<Exception>,
            ExceptionData_server_blocked_absent<Exception>
        {};
    private:
        ObjectStateId check_server_blocked_status_present(OperationContext &_ctx) const;
        const std::string object_handle_;
        const ObjectType object_type_;
        Optional< std::string > notice_;
    };//class CreateAdminObjectStateRestoreRequest

}//namespace Fred

#endif//CREATE_ADMINISTRATIVE_OBJECT_STATE_RESTORE_REQUEST_H_
