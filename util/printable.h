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
#include <utility>
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

    /**
     * Print state of operation into the string.
     * @param operation_name is name of the operation
     * @param key_value_list names and values of operation data members
     * @return description of operation state using implemented format
     */
    std::string format_operation_state(const std::string& operation_name,
        const std::vector<std::pair<std::string, std::string> >& key_value_list);

    /**
     * Print data of data-structure into the string.
     * @param data_structure_name is name of the data-structure
     * @param key_value_list names and values of the structure data members
     * @return description of the data-structure state using implemented format
     */
    std::string format_data_structure(const std::string& data_structure_name,
        const std::vector<std::pair<std::string, std::string> >& key_value_list);

}//namespace Util

#endif //PRINTABLE_H_
