/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DOMAIN_REGISTRATION_TIME_HH_63D73F88A6A5499F9976B4AA070722A1
#define DOMAIN_REGISTRATION_TIME_HH_63D73F88A6A5499F9976B4AA070722A1

#include <stdexcept>

namespace Epp {
namespace Domain {

class DomainRegistrationTime
{
public:
    struct Unit
    {
        enum Enum
        {
            month,
            year

        };

    };

private:
    int length_of_domain_registration;
    Unit::Enum unit_of_domain_registration_time;

public:
    DomainRegistrationTime(
            int _length_of_domain_registration,
            Unit::Enum _unit_of_domain_registration_time)
        : length_of_domain_registration(_length_of_domain_registration),
          unit_of_domain_registration_time(_unit_of_domain_registration_time)
    {
    }


    int get_length_of_domain_registration_in_months() const
    {
        const int months_in_year = 12;
        switch (unit_of_domain_registration_time)
        {
            case Unit::month:
                return length_of_domain_registration;

                break;

            case Unit::year:
                return length_of_domain_registration * months_in_year;

                break;
        }
        throw std::runtime_error("internal error"); // unreachable
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
