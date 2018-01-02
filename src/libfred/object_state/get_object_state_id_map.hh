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

#ifndef GET_OBJECT_STATE_ID_MAP_HH_564EE8F37C4F4F1A9ECF8B417EEE2C20
#define GET_OBJECT_STATE_ID_MAP_HH_564EE8F37C4F4F1A9ECF8B417EEE2C20

#include "src/libfred/opexception.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/object_state/typedefs.hh"

#include <string>
#include <map>

namespace LibFred
{

    class GetObjectStateIdMap
    {
    public:
        GetObjectStateIdMap(const StatusList &_status_list, ObjectType _object_type);

        typedef std::map< std::string, ObjectStateId > StateIdMap;
        StateIdMap& exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(state_not_found, std::string);

        struct Exception
        :   virtual LibFred::OperationException,
            ExceptionData_state_not_found<Exception>
        {};

        static StateIdMap& get_result(OperationContext &_ctx, const StatusList &_status_list, ObjectType _object_type,
                                      StateIdMap &_result);
    private:
        const StatusList status_list_;
        const ObjectType object_type_;
        StateIdMap state_id_map_;
    };//class GetObjectStateIdMap

} // namespace LibFred

#endif
