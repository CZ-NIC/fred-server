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

#ifndef GET_REQUEST_FEE_MESSAGE_HH_EAFBD242B0F6426592586876A9513248
#define GET_REQUEST_FEE_MESSAGE_HH_EAFBD242B0F6426592586876A9513248

#include "src/util/decimal/decimal.hh"
#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/ptime.hpp>

namespace LibFred {
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
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id,
        const boost::posix_time::ptime& period_to,
        const std::string& time_zone);

RequestFeeInfoEvent get_last_request_fee_info_message(
        LibFred::OperationContext& ctx,
        unsigned long long registrar_id);

} // namespace LibFred::Poll
} // namespace LibFred

#endif
