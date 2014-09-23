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
 *  postgresql array utils
 */

#ifndef PG_ARRAY_UTILS_H_
#define PG_ARRAY_UTILS_H_

#include <stdexcept>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "util/db/nullable.h"

/**
 * postgresql array parser
 * implemented postgresql v9.1 array output syntax with backslash unquoting, except of box type
 */
class PgArray
{
    std::string array_literal_;

    void shift_array_content(std::string::size_type parsed_size, std::string& array_content)
    {
        if(array_content.size() > parsed_size && array_content[parsed_size] != ',')//error
        {
            throw std::runtime_error("unable to find delimiter ','");
        }
        else if (array_content.size() > parsed_size && array_content[parsed_size] == ',')//shift
        {
            array_content = array_content.substr(parsed_size+1);
        }
        else //it was the last
            array_content.clear();
    }

public:
    PgArray(const std::string& array_literal)
    : array_literal_(array_literal)
    {}

    std::vector<Nullable<std::string> > parse()/**< postgresql text[] output parser, other types convertible to text will also work except of postgresql box type with ; delimiter (is not implemented, and will cause undefined output)*/
    {
        std::vector<Nullable<std::string> > ret;
        if(!array_literal_.empty() && array_literal_[0] == '{' && array_literal_[array_literal_.length()-1] == '}')
        {
            std::string array_content = array_literal_.substr(1,array_literal_.length()-2);

            while(!array_content.empty())
            {
                if(array_content.substr(0,4) == "NULL")
                {
                    ret.push_back(Nullable<std::string>());//add null
                    shift_array_content(4, array_content);
                }
                else if(array_content[0] == '"')
                {
                    bool found_second_dq = false;
                    std::string::size_type i = 1;
                    for(; i < array_content.size(); ++i)
                    {
                        if(array_content[i] == '"' && array_content[i-1] != '\\')
                        {
                            found_second_dq = true;
                            ret.push_back(Nullable<std::string>(
                                boost::algorithm::replace_all_copy(
                                    boost::algorithm::replace_all_copy(
                                        array_content.substr(1,i-1)
                                    ,"\\\"", "\"")
                                ,"\\\\", "\\")//backslash unquote
                                ));//add quoted string
                            break;
                        }
                    }
                    if(!found_second_dq) throw std::runtime_error("unable to find second '\"'");

                    shift_array_content(i+1, array_content);
                }
                else if(array_content[0] == '{')
                {
                    std::string::size_type i = array_content.find('}');
                    if(i != std::string::npos)
                    {
                        ret.push_back(Nullable<std::string>(array_content.substr(0,i+1)));//add sub-array
                        shift_array_content(i+1, array_content);
                    }
                    else
                        throw std::runtime_error("unable to find second '}'");
                }
                else
                {
                    std::string::size_type i = array_content.find(',');
                    if(i != std::string::npos)
                    {
                        ret.push_back(Nullable<std::string>(array_content.substr(0,i)));//add unquoted string
                        shift_array_content(i, array_content);
                    }
                    else
                    {
                        ret.push_back(Nullable<std::string>(array_content));//string
                        array_content.clear();
                    }
                }
            }
            return ret;
        }
        throw std::runtime_error("not a pg array");
    }
};


#endif
