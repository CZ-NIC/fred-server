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

#ifndef UPDATE_REGISTRAR_ZONE_ACCESS_HH_869E1F1CF81E4E7AA6D1AE29840B5E08
#define UPDATE_REGISTRAR_ZONE_ACCESS_HH_869E1F1CF81E4E7AA6D1AE29840B5E08

#include "src/libfred/opcontext.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/optional.hpp>

namespace LibFred {
namespace Registrar {
namespace ZoneAccess {

class UpdateRegistrarZoneAccess{

public:
    explicit UpdateRegistrarZoneAccess(unsigned long long _id);

    UpdateRegistrarZoneAccess& set_from_date(const boost::optional<boost::gregorian::date>& _from_date);

    UpdateRegistrarZoneAccess& set_to_date(const boost::optional<boost::gregorian::date>& _to_date);

    unsigned long long exec(OperationContext& _ctx) const;

private:
    unsigned long long id_;
    boost::optional<boost::gregorian::date> from_date_;
    boost::optional<boost::gregorian::date> to_date_;
};

} // namespace LibFred::Registrar::ZoneAccess
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
