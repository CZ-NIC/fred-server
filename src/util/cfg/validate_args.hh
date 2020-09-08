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
/**
 *  @validate_args.h
 *  program options validate overloads for
 *  option argument value custom validation
 */

#ifndef VALIDATE_ARGS_HH_BAA5803FB80F4518ABC0DC0372B55DCF
#define VALIDATE_ARGS_HH_BAA5803FB80F4518ABC0DC0372B55DCF

#include "src/util/cfg/checked_types.hh"

#include <boost/any.hpp>

#include <iostream>
#include <string>
#include <vector>

#ifdef PROGRAM_OPTIONS_VP_2003_05_19
#error "boost/program_options.hpp included"
#endif

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

std::ostream& operator<<(std::ostream&, const Checked::Date&);
std::istream& operator>>(std::istream&, Checked::Date&);

namespace boost {
namespace program_options {

//overrides the 'validate' function for the 'std::string' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::string*, int);

//overrides the 'validate' function for the 'unsigned long long' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::ulonglong*, int);

//overrides the 'validate' function for the 'unsigned long' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::ulong*, int);

//overrides the 'validate' function for the 'double' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::fpnumber*, int);

void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::string_fpnumber*, int);

//overrides the 'validate' function for the 'boost::gregorian::date' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::date*, int);

//overrides the 'validate' function for the 'boost::posix_time::ptime' type
void validate(boost::any& v,
        const std::vector<std::string>& values,
        Checked::ptime*, int);

}//namespace boost::program_options
}//namespace boost::

#endif
