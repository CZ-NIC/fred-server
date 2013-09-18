/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  print description of object data to the string and ostream for debugging and error handling
 */


#ifndef PRINTABLE_H_
#define PRINTABLE_H_

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <iterator>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

namespace Util
{

    /**
     * Base class that adds ostream& @ref operator<<.
     */
    class Printable
    {
    public:
        /**
         * Empty destructor.
         */
        virtual ~Printable(){};
        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        virtual std::string to_string() const = 0;
        /**
        * Dumps state of the instance into stream
        * @param os contains output stream reference
        * @param i reference of instance to be dumped into the stream
        * @return output stream reference
        */
        friend std::ostream& operator<<(std::ostream& os, const Printable& i)
        {
            return os << i.to_string();
        }
    };

}//namespace Fred

#endif //PRINTABLE_H_
