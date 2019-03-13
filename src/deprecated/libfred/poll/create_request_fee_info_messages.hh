/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CREATE_REQUEST_FEE_INFO_MESSAGES_HH_C92C3C124088452E88DD904BEA99F544
#define CREATE_REQUEST_FEE_INFO_MESSAGES_HH_C92C3C124088452E88DD904BEA99F544

#include "libfred/opcontext.hh"
#include "src/deprecated/libfred/logger_client.hh"

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
