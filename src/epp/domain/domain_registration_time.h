/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#ifndef DOMAIN_REGISTRATION_TIME_H_521BF5BEF4D94811AAAE0FEB4C801D0C
#define DOMAIN_REGISTRATION_TIME_H_521BF5BEF4D94811AAAE0FEB4C801D0C

#include <stdexcept>

namespace Epp {

    /**
     *
     */
    class DomainRegistrationTime
    {
    public:
        struct Unit
        {
            enum Enum {month,year};
        };
    private:
        int length_of_domain_registration;
        Unit::Enum unit_of_domain_registration_time;
    public:
        DomainRegistrationTime(int _length_of_domain_registration,
            Unit::Enum _unit_of_domain_registration_time)
        : length_of_domain_registration(_length_of_domain_registration)
        , unit_of_domain_registration_time(_unit_of_domain_registration_time)
        {
            if (length_of_domain_registration < 0) throw std::out_of_range("negative time of domain registration");
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
            };
            throw std::runtime_error("internal error");//unreachable
        }
    };
};

#endif
