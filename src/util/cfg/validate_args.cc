/*
 * Copyright (C) 2011-2020  CZ.NIC, z. s. p. o.
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

#include "src/util/cfg/validate_args.hh"
#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/serialization/strong_typedef.hpp>

#include <memory>
#include <stdexcept>

namespace boost {
namespace program_options {

//All this 'validate' functions are just ugly hack!!!
//Checked::string (and other Checked::types) is alias for std::string, so
//this validator overrides the one distributed in binary boost library!
//Static linking of boost library is impossible due the multiple function
//definitions.

//overrides the 'validate' function for the 'std::string' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::string*, int)
{
    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const Checked::string& s = validators::get_single_string(values);

    if ((2 <= s.size()) //string surrounded by quotation marks
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

//overrides the 'validate' function for the 'unsigned long long' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::ulonglong*, int)
{
    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);

    if ((2 <= s.size()) //string surrounded by quotation marks
            && // and
            (
                    (*s.begin() == '\'' && *s.rbegin() == '\'') //quoted by '\'
                    ||                                          //or
                    (*s.begin() == '"' && *s.rbegin() == '"')   //quoted by '"'
                    )
            )
    {
        std::string str_inside_quotes = s.substr(1, s.size()-2);

        std::size_t found=str_inside_quotes.find_first_not_of("0123456789");
        if (found!=std::string::npos)
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
        catch(const std::exception&)
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
            std::size_t found=s.find_first_not_of("0123456789");
            if (found!=std::string::npos)
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
            catch(const std::exception&)
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

//overrides the 'validate' function for the 'unsigned long' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::ulong*, int)
{
    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);

    if ((2 <= s.size()) //string surrounded by quotation marks
            && // and
            (
                    (*s.begin() == '\'' && *s.rbegin() == '\'') //quoted by '\'
                    ||                                          //or
                    (*s.begin() == '"' && *s.rbegin() == '"')   //quoted by '"'
                    )
            )
    {
        std::string str_inside_quotes = s.substr(1, s.size()-2);

        std::size_t found=str_inside_quotes.find_first_not_of("0123456789");
        if (found!=std::string::npos)
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
        catch(const std::exception&)
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
            std::size_t found=s.find_first_not_of("0123456789");
            if (found!=std::string::npos)
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
            catch(const std::exception&)
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


//overrides the 'validate' function for the 'double' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::fpnumber*, int)
{
    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);


    //fp number check
    static const boost::regex fp_number_regex("[-+]?([0-9]*\\.[0-9]+|[0-9]+)");

    if ((2 <= s.size()) //string surrounded by quotation marks
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
        catch(const std::exception&)
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
            catch(const std::exception&)
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
    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);


    //fp number check
    static const boost::regex fp_number_regex("[-+]?([0-9]*\\.[0-9]+|[0-9]+)");

    if ((2 <= s.size()) //string surrounded by quotation marks
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
        catch(const std::exception&)
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
            catch(const std::exception&)
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

//overrides the 'validate' function for the 'boost::gregorian::date' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::date*, int)
{
    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);


    //date check YYYY-MM-DD or YYYY/MM/DD
    static const boost::regex date_regex("((19|20)\\d\\d)([/-])(0[1-9]|1[012])([/-])(0[1-9]|[12][0-9]|3[01])");

    if ((2 <= s.size()) //string surrounded by quotation marks
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
        catch(const std::exception&)
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
            catch(const std::exception&)
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

//overrides the 'validate' function for the 'boost::posix_time::ptime' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::ptime*, int)
{
    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);


    //ptime check YYYY-MM-DD hh:mm:ss or YYYY/MM/DD hh:mm:ss
    static const boost::regex ptime_regex(
            "((19|20)\\d\\d)([/-])(0[1-9]|1[012])([/-])(0[1-9]|[12][0-9]|3[01]) (0[0-9]|1[0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])");

    if ((2 <= s.size()) //string surrounded by quotation marks
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
        catch(const std::exception&)
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
            catch(const std::exception&)
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

}//namespace boost::program_options
}//namespace boost

std::ostream& operator<<(std::ostream& o, const Checked::string_fpnumber& s)
{
    return o << (s.to_string());
}

std::istream& operator>>(std::istream& i, Checked::string_fpnumber& s)
{
    std::string tmp;
    i >> tmp;
    s = tmp;
    return i;
}

std::ostream& operator<<(std::ostream& o, const Checked::Date& s)
{
    return o << boost::gregorian::to_iso_extended_string(s.date);
}

std::istream& operator>>(std::istream& i, Checked::Date& s)
{
    std::string date;
    i >> date;
    s.date = boost::gregorian::from_string(date);
    return i;
}
