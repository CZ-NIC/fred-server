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

#ifndef REGISTRAR_ZONE_ACCESS_H_10675EA02C1D4994B3D25C4098D07F3D
#define REGISTRAR_ZONE_ACCESS_H_10675EA02C1D4994B3D25C4098D07F3D

#include <boost/date_time/gregorian/gregorian.hpp>
#include "src/fredlib/opcontext.h"

namespace Fred {

/**
 * check if the registrar has an access to the zone granted
 */
bool is_zone_accessible_by_registrar(
        unsigned long long _registrar_id,
        unsigned long long _zone_id,
        boost::gregorian::date _local_today,
        OperationContext& _ctx);

} // namespace Fred

#endif
