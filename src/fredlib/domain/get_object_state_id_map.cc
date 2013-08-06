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
 *  @file get_object_state_id_map.cc
 *  get object state id map
 */

#include "fredlib/domain/get_object_state_id_map.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>

namespace Fred
{

    GetObjectStateIdMap::GetObjectStateIdMap(const StatusList &_status_list, ObjectType _object_type)
    :   status_list_(_status_list),
        object_type_(_object_type)
    {}

    GetObjectStateIdMap::StateIdMap& GetObjectStateIdMap::exec(OperationContext &_ctx)
    {
        state_id_map_.clear();
        if (status_list_.empty()) {
            return state_id_map_;
        }
        StatusList::const_iterator pState = status_list_.begin();
        Database::query_param_list param(*pState);
        ++pState;
        std::ostringstream query;
        enum ResultColumnIdx
        {
            ID_IDX   = 0,
            NAME_IDX = 1,
        };
        query << "SELECT id,name "
                 "FROM enum_object_states "
                 "WHERE " << object_type_ << "=ANY(types) AND "
                     "name IN ($" << param.size() << "::text";
        while (pState != status_list_.end()) {
            param(*pState);
            query << ",$" << param.size() << "::text";
            ++pState;
        }
        query << ")";
        Database::Result id_name_result = _ctx.get_conn().exec_params(query.str(), param);
        for (::size_t rowIdx = 0; rowIdx < id_name_result.size(); ++rowIdx) {
            const Database::Row &row = id_name_result[rowIdx];
            state_id_map_[row[NAME_IDX]] = row[ID_IDX];
        }
        if (state_id_map_.size() < status_list_.size()) {
            std::string not_found;
            for (StatusList::const_iterator pState = status_list_.begin(); pState != status_list_.end(); ++pState) {
                if (state_id_map_.count(*pState) == 0) {
                    not_found += " " + *pState;
                }
            }
            if (!not_found.empty()) {
                BOOST_THROW_EXCEPTION(Exception().set_state_not_found(not_found));
            }
        }
        return state_id_map_;
    }

}//namespace Fred
