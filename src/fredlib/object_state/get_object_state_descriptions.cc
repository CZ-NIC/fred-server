/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  @file
 *  get object state descriptions
 */

#include <map>
#include <string>

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "get_object_state_descriptions.h"

namespace Fred
{

    GetObjectStateDescriptions::GetObjectStateDescriptions(const std::string& description_language)
    : description_language_(description_language)
    , external_states(false)
    {}

    GetObjectStateDescriptions& GetObjectStateDescriptions::set_external()
    {
        external_states = true;
        return *this;
    }

    GetObjectStateDescriptions& GetObjectStateDescriptions::set_object_type(const std::string& object_type)
    {
        object_type_ = object_type;
        return *this;
    }

    std::map<unsigned long long, std::string> GetObjectStateDescriptions::exec(OperationContext& ctx)
    {
        Database::query_param_list params;
        std::string sql = "SELECT eosd.state_id, COALESCE(eosd.description, '') ";
        sql += " FROM enum_object_states_desc eosd ";

        if(external_states || !object_type_.empty())
        {
            sql += " JOIN enum_object_states eos ON eos.id = eosd.state_id ";
        }

        sql += " WHERE UPPER(eosd.lang) = UPPER($";
        sql += params.add(description_language_);
        sql += "::text) ";

        if(external_states)
        {
            sql += " AND eos.external = TRUE ";
        }

        if(!object_type_.empty())
        {
            sql += " AND (SELECT id FROM enum_object_type WHERE name =$";
            sql += params.add(object_type_);
            sql += "::text) = ANY (eos.types) ";
        }

        Database::Result domain_state_descriptions_result = ctx.get_conn().exec_params(
                sql, params);

        std::map<unsigned long long, std::string> result;
        for(unsigned long long i = 0 ; i < domain_state_descriptions_result.size() ; ++i)
        {
            std::pair<std::map<unsigned long long, std::string>::iterator,bool> iret = result.insert(std::make_pair(
            static_cast<unsigned long long>(domain_state_descriptions_result[i][0])
            , static_cast<std::string>(domain_state_descriptions_result[i][1]))
            );
            if(!iret.second)//insert into map failed
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("insert into map of object state descriptions failed"));
            }
        }//for i

        return result;
    }
}//namespace Fred
