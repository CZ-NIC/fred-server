/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
  * parametrized query composition.
 */


#include <vector>
#include <string>
#include <utility>
#include <stack>

#include <boost/shared_ptr.hpp>

#include "util/db/param_query_composition.h"
#include "util/db/query_param.h"
#include "util/util.h"
#include "util/map_at.h"
#include "util/optional_value.h"

namespace Database
{
    ReusableParameter::ReusableParameter(
        const Database::QueryParam& value,
        const std::string& type)
    : lid_(new int(0))
    , type_(type)
    , value_(value)
    {}

    boost::shared_ptr<int> ReusableParameter::get_lid() const
    {
        return lid_;
    }

    std::string ReusableParameter::get_type() const
    {
        return type_;
    }

    Database::QueryParam ReusableParameter::get_value() const
    {
        return value_;
    }


    ParamQuery::Element::Element()
    : tag_ (PQE_NONE)
    {}

    ParamQuery::Element& ParamQuery::Element::set_string(const std::string& val)
    {
        tag_ = PQE_STRING;
        query_string_element_ = val;
        return *this;
    }

    //non repeatable parameter
    ParamQuery::Element& ParamQuery::Element::set_param(
        const Database::QueryParam& val,
        const std::string& pg_typname)
    {
        tag_ = PQE_PARAM;
        boost::shared_ptr<int> new_lid(static_cast<int*>(0));
        lid_ = new_lid;
        query_string_element_ = pg_typname;
        query_param_element_ = val;
        return *this;
    }

    //repeatable parameter
    ParamQuery::Element& ParamQuery::Element::set_param(
        const Database::QueryParam& val,
        const std::string& pg_typname,
        const boost::shared_ptr<int>& lid)
    {
        tag_ = PQE_PARAM_REPETABLE;
        lid_ = lid;
        query_string_element_ = pg_typname;
        query_param_element_ = val;

        return *this;
    }

    ParamQuery::Element::TypeTag ParamQuery::Element::get_tag() const
    {
        return tag_;
    }

    boost::shared_ptr<int> ParamQuery::Element::get_lid() const
    {
        return lid_;
    }

    std::string ParamQuery::Element::get_string() const
    {
        return query_string_element_;
    }

    Database::QueryParam ParamQuery::Element::get_param() const
    {
        return query_param_element_;
    }


    ParamQuery::ParamQuery()
    {}

    ParamQuery::ParamQuery(const ParamQuery& val)
    : param_query_(val.param_query_)
    {}

    ParamQuery::ParamQuery(const std::string& val)
    : param_query_(1,Element().set_string(val))
    {}

    ParamQuery& ParamQuery::operator()(const ParamQuery& val)
    {
        param_query_.insert(param_query_.end(),
            val.param_query_.begin(), val.param_query_.end());
        return *this;
    }

    ParamQuery& ParamQuery::operator()(const std::string& val)
    {
        param_query_.push_back(Element().set_string(val));
        return *this;
    }

    ParamQuery& ParamQuery::param(const Database::QueryParam& val,
        const std::string& pg_typname)
    {
        param_query_.push_back(Element().set_param(val, pg_typname));
        return *this;
    }

    ParamQuery& ParamQuery::param_bigint(const Database::QueryParam& val)
    {
        return param(val, "bigint");
    }

    ParamQuery& ParamQuery::param_numeric(const Database::QueryParam& val)
    {
        return param(val, "numeric");
    }

    ParamQuery& ParamQuery::param_text(const Database::QueryParam& val)
    {
        return param(val, "text");
    }

    ParamQuery& ParamQuery::param_timestamp(const Database::QueryParam& val)
    {
        return param(val, "timestamp");
    }

    ParamQuery& ParamQuery::param_date(const Database::QueryParam& val)
    {
        return param(val, "date");
    }

    ParamQuery& ParamQuery::param_bool(const Database::QueryParam& val)
    {
        return param(val, "bool");
    }


    ParamQuery& ParamQuery::param(const Database::ReusableParameter& p)
    {
        param_query_.push_back(
            Element().set_param(p.get_value(),
                p.get_type(),
                p.get_lid())
            );
        return *this;
    }

    std::pair<std::string,query_param_list> ParamQuery::get_query() const
    {
        std::map<boost::shared_ptr<int>, std::string > param_lid_position;
        std::pair<std::string,query_param_list> query;

        for(std::vector<Element>::const_iterator
            ci = param_query_.begin(); ci != param_query_.end(); ++ci)
        {
            switch (ci->get_tag())
            {
                case Element::PQE_STRING:
                {
                    query.first += ci->get_string();
                }
                break;

                case Element::PQE_PARAM:
                {
                    const std::string pos = query.second.add(ci->get_param());
                    query.first += "$";
                    query.first += pos;//parameter position beginning from 1
                    query.first += "::";
                    query.first += ci->get_string();
                }
                break;

                case Element::PQE_PARAM_REPETABLE:
                {
                    boost::shared_ptr<int> lid = ci->get_lid();
                    Optional<std::string> pos = optional_map_at<Optional>(param_lid_position, lid);

                    if(!pos.isset()) //new parameter instance
                    {
                        pos = Optional<std::string>(query.second.add(ci->get_param()));
                        std::pair<std::map<boost::shared_ptr<int>, std::string >::iterator, bool> insert_result=
                        param_lid_position.insert(std::pair<boost::shared_ptr<int>, std::string>(lid, pos.get_value()));
                        if(!insert_result.second)
                        {
                            throw std::runtime_error("ParamQuery::Element::PQE_PARAM_REPETABLE insert failed");
                        }
                    }

                    query.first += "$";
                    query.first += pos.get_value();//parameter position beginning from 1
                    query.first += "::";
                    query.first += ci->get_string();
                }
                break;


              default:
                throw std::runtime_error(
                    "unknown ParamQuery::Element tag");
                break;
            };
        }

        return query;
    }

};

