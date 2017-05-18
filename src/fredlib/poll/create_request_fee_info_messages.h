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

#include "src/fredlib/opcontext.h"
#include "src/fredlib/logger_client.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {
namespace Poll {

void create_request_fee_info_messages(
    Fred::OperationContext& _ctx,
    Logger::LoggerClient& _logger_client,
    unsigned long long _zone_id,
    boost::gregorian::date _period_to,
    const std::string& _time_zone = "Europe/Prague");

} // namespace Fred::Poll
} // namespace Fred

#endif
