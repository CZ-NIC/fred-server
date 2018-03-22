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

/**
 *  @file
 *  registrar certification type
 */

#ifndef REGISTRAR_CERTIFICATION_TYPE_HH_D0F08A06E53E4022B48B1AA559AE096A
#define REGISTRAR_CERTIFICATION_TYPE_HH_D0F08A06E53E4022B48B1AA559AE096A

#include <boost/date_time/gregorian/gregorian.hpp>

namespace LibFred {
namespace Registrar {

struct RegistrarCertification
{
    unsigned long long id;
    boost::gregorian::date valid_from;
    boost::gregorian::date valid_until;
    int classification;
    unsigned long long eval_file_id;

    bool operator==(const RegistrarCertification& _other) const
    {
        return (id == _other.id &&
            valid_from == _other.valid_from &&
            valid_until == _other.valid_until &&
            classification == _other.classification &&
            eval_file_id == _other.eval_file_id);
    }
};

} // namespace Registrar
} // namespace LibFred
#endif
