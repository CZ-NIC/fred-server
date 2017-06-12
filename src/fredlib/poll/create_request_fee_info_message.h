/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_REQUEST_FEE_INFO_MESSAGE_H_77A356ACEC01445E95360A5D704119D6
#define CREATE_REQUEST_FEE_INFO_MESSAGE_H_77A356ACEC01445E95360A5D704119D6

#include "src/fredlib/opcontext.h"
#include "util/decimal/decimal.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {
namespace Poll {

class CreateRequestFeeInfoMessage
{
    unsigned long long registrar_id;
    boost::posix_time::ptime period_from;
    boost::posix_time::ptime period_to;
    unsigned long long total_free_count;
    unsigned long long request_count;
    Decimal price;

public:
    CreateRequestFeeInfoMessage(
        unsigned long long _registrar_id,
        const boost::posix_time::ptime& _period_from,
        const boost::posix_time::ptime& _period_to,
        unsigned long long _total_free_count,
        unsigned long long _request_count,
        const Decimal& _price)
    :
        registrar_id(_registrar_id),
        period_from(_period_from),
        period_to(_period_to),
        total_free_count(_total_free_count),
        request_count(_request_count),
        price(_price)
    {}

    unsigned long long exec(OperationContext& _ctx, const std::string& _time_zone = "Europe/Prague") const;
};

} // namespace Fred::Poll
} // namespace Fred

#endif
