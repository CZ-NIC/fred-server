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
 *  object id and handle pair
 */

#ifndef OBJECT_ID_HANDLE_PAIR_HH_B98F5620CB614CC4AD93C8CF08D46449
#define OBJECT_ID_HANDLE_PAIR_HH_B98F5620CB614CC4AD93C8CF08D46449

#include <string>
#include "src/util/printable.hh"

namespace LibFred
{

    /**
     * Object identification structure.
     * Id is unique object identifier but not very illustrative
     * , handle is humanly readable but when object gets deleted
     * , the same handle might be reused for different object.
     */
    struct ObjectIdHandlePair: public Util::Printable
    {
        unsigned long long id;/**< database id of the object*/
        std::string handle;/**< handle of the object*/

        /**
        * Constructor of data structure.
        * @param _id sets object database id into @ref id attribute
        * @param _handle sets object handle into @ref handle attribute
        */
        ObjectIdHandlePair(const unsigned long long _id, const std::string& _handle);

        /**
        * Default constructor of data structure.
        */
        ObjectIdHandlePair();

        /**
         * Comparison operator comparing both attributes.
         * Handle is compared converted to uppercase
         * @param rhs data compared with this instance
         */
        bool operator==(const ObjectIdHandlePair& rhs) const;
        /**
         * Comparison operator comparing both attributes.
         * Handle is compared converted to uppercase
         * @param rhs data compared with this instance
         */
        bool operator!=(const ObjectIdHandlePair& rhs) const;

        /**
         * Comparison operator comparing both attributes in string.
         * Handle is compared converted to uppercase
         * @param rhs data compared with this instance
         */
        bool operator<(const ObjectIdHandlePair& rhs) const;

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

} // namespace LibFred

#endif
