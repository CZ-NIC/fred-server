/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @optional_value.h
 *  optional value
 */

#ifndef OPTIONAL_VALUE_H_
#define OPTIONAL_VALUE_H_

#include <utility>
#include <string>
#include <sstream>
#include <stdexcept>

#include "util/printable.h"

namespace arbitrary_pair_ostream_support
{
    template<class T1, class T2> std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& input_pair)
    {
        return os << "first: " << input_pair.first << " second: " << input_pair.second;
    }

    template<class T1, class T2> std::ostream& operator<<(std::ostream& os, const std::pair<std::vector<T1>, std::vector<T2> >& input_pair)
    {
        return os << "first: " << Util::format_vector(input_pair.first) << " second: " << Util::format_vector(input_pair.second);
    }
}

//simple optional value template
template<typename T>
class Optional
{
private:
    bool isset_;
    T value_;

public:
    Optional()
        : isset_(false),
          value_()
    {}

    Optional(const T &_value)
        : isset_(true),
          value_(_value)
    {}//init

    Optional(Optional const & rhs)
    : isset_(rhs.isset_)
    , value_(rhs.value_)
    {}//copy

    Optional& operator=(Optional const & rhs)
    {
        if (this != &rhs)
        {
            value_=rhs.value_;
            isset_ = rhs.isset_;
        }
        return *this;
    }//assignment

    bool isset() const
    {
        return isset_;
    }

    T get_value() const
    {
        if (!isset())
        {
            throw std::logic_error("value is not set");
        }

        return value_;
    }

    T get_value_or_default() const
    {
        return value_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Optional<T>& ov)
    {
        using namespace arbitrary_pair_ostream_support;
        return os << ov.get_value();
    }

    std::string print_quoted() const
    {
        std::stringstream ss;
        ss << (*this);
        return isset() ? std::string("'") + ss.str() + "'" : std::string("[N/A]");
    }

};

#endif //OPTIONAL_VALUE_H_

