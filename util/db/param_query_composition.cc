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

namespace Database
{
    ParamQueryParameter::ParamQueryParameter(
        const Database::QueryParam& value,
        const std::string& type)
    : lid_(new int(0))
    , type_(type)
    , value_(value)
    {}

    boost::shared_ptr<int> ParamQueryParameter::get_lid() const
    {
        return lid_;
    }

    std::string ParamQueryParameter::get_type() const
    {
        return type_;
    }

    Database::QueryParam ParamQueryParameter::get_value() const
    {
        return value_;
    }


    ParamQueryElement::ParamQueryElement()
    : tag_ (PQE_NONE)
    {}

    ParamQueryElement& ParamQueryElement::set_string(const std::string& val)
    {
        tag_ = PQE_STRING;
        query_string_element_ = val;
        return *this;
    }

    ParamQueryElement& ParamQueryElement::set_param(
        const Database::QueryParam& val,
        const std::string& pg_typname)
    {
        tag_ = PQE_PARAM;
        boost::shared_ptr<int> new_lid(new int(0));
        lid_ = new_lid;
        query_string_element_ = pg_typname;
        query_param_element_ = val;
        return *this;
    }

    ParamQueryElement& ParamQueryElement::set_param(
        const Database::QueryParam& val,
        const std::string& pg_typname,
        const boost::shared_ptr<int>& lid)
    {
        tag_ = PQE_PARAM;
        lid_ = lid;
        query_string_element_ = pg_typname;
        query_param_element_ = val;

        return *this;
    }

    ParamQueryElement::TypeTag ParamQueryElement::get_tag() const
    {
        return tag_;
    }

    boost::shared_ptr<int> ParamQueryElement::get_lid() const
    {
        return lid_;
    }

    std::string ParamQueryElement::get_string() const
    {
        return query_string_element_;
    }

    Database::QueryParam ParamQueryElement::get_param() const
    {
        return query_param_element_;
    }


    ParamQuery::ParamQuery()
    {}

    ParamQuery::ParamQuery(const ParamQuery& val)
    : param_query_(val.param_query_)
    {}

    ParamQuery::ParamQuery(const std::string& val)
    : param_query_(1,ParamQueryElement().set_string(val))
    {}

    ParamQuery& ParamQuery::operator()(const ParamQuery& val)
    {
        param_query_.insert(param_query_.end(),
            val.param_query_.begin(), val.param_query_.end());
        return *this;
    }

    ParamQuery& ParamQuery::operator()(const std::string& val)
    {
        param_query_.push_back(ParamQueryElement()
            .set_string(val));
        return *this;
    }

    ParamQuery& ParamQuery::param(const Database::QueryParam& val,
        const std::string& pg_typname)
    {
        param_query_.push_back(ParamQueryElement()
            .set_param(val, pg_typname));
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


    ParamQuery& ParamQuery::param(const Database::ParamQueryParameter& p)
    {
        param_query_.push_back(
            ParamQueryElement()
            .set_param(p.get_value(),
                p.get_type(),
                p.get_lid())
            );
        return *this;
    }

    std::pair<std::string,query_param_list> ParamQuery::get_query()
    {
        std::vector<boost::shared_ptr<int> > param_lid_position;
        std::pair<std::string,query_param_list> query;

        for(std::vector<ParamQueryElement>::const_iterator
            ci = param_query_.begin(); ci != param_query_.end(); ++ci)
        {
            switch (ci->get_tag())
            {
                case ParamQueryElement::PQE_STRING:
                {
                    query.first += ci->get_string();
                }
                break;

                case ParamQueryElement::PQE_PARAM:
                {
                    std::vector<boost::shared_ptr<int> >::iterator
                        param_lid_it = std::find(param_lid_position.begin(),
                            param_lid_position.end(), ci->get_lid());

                    if(param_lid_it == param_lid_position.end())//not found, got new param lid
                    {
                        param_lid_position.push_back(ci->get_lid());
                        query.second.push_back(ci->get_param());//append new param
                        param_lid_it = --param_lid_position.end();//set new lid iterator
                    }

                    query.first += "$";
                    query.first += boost::lexical_cast<std::string>(
                        param_lid_it - param_lid_position.begin() + 1);//parameter position beginning from 1
                    query.first += "::";
                    query.first += ci->get_string();
                }
                break;

              default:
                throw std::runtime_error(
                    "unknown ParamQueryElement tag");
                break;
            };
        }

        return query;
    }

    std::string ParamQuery::get_query_string()
    {
        return get_query().first;
    }

    QueryParams ParamQuery::get_query_params()
    {
        return get_query().second;
    }


};

