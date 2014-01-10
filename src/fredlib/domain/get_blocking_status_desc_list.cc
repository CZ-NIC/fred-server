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
 *  @file get_blocking_status_desc_list.cc
 *  get blocking status desc list
 */

#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/object_state/get_object_state_id_map.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>

namespace Fred
{

    GetBlockingStatusDescList::GetBlockingStatusDescList()
    {}

    GetBlockingStatusDescList::GetBlockingStatusDescList(const Optional< std::string > &_lang,
        const Optional< ObjectType > &_object_type)
    :   lang_(_lang),
        object_type_(_object_type)
    {}

    GetBlockingStatusDescList& GetBlockingStatusDescList::set_lang(const std::string &_lang)
    {
        lang_ = _lang;
        return *this;
    }

    GetBlockingStatusDescList& GetBlockingStatusDescList::set_object_type(ObjectType _object_type)
    {
        object_type_ = _object_type;
        return *this;
    }

    GetBlockingStatusDescList::StatusDescList& GetBlockingStatusDescList::exec(OperationContext &_ctx)
    {
        status_desc_list_.clear();
        static const std::string defaultLang = "EN";
        const std::string lang = lang_.isset() ? lang_.get_value() : defaultLang;
        enum ResultColumnIndex
        {
            ID_IDX   = 0,
            NAME_IDX = 1,
            DESC_IDX = 2,
        };
        std::string query =
            "SELECT eos.id,eos.name,eosd.description "
            "FROM enum_object_states eos "
            "JOIN enum_object_states_desc eosd ON eosd.state_id=eos.id "
            "WHERE eos.manual AND "
                  "eos.name LIKE 'server%' AND "
                  "eos.name!='serverBlocked' AND "
                  "UPPER(eosd.lang)=UPPER($1)";
        Database::query_param_list param(lang);
        if (object_type_.isset()) {
            query += " AND $2=ANY(eos.types)";
            param(object_type_.get_value());
        }
        Database::Result nameDescResult = _ctx.get_conn().exec_params(query.c_str(), param);
        if (nameDescResult.size() <= 0) {
            BOOST_THROW_EXCEPTION(Exception().set_lang_not_found(lang));
        }
        status_desc_list_.reserve(nameDescResult.size());
        for (::size_t rowIdx = 0; rowIdx < nameDescResult.size(); ++rowIdx) {
            const StatusDesc statusDesc(nameDescResult[rowIdx][ID_IDX],
                                        nameDescResult[rowIdx][NAME_IDX],
                                        nameDescResult[rowIdx][DESC_IDX]);
            status_desc_list_.push_back(statusDesc);
        }
        return status_desc_list_;
    }//GetBlockingStatusDescList::exec

}//namespace Fred
