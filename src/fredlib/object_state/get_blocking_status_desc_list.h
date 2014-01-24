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

#include "src/fredlib/object_state/create_object_state_request.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include <string>
#include <vector>
#include <map>

namespace Fred
{

    class GetBlockingStatusDescList
    {
    public:
        GetBlockingStatusDescList();
        GetBlockingStatusDescList(const Optional< std::string > &_lang, const Optional< ObjectType > &_object_type);
        GetBlockingStatusDescList& set_lang(const std::string &_lang);//language EN/CS
        GetBlockingStatusDescList& set_object_type(ObjectType _object_type);

        typedef struct _StatusDesc
        {
            _StatusDesc() {}
            _StatusDesc(TID _state_id, std::string _status, std::string _desc)
            :   state_id(_state_id),
                status(_status),
                desc(_desc)
            {}
            _StatusDesc(const struct _StatusDesc &_src)
            :   state_id(_src.state_id),
                status(_src.status),
                desc(_src.desc)
            {}
            TID state_id;
            std::string status;
            std::string desc;
        } StatusDesc;
        typedef std::vector< StatusDesc > StatusDescList;

        StatusDescList& exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(lang_not_found, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_lang_not_found<Exception>
        {};
    private:
        Optional< std::string > lang_;
        Optional< ObjectType > object_type_;
        StatusDescList status_desc_list_;
    };//class GetBlockingStatusDescList

}//namespace Fred

#endif//GET_BLOCKING_STATUS_DESC_LIST_H_
