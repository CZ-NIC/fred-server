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
 *  domain ENUM validation extension
 */

#ifndef ENUM_VALIDATION_EXTENSION_HH_284897C7D2FA49E7803BCDB2B0E906E1
#define ENUM_VALIDATION_EXTENSION_HH_284897C7D2FA49E7803BCDB2B0E906E1

#include <boost/date_time/gregorian/gregorian.hpp>


namespace LibFred
{

    /**
     * ENUM domain validation extension
     */
    struct ENUMValidationExtension
    {
        boost::gregorian::date validation_expiration;/**< the expiration date of the ENUM domain validation */
        bool publish;/**< flag for publishing ENUM number and associated contact in public directory */
        /**
         * Default constructor initializing @ref validation_expiration to not-a-date-time and @ref publish to false
         */
        ENUMValidationExtension()
        : validation_expiration()//not a date time
        , publish(false)
        {}
        /**
         * Constructor for costum initialization of attributes
         * @param _validation_expiration sets the expiration date of the ENUM domain validation into @ref validation_expiration attribute
         * @param _publish sets flag for publishing ENUM number and associated contact in public directory into @ref publish attribute
         */
        ENUMValidationExtension(const boost::gregorian::date& _validation_expiration
                , bool _publish)
        : validation_expiration(_validation_expiration)
        , publish(_publish)
        {}
        /**
         * Comparison operator comparing both attributes
         * @param rhs data compared with this instance
         */
        bool operator==(const ENUMValidationExtension& rhs) const
        {
            return (publish == rhs.publish) && (validation_expiration == rhs.validation_expiration);
        }

        /**
        * Dumps state of the instance into stream
        * @param os contains output stream reference
        * @param i reference of instance to be dumped into the stream
        * @return output stream reference
        */
        friend std::ostream& operator<<(std::ostream& os, const ENUMValidationExtension& i)
        {
            return os << "ENUMValidationExtension publish: " << i.publish
                << " validation_expiration: " << boost::gregorian::to_simple_string(i.validation_expiration);
        }
    };

} // namespace LibFred

#endif
