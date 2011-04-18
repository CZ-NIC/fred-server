/*  
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @validate_args.h
 *  program options validate overloads for
 *  option argument value custom validation
 */

#ifndef VALIDATE_ARGS_H_
#define VALIDATE_ARGS_H_

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "faked_args.h"
#include "handle_args.h"

//type alias checked by custom validator

struct Checked
{
    typedef std::string string;
    typedef unsigned long long ulonglong;
    typedef unsigned long ulong;
    typedef ulonglong id;
    typedef double fpnumber;
};

namespace boost
{
    namespace program_options
    {
        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::string* target_type, int)
        {
            using namespace boost;
            using namespace boost::program_options;
            using namespace std;

            // Make sure no previous assignment to 'a' was made.
            validators::check_first_occurrence(v);
            // Extract the first string from 'values'. If there is more than
            // one string, it's an error, and exception will be thrown.
            const Checked::string& s = validators::get_single_string(values);

            if (!s.empty() //string not empty
                && // and
                (
                    (*s.begin() == '\'' && *s.rbegin() == '\'') //quoted by '\'
                    ||                                          //or
                    (*s.begin() == '"' && *s.rbegin() == '"')   //quoted by '"'
                )
            )
            {
                v = any(s.substr(1, s.size()-2));
            }
            else
            {   if (*s.begin() == '-')
                    throw validation_error(
                            "invalid argument value, argument value may be missing"
                            ", unquoted arg can't start with '-' ");
                v = any(s);
            }
        }//validate checked_string

        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::ulonglong* target_type, int)
        {
            using namespace boost;
            using namespace boost::program_options;
            using namespace std;

            // Make sure no previous assignment to 'a' was made.
            validators::check_first_occurrence(v);
            // Extract the first string from 'values'. If there is more than
            // one string, it's an error, and exception will be thrown.
            const std::string& s = validators::get_single_string(values);

            if (!s.empty() //string not empty
                && // and
                (
                    (*s.begin() == '\'' && *s.rbegin() == '\'') //quoted by '\'
                    ||                                          //or
                    (*s.begin() == '"' && *s.rbegin() == '"')   //quoted by '"'
                )
            )
            {
                std::string str_inside_quotes = s.substr(1, s.size()-2);

                size_t found=str_inside_quotes.find_first_not_of("0123456789");
                if (found!=string::npos)
                {
                    throw validation_error(std::string("invalid quoted argument value: '")
                        + s.at(found) + "' arg may contain only numbers 0 - 9");
                }

                try
                {
                    boost::lexical_cast<unsigned long long>(str_inside_quotes);
                }
                catch(const std::exception& ex)
                {
                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
                }


                v = any(str_inside_quotes);
            }
            else
            {
                if (!s.empty()) //string not empty
                {
                    size_t found=s.find_first_not_of("0123456789");
                    if (found!=string::npos)
                    {
                        throw validation_error(std::string("invalid argument value: '")
                            + s.at(found) + "' arg may contain only numbers 0 - 9");
                    }
                    try
                    {
                        boost::lexical_cast<unsigned long long>(s);
                    }
                    catch(const std::exception& ex)
                    {
                        throw validation_error(
                                std::string("invalid quoted argument value cast"));
                    }
                }

                v = any(s);
            }
        }//validate checked_ulonglong

        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::ulong* target_type, int)
        {
            using namespace boost;
            using namespace boost::program_options;
            using namespace std;

            // Make sure no previous assignment to 'a' was made.
            validators::check_first_occurrence(v);
            // Extract the first string from 'values'. If there is more than
            // one string, it's an error, and exception will be thrown.
            const std::string& s = validators::get_single_string(values);

            if (!s.empty() //string not empty
                && // and
                (
                    (*s.begin() == '\'' && *s.rbegin() == '\'') //quoted by '\'
                    ||                                          //or
                    (*s.begin() == '"' && *s.rbegin() == '"')   //quoted by '"'
                )
            )
            {
                std::string str_inside_quotes = s.substr(1, s.size()-2);

                size_t found=str_inside_quotes.find_first_not_of("0123456789");
                if (found!=string::npos)
                {
                    throw validation_error(std::string("invalid quoted argument value: '")
                        + s.at(found) + "' arg may contain only numbers 0 - 9");
                }

                try
                {
                    boost::lexical_cast<unsigned long>(str_inside_quotes);
                }
                catch(const std::exception& ex)
                {
                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
                }

                v = any(str_inside_quotes);
            }
            else
            {
                if (!s.empty()) //string not empty
                {
                    size_t found=s.find_first_not_of("0123456789");
                    if (found!=string::npos)
                    {
                        throw validation_error(std::string("invalid argument value: '")
                            + s.at(found) + "' arg may contain only numbers 0 - 9");
                    }

                    try
                    {
                        boost::lexical_cast<unsigned long>(s);
                    }
                    catch(const std::exception& ex)
                    {
                        throw validation_error(
                                std::string("invalid argument value cast"));
                    }

                }

                v = any(s);
            }
        }//validate checked_ulong


        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::fpnumber* target_type, int)
        {
            using namespace boost;
            using namespace boost::program_options;
            using namespace std;

            // Make sure no previous assignment to 'a' was made.
            validators::check_first_occurrence(v);
            // Extract the first string from 'values'. If there is more than
            // one string, it's an error, and exception will be thrown.
            const std::string& s = validators::get_single_string(values);


            //fp number check
            static const boost::regex fp_number_regex("[-+]?([0-9]*\\.[0-9]+|[0-9]+)");

            if (!s.empty() //string not empty
                && // and
                (
                    (*s.begin() == '\'' && *s.rbegin() == '\'') //quoted by '\'
                    ||                                          //or
                    (*s.begin() == '"' && *s.rbegin() == '"')   //quoted by '"'
                )
            )
            {
                std::string str_inside_quotes = s.substr(1, s.size()-2);

                if (!boost::regex_match(str_inside_quotes, fp_number_regex))
                {
                    throw validation_error(
                            std::string("invalid quoted argument value"));
                }

                try
                {
                    boost::lexical_cast<double>(str_inside_quotes);
                }
                catch(const std::exception& ex)
                {
                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
                }

                v = any(str_inside_quotes);
            }
            else
            {
                if (!s.empty()) //string not empty
                {

                    if (!boost::regex_match(s, fp_number_regex))
                    {
                        throw validation_error(
                                std::string("invalid argument value"));
                    }

                    try
                    {
                        boost::lexical_cast<double>(s);
                    }
                    catch(const std::exception& ex)
                    {
                        throw validation_error(
                                std::string("invalid argument value cast"));
                    }
                }

                v = any(s);
            }
        }//validate checked_fpnumber
    }//namespace program_options
}//namespace boost
#endif //VALIDATE_ARGS_H_
