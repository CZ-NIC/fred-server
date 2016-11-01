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
 *  registrar zone access
 */

#ifndef REGISTRAR_ZONE_ACCESS_H_2753b3b14fbc4ef09d5b28192fb7e0bf
#define REGISTRAR_ZONE_ACCESS_H_2753b3b14fbc4ef09d5b28192fb7e0bf


#include <boost/date_time/gregorian/gregorian.hpp>
#include "src/fredlib/opcontext.h"

namespace Fred
{
    /**
     * check if registrar have allowed access to the zone
     */
    bool registrar_zone_access(unsigned long long registrar_id,
            unsigned long long zone_id,
            boost::gregorian::date _local_today, OperationContext& ctx);
}//namespace Fred

#endif
