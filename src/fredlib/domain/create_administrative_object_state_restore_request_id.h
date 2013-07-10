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
 *  @file create_administrative_object_state_restore_request_id.h
 *  create administrative object state restore request
 */

/*
administrativni nastaveni stavu blokovani objektu, (insert do object_state_request) CreateAdministrativeObjectStateRestoreRequestId
  M handle objektu,
  M typ objektu,
  M seznam stavu (jmena)
  od
  do
  poznamka
*/

#ifndef CREATE_ADMINISTRATIVE_OBJECT_STATE_RESTORE_REQUEST_ID_H_
#define CREATE_ADMINISTRATIVE_OBJECT_STATE_RESTORE_REQUEST_ID_H_

#include "fredlib/domain/create_object_state_request_id.h"

namespace Fred
{

    class CreateAdministrativeObjectStateRestoreRequestId
    {
    public:
        CreateAdministrativeObjectStateRestoreRequestId(ObjectId _object_id);
        CreateAdministrativeObjectStateRestoreRequestId(ObjectId _object_id,
            const std::string &_notice
            );
        CreateAdministrativeObjectStateRestoreRequestId& set_notice(const std::string &_notice);
        void exec(OperationContext &_ctx);

    private:
        ObjectStateId check_server_blocked_status_present(OperationContext &_ctx) const;
        const ObjectId object_id_;
        Optional< std::string > notice_;
    };//class CreateAdministrativeObjectStateRestoreRequestId

//exception impl
    class CreateAdministrativeObjectStateRestoreRequestIdException
    : public OperationExceptionImpl<CreateAdministrativeObjectStateRestoreRequestIdException, 2048>
    {
    public:
        CreateAdministrativeObjectStateRestoreRequestIdException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< CreateAdministrativeObjectStateRestoreRequestIdException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"invalid argument:state", "not found:state", "serverBlocked:absent", "not found:object_id"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class CreateAdministrativeObjectStateRestoreRequestIdException

typedef CreateAdministrativeObjectStateRestoreRequestIdException::OperationErrorType CreateAdministrativeObjectStateRestoreRequestIdError;

}//namespace Fred

#endif//CREATE_ADMINISTRATIVE_OBJECT_STATE_RESTORE_REQUEST_ID_H_
