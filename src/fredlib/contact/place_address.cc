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

#include "place_address.h"
#include "util/util.h"

namespace Fred {
namespace Contact {

std::string PlaceAddress::to_string()const
{
    return Util::format_data_structure("PlaceAddress",
    Util::vector_of< std::pair< std::string, std::string > >
    (std::make_pair("street1", street1))
    (std::make_pair("street2", street2.print_quoted()))
    (std::make_pair("street3", street3.print_quoted()))
    (std::make_pair("city", city))
    (std::make_pair("stateorprovince", stateorprovince.print_quoted()))
    (std::make_pair("postalcode", postalcode))
    (std::make_pair("country", country))
    );
}

bool PlaceAddress::operator==(const PlaceAddress &_b)const
{
    return street1 == _b.street1 &&
           street2 == _b.street2 &&
           street3 == _b.street3 &&
           city == _b.city &&
           stateorprovince == _b.stateorprovince &&
           postalcode == _b.postalcode &&
           country == _b.country;
}

PlaceAddress::PlaceAddress()
{}

PlaceAddress::PlaceAddress(const std::string& _street1,
        const Optional< std::string >& _street2,
        const Optional< std::string >& _street3,
        const std::string& _city,
        const Optional< std::string >& _stateorprovince,
        const std::string& _postalcode,
        const std::string& _country)
    : street1(_street1)
    , street2(_street2)
    , street3(_street3)
    , city(_city)
    , stateorprovince(_stateorprovince)
    , postalcode(_postalcode)
    , country(_country)
    {}

std::ostream& operator<<(std::ostream &_out, const PlaceAddress &_place)
{
    return _out << _place.to_string();
}

}//namespace Contact
}//namespace Fred
