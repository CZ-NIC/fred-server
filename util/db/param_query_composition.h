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

#ifndef PARAM_QUERY_COMPOSITION_H_8d1b4342c9104204b6492eca97a762b2
#define PARAM_QUERY_COMPOSITION_H_8d1b4342c9104204b6492eca97a762b2

#include <vector>
#include <string>
#include <utility>
#include <stack>


#include <boost/shared_ptr.hpp>

#include "util/db/query_param.h"
#include "util/util.h"
namespace Database
{
    /**
     * Independent query parameter.
     * May be repeatedly added to ParamQuery instances.
     */
    class ReusableParameter
    {
        boost::shared_ptr<int> lid_;
        std::string type_;
        Database::QueryParam value_;
    public:
        /**
         * @param value is the value of query parameter of any type QueryParam can handle
         * @param type is the postgresql type, it will be used in parameterized query for parameter value cast like: $1::type
         */
        ReusableParameter(
            const Database::QueryParam& value,
            const std::string& type);

        /**
         * Gets identification of query parameter instance.
         * Identification is valid and unique in process scope.
         */
        boost::shared_ptr<int> get_lid() const;
        /**
         * Gets postgresql type of parameter.
         *
         */
        std::string get_type() const;
        Database::QueryParam get_value() const;
    };

    /**
     * Parameterized query or part thereof.
     * It might consists of text strings, query parameters and other ParamQuery instances.
     */
    class ParamQuery
    {
        class Element
        {
        public:
            enum TypeTag {PQE_NONE, PQE_STRING, PQE_PARAM, PQE_PARAM_REPETABLE};
        private:
            TypeTag tag_;
            boost::shared_ptr<int> lid_;
            std::string query_string_element_;
            Database::QueryParam query_param_element_;
        public:
            Element();

            Element& set_string(const std::string& val);

            Element& set_param(
                const Database::QueryParam& val,
                const std::string& pg_typname);

            Element& set_param(
                const Database::QueryParam& val,
                const std::string& pg_typname,
                const boost::shared_ptr<int>& lid);

            TypeTag get_tag() const;

            boost::shared_ptr<int> get_lid() const;

            std::string get_string() const;

            Database::QueryParam get_param() const;
        };

        std::vector<Element> param_query_;
    public:

        /**
         * Makes empty query.
         */
        ParamQuery();

        /**
         * Makes copy.
         */
        ParamQuery(const ParamQuery& val);

        /**
         * Makes query from text string, no parameters.
         */
        ParamQuery(const std::string& val);

        /**
         * Adds query part to the query, concatenates them into a single query.
         */
        ParamQuery& operator()(const ParamQuery& val);

        /**
         * Adds text string to the query.
         */
        ParamQuery& operator()(const std::string& val);

        /**
         * Adds query parameter with custom postgresql type.
         */
        ParamQuery& param(const Database::QueryParam& val,
            const std::string& pg_typname);

        /**
         * Adds query parameter of postgresql type "bigint".
         */
        ParamQuery& param_bigint(const Database::QueryParam& val);

        /**
         * Adds query parameter of postgresql type "numeric".
         */
        ParamQuery& param_numeric(const Database::QueryParam& val);

        /**
         * Adds query parameter of postgresql type "text".
         */
        ParamQuery& param_text(const Database::QueryParam& val);

        /**
         * Adds query parameter of postgresql type "timestamp".
         */
        ParamQuery& param_timestamp(const Database::QueryParam& val);

        /**
         * Adds query parameter of postgresql type "date".
         */
        ParamQuery& param_date(const Database::QueryParam& val);

        /**
         * Adds query parameter of postgresql type "bool".
         */
        ParamQuery& param_bool(const Database::QueryParam& val);

        /**
         * Adds independent query parameter instance.
         * The same independent query parameter instance can be added repeatedly.
         */
        ParamQuery& param(const Database::ReusableParameter& p);

        /**
         * Generates SQL query.
         * @return pair of SQL string with parameter numbers (first) and list of query parameter values (second).
         */
        std::pair<std::string,query_param_list> get_query() const;
    };
};

#endif //PARAM_QUERY_COMPOSITION_H_8d1b4342c9104204b6492eca97a762b2
