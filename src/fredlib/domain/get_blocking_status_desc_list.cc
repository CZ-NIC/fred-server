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
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>

#ifndef __ASSERT_FUNCTION
#define __ASSERT_FUNCTION __PRETTY_FUNCTION__
#endif

#define MY_EXCEPTION_CLASS(DATA) GetBlockingStatusDescListException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA) GetBlockingStatusDescListError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

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
                std::string errmsg("|| not found:state:" + not_found);
                errmsg += " of type " + boost::lexical_cast< std::string >(object_type_);
                errmsg += " |";
                throw MY_EXCEPTION_CLASS(errmsg.c_str());
            }
        }
        return state_id_map_;
    }

    GetBlockingStatusDescList::GetBlockingStatusDescList()
    {}

    GetBlockingStatusDescList::GetBlockingStatusDescList(const Optional< std::string > &_lang)
    :   lang_(_lang)
    {}

    GetBlockingStatusDescList& GetBlockingStatusDescList::set_lang(const std::string &_lang)
    {
        lang_ = _lang;
        return *this;
    }

    GetBlockingStatusDescList::StatusDescList& GetBlockingStatusDescList::exec(OperationContext &_ctx)
    {
        status_desc_list_.clear();
        static const std::string defaultLang = "EN";
        const std::string lang = lang_.isset() ? lang_.get_value() : defaultLang;
        enum ResultColumnIndex
        {
            NAME_IDX = 0,
            DESC_IDX = 1,
        };
        Database::Result nameDescResult = _ctx.get_conn().exec_params(
          "SELECT eos.name,eosd.description "
          "FROM enum_object_states eos "
          "JOIN enum_object_states_desc eosd ON eosd.state_id=eos.id "
          "WHERE eos.manual AND "
                "eos.name LIKE 'server%' AND "
                "eos.name!='serverBlocked' AND "
                "eosd.lang=$1",
          Database::query_param_list(lang));
        if (nameDescResult.size() <= 0) {
            std::string errmsg("|| not found:lang: ");
            errmsg += boost::replace_all_copy(lang,"|", "[pipe]");//quote pipes
            errmsg += " |";
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
        }
        status_desc_list_.reserve(nameDescResult.size());
        for (::size_t rowIdx = 0; rowIdx < nameDescResult.size(); ++rowIdx) {
            const StatusDesc statusDesc(nameDescResult[rowIdx][NAME_IDX],
                                        nameDescResult[rowIdx][DESC_IDX]);
            status_desc_list_.push_back(statusDesc);
        }
        return status_desc_list_;
    }//GetBlockingStatusDescList::exec

}//namespace Fred
