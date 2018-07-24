/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef REGISTRAR_HH_9C42489EE07E4E49B87AF2941D36710D
#define REGISTRAR_HH_9C42489EE07E4E49B87AF2941D36710D

namespace Fred {
namespace Backend {
namespace Accounting {

#include <string>

struct PlaceAddress
{
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string stateorprovince;
    std::string postalcode;
    std::string country_code;
};

struct Registrar
{
    std::string handle;
    std::string name;
    std::string organization;
    std::string cin;
    std::string tin;
    std::string url;
    std::string phone;
    std::string fax;
    PlaceAddress address;
};

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
