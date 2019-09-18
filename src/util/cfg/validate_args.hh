/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @validate_args.h
 *  program options validate overloads for
 *  option argument value custom validation
 */

#ifndef VALIDATE_ARGS_HH_BAA5803FB80F4518ABC0DC0372B55DCF
#define VALIDATE_ARGS_HH_BAA5803FB80F4518ABC0DC0372B55DCF

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <memory>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/strong_typedef.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"
#include "src/util/cfg/checked_types.hh"

template<class Char, class Traits>
std::basic_ostream<Char, Traits>&
operator<<(std::basic_ostream<Char, Traits> &o, const Checked::string_fpnumber &s)
{
    return o << (s.to_string());
}

template<class Char, class Traits>
std::basic_istream<Char, Traits>&
operator>>(std::basic_istream<Char, Traits> &i, Checked::string_fpnumber &s)
{
    std::string tmp;
    i >> tmp;
    s = tmp;
    return i;
}


namespace boost
{
    namespace program_options
    {
        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::string*, int)
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
            {
                if (*s.begin() == '-')

#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(
                            "invalid argument value, argument value may be missing"
                            ", unquoted arg can't start with '-' ");
#endif

                v = any(s);

            }
        }//validate checked_string

        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::ulonglong*, int)
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
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(std::string("invalid quoted argument value: '")
                        + s.at(found) + "' arg may contain only numbers 0 - 9");
#endif
                }

                try
                {
                    boost::lexical_cast<unsigned long long>(str_inside_quotes);
                }
                catch(const std::exception& ex)
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
#endif
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
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(std::string("invalid argument value: '")
                            + s.at(found) + "' arg may contain only numbers 0 - 9");
#endif
                    }
                    try
                    {
                        boost::lexical_cast<unsigned long long>(s);
                    }
                    catch(const std::exception& ex)
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid quoted argument value cast"));
#endif
                    }
                }

                v = any(s);
            }
        }//validate checked_ulonglong

        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::ulong*, int)
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
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(std::string("invalid quoted argument value: '")
                        + s.at(found) + "' arg may contain only numbers 0 - 9");
#endif
                }

                try
                {
                    boost::lexical_cast<unsigned long>(str_inside_quotes);
                }
                catch(const std::exception& ex)
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
#endif
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
#if ( BOOST_VERSION > 104100 )
                        boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(std::string("invalid argument value: '")
                            + s.at(found) + "' arg may contain only numbers 0 - 9");
#endif
                    }

                    try
                    {
                        boost::lexical_cast<unsigned long>(s);
                    }
                    catch(const std::exception& ex)
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value cast"));
#endif
                    }

                }

                v = any(s);
            }
        }//validate checked_ulong


        //overload of the 'validate' function for the user-defined class
        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::fpnumber*, int)
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
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(
                            std::string("invalid quoted argument value"));
#endif
                }

                try
                {
                    boost::lexical_cast<double>(str_inside_quotes);
                }
                catch(const std::exception& ex)
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else

                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
#endif
                }

                v = any(str_inside_quotes);
            }
            else
            {
                if (!s.empty()) //string not empty
                {

                    if (!boost::regex_match(s, fp_number_regex))
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value"));
#endif
                    }

                    try
                    {
                        boost::lexical_cast<double>(s);
                    }
                    catch(const std::exception& ex)
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value cast"));
#endif
                    }
                }

                v = any(s);
            }
        }//validate checked_fpnumber

        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::string_fpnumber*, int)
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
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(
                            std::string("invalid quoted argument value"));
#endif
                }

                try
                {
                    boost::lexical_cast<double>(str_inside_quotes);
                }
                catch(const std::exception& ex)
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else

                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
#endif
                }

                v = any(str_inside_quotes);
            }
            else
            {
                if (!s.empty()) //string not empty
                {

                    if (!boost::regex_match(s, fp_number_regex))
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value"));
#endif
                    }

                    try
                    {
                        boost::lexical_cast<double>(s);
                    }
                    catch(const std::exception& ex)
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value cast"));
#endif
                    }
                }

                v = any(s);
            }
        }

        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::date*, int)
        {
            using namespace boost;
            using namespace boost::program_options;
            using namespace std;

            // Make sure no previous assignment to 'a' was made.
            validators::check_first_occurrence(v);
            // Extract the first string from 'values'. If there is more than
            // one string, it's an error, and exception will be thrown.
            const std::string& s = validators::get_single_string(values);


            //date check YYYY-MM-DD or YYYY/MM/DD
            static const boost::regex date_regex("((19|20)\\d\\d)([/-])(0[1-9]|1[012])([/-])(0[1-9]|[12][0-9]|3[01])");

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

                if (!boost::regex_match(str_inside_quotes, date_regex))
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(
                            std::string("invalid quoted argument value"));
#endif
                }

                try
                {
                    boost::gregorian::date tmp_date = boost::gregorian::from_string(str_inside_quotes);
                    if (tmp_date.is_special()) throw std::runtime_error("invalid date");
                }
                catch(const std::exception& ex)
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else

                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
#endif
                }

                v = any(str_inside_quotes);
            }
            else
            {
                if (!s.empty()) //string not empty
                {

                    if (!boost::regex_match(s, date_regex))
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value"));
#endif
                    }

                    try
                    {
                        boost::gregorian::date tmp_date = boost::gregorian::from_string(s);
                        if (tmp_date.is_special()) throw std::runtime_error("invalid date");
                    }
                    catch(const std::exception& ex)
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value cast"));
#endif
                    }
                }

                v = any(s);
            }
        }//validate checked_date



        void validate(boost::any& v,
                      const std::vector<std::string>& values,
                      Checked::ptime*, int)
        {
            using namespace boost;
            using namespace boost::program_options;
            using namespace std;

            // Make sure no previous assignment to 'a' was made.
            validators::check_first_occurrence(v);
            // Extract the first string from 'values'. If there is more than
            // one string, it's an error, and exception will be thrown.
            const std::string& s = validators::get_single_string(values);


            //ptime check YYYY-MM-DD hh:mm:ss or YYYY/MM/DD hh:mm:ss
            static const boost::regex ptime_regex(
                "((19|20)\\d\\d)([/-])(0[1-9]|1[012])([/-])(0[1-9]|[12][0-9]|3[01]) (0[0-9]|1[0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])");

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

                if (!boost::regex_match(str_inside_quotes, ptime_regex))
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                    throw validation_error(
                            std::string("invalid quoted argument value"));
#endif
                }

                try
                {
                    boost::posix_time::ptime tmp_ptime = boost::posix_time::time_from_string(str_inside_quotes);
                    if (tmp_ptime.is_special()) throw std::runtime_error("invalid ptime");
                }
                catch(const std::exception& ex)
                {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else

                    throw validation_error(
                            std::string("invalid quoted argument value cast"));
#endif
                }

                v = any(str_inside_quotes);
            }
            else
            {
                if (!s.empty()) //string not empty
                {

                    if (!boost::regex_match(s, ptime_regex))
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value"));
#endif
                    }

                    try
                    {
                        boost::posix_time::ptime tmp_ptime = boost::posix_time::time_from_string(s);
                        if (tmp_ptime.is_special()) throw std::runtime_error("invalid ptime");

                    }
                    catch(const std::exception& ex)
                    {
#if ( BOOST_VERSION > 104100 )
                    boost::throw_exception(invalid_option_value(s));
#else
                        throw validation_error(
                                std::string("invalid argument value cast"));
#endif
                    }
                }

                v = any(s);
            }
        }//validate checked ptime


    }//namespace program_options
} // namespace boost
#endif
