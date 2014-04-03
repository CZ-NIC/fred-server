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
    Util::vector_of< std::pair< std::string, std::string > > data(std::make_pair("street1", street1));
    if (street2.isset()) {
        data(std::make_pair("street2", street2.get_value()));
    }
    if (street3.isset()) {
        data(std::make_pair("street3", street3.get_value()));
    }
    data(std::make_pair("city", city));
    if (stateorprovince.isset()) {
        data(std::make_pair("stateorprovince", stateorprovince.get_value()));
    }
    data(std::make_pair("postalcode", postalcode));
    data(std::make_pair("country", country));
    return Util::format_data_structure("PlaceAddress", data);
}

namespace
{

template < typename T > bool operator==(const Optional< T > &_a, const Optional< T > &_b)
{
    return (_a.isset() && _b.isset() && (_a.get_value() == _b.get_value())) ||
           (!_a.isset() && !_b.isset());
}

}

bool PlaceAddress::operator==(const struct _PlaceAddress &_b)const
{
    return street1 == _b.street1 &&
           street2 == _b.street2 &&
           street3 == _b.street3 &&
           city == _b.city &&
           stateorprovince == _b.stateorprovince &&
           postalcode == _b.postalcode &&
           country == _b.country;
}

}//namespace Contact
}//namespace Fred

std::ostream& operator<<(std::ostream &_out, const Fred::Contact::PlaceAddress &_place)
{
    return _out << _place.to_string();
}
