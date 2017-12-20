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

#ifndef CREATE_REQUEST_FEE_INFO_MESSAGES_H_D5F1A331247F45C694D57DDA33B7EA23
#define CREATE_REQUEST_FEE_INFO_MESSAGES_H_D5F1A331247F45C694D57DDA33B7EA23

#include "src/libfred/opcontext.hh"
#include "src/libfred/logger_client.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>

namespace LibFred {
namespace Poll {

void create_request_fee_info_messages(
        LibFred::OperationContext& ctx,
        Logger::LoggerClient& logger_client,
        unsigned long long zone_id,
        const boost::optional<boost::gregorian::date>& period_to,
        const std::string& time_zone);

} // namespace LibFred::Poll
} // namespace LibFred

#endif
