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
 *  @file get_blocking_status_desc_list.h
 *  get blocking status desc list
 */

/*
ziska seznam vsech stavu blokovani objektu GetBlockingStatusDescList
  lang
*/

#ifndef GET_BLOCKING_STATUS_DESC_LIST_H_
#define GET_BLOCKING_STATUS_DESC_LIST_H_

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
    };//class GetBlockingStatusDescList


    class GetBlockingStatusDescList
    {
    public:
        GetBlockingStatusDescList();
        GetBlockingStatusDescList(const Optional< std::string > &_lang);
        GetBlockingStatusDescList& set_lang(const std::string &_lang);//language EN/CS

        typedef struct _StatusDesc
        {
            _StatusDesc() {}
            _StatusDesc(std::string _status, std::string _desc)
            :   status(_status),
                desc(_desc)
            {}
            _StatusDesc(const struct _StatusDesc &_src)
            :   status(_src.status),
                desc(_src.desc)
            {}
            std::string status;
            std::string desc;
        } StatusDesc;
        typedef std::vector< StatusDesc > StatusDescList;

        StatusDescList& exec(OperationContext &_ctx);

    private:
        Optional< std::string > lang_;
        StatusDescList status_desc_list_;
    };//class GetBlockingStatusDescList

//exception impl
    class GetBlockingStatusDescListException
    : public OperationExceptionImpl<GetBlockingStatusDescListException, 2048>
    {
    public:
        GetBlockingStatusDescListException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< GetBlockingStatusDescListException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"not found:lang"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class GetBlockingStatusDescListException

typedef GetBlockingStatusDescListException::OperationErrorType GetBlockingStatusDescListError;

}//namespace Fred

#endif//GET_BLOCKING_STATUS_DESC_LIST_H_
