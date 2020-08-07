/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
 *  @file
 *  checked config option types
 */

#ifndef CHECKED_TYPES_HH_AF1016A8028542649AF4E0BFF7CEE830
#define CHECKED_TYPES_HH_AF1016A8028542649AF4E0BFF7CEE830

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <vector>

// types checked by custom validator
struct Checked
{

    typedef std::string string;
    typedef unsigned long long ulonglong;
    typedef unsigned long ulong;
    typedef ulonglong id;
    typedef double fpnumber;
    struct Date
    {
        boost::gregorian::date date;
    };
    typedef boost::posix_time::ptime ptime;
    typedef std::vector<std::string> ip_addresses;
    typedef unsigned short ushort;

    class string_fpnumber
    {
    public:
        std::string to_string() const
        {
            return s_;
        }


        explicit string_fpnumber(const std::string _s)
            : s_(_s)
        {
        }


        string_fpnumber()
        {
        }


        string_fpnumber(const string_fpnumber& _s)
            : s_(_s.to_string())
        {
        }


        string_fpnumber& operator=(const string_fpnumber& rhs)
        {
            s_ = rhs.s_;
            return *this;
        }


        string_fpnumber& operator=(const std::string& rhs)
        {
            s_ = rhs;
            return *this;
        }


        bool operator==(const string_fpnumber& rhs) const
        {
            return s_ == rhs.s_;
        }


        bool operator<(const string_fpnumber& rhs) const
        {
            return s_ < rhs.s_;
        }


    private:
        std::string s_;
    };
};


#endif
