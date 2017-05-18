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

#ifndef GET_REQUEST_FEE_MESSAGE_H_0FA209472EEE4710B9300DF399837115
#define GET_REQUEST_FEE_MESSAGE_H_0FA209472EEE4710B9300DF399837115

#include "util/decimal/decimal.h"
#include "src/fredlib/opcontext.h"

#include <boost/date_time/posix_time/ptime.hpp>

namespace Fred {
namespace Poll {

struct RequestFeeInfoEvent
{
    boost::posix_time::ptime from;
    boost::posix_time::ptime to;
    unsigned long long free_count;
    unsigned long long used_count;
    Decimal price;
};

RequestFeeInfoEvent get_request_fee_info_message(
    Fred::OperationContext& _ctx,
    unsigned long long _registrar_id,
    const boost::posix_time::ptime& _period_to,
    const std::string& _time_zone = "Europe/Prague");

RequestFeeInfoEvent get_last_request_fee_info_message(
    Fred::OperationContext& _ctx,
    unsigned long long _registrar_id);

} // namespace Fred::Poll
} // namespace Fred

#endif
