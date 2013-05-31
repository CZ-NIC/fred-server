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
 *  @file enum_validation_extension.h
 *  domain ENUM validation extension
 */

#ifndef ENUM_VALIDATION_EXTENSION_H_
#define ENUM_VALIDATION_EXTENSION_H_

#include <boost/date_time/gregorian/gregorian.hpp>


namespace Fred
{

    ///enum domain validation extension
    struct ENUMValidationExtension
    {
        boost::gregorian::date validation_expiration;//expiration date of validation
        bool publish;//publish in ENUM dictionary
        ENUMValidationExtension()
        : validation_expiration()//not a date time
        , publish(false)
        {}
        ENUMValidationExtension(const boost::gregorian::date& _validation_expiration
                , bool _publish)
        : validation_expiration(_validation_expiration)
        , publish(_publish)
        {}

        bool operator==(const ENUMValidationExtension& rhs) const
        {
            return (publish == rhs.publish) && (validation_expiration == rhs.validation_expiration);
        }
        friend std::ostream& operator<<(std::ostream& os, const ENUMValidationExtension& i)
        {
            return os << "ENUMValidationExtension publish: " << i.publish
                << " validation_expiration: " << boost::gregorian::to_simple_string(i.validation_expiration);
        }
    };

}//namespace Fred

#endif//ENUM_VALIDATION_EXTENSION_H_
