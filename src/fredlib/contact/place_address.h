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
 *  place address type
 */

#ifndef PLACE_ADDRESS_H_40BCB51572DBDD30F6E1441AE2CCADF5 // PLACE_ADDRESS_H_$(date "+%s"|md5sum)
#define PLACE_ADDRESS_H_40BCB51572DBDD30F6E1441AE2CCADF5

#include "util/optional_value.h"
#include <iosfwd>
#include <string>

namespace Fred {
namespace Contact {

typedef struct _PlaceAddress
{
    std::string street1;
    Optional< std::string > street2;
    Optional< std::string > street3;
    std::string city;
    Optional< std::string > stateorprovince;
    std::string postalcode;
    std::string country;
    std::string to_string()const;
    bool operator==(const struct _PlaceAddress &_b)const;
} PlaceAddress;

}//namespace Contact
}//namespace Fred

std::ostream& operator<<(std::ostream&, const Fred::Contact::PlaceAddress&);

#endif//PLACE_ADDRESS_H_40BCB51572DBDD30F6E1441AE2CCADF5
