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
 *  @file get_object_state_id_map.h
 *  get object state id map
 */

/*
ziska relaci stav blokovani objektu -> id stavu blokovani objektu
  status_list
  object_type
*/

#ifndef GET_OBJECT_STATE_ID_MAP_H_
#define GET_OBJECT_STATE_ID_MAP_H_

#include "fredlib/domain/create_object_state_request.h"
#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include <string>
#include <vector>
#include <map>

namespace Fred
{

    class GetObjectStateIdMap
    {
    public:
        GetObjectStateIdMap(const StatusList &_status_list, ObjectType _object_type);

        typedef std::map< std::string, ObjectStateId > StateIdMap;
        StateIdMap& exec(OperationContext &_ctx);
    private:
        const StatusList status_list_;
        const ObjectType object_type_;
        StateIdMap state_id_map_;
    };//class GetObjectStateIdMap

//exception impl
    class GetObjectStateIdMapException
    : public OperationExceptionImpl< GetObjectStateIdMapException, 2048 >
    {
    public:
        GetObjectStateIdMapException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< GetObjectStateIdMapException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"not found:state"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class GetBlockingStatusDescListException

typedef GetObjectStateIdMapException::OperationErrorType GetObjectStateIdMapError;

}//namespace Fred

#endif//GET_OBJECT_STATE_ID_MAP_H_
