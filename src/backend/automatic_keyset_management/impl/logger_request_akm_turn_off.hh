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
#ifndef LOGGER_REQUEST_AKM_TURN_OFF_HH_F202EBBD883D4D4B9A3AE01D0334D496
#define LOGGER_REQUEST_AKM_TURN_OFF_HH_F202EBBD883D4D4B9A3AE01D0334D496

#include "src/backend/epp/keyset/dns_key.hh"
#include "src/deprecated/libfred/logger_client.hh"
#include "libfred/object/object_id_handle_pair.hh"

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

class LoggerRequestAkmTurnOff
{
public:
    LoggerRequestAkmTurnOff(
            LibFred::Logger::LoggerClient& _logger_client,
            const ObjectIdHandlePair& _domain)
        : logger_request_(
                  _logger_client,
                  LoggerRequestData()
                          .add<LoggerRequestObjectType::domain>(_domain.id)
                          .add<LoggerRequestProperty::name>(_domain.handle))
    {
    }

    void close_on_success(
            const ObjectIdHandlePair& _domain,
            const ObjectIdHandlePair& _keyset) const
    {
        LoggerRequestData logger_request_data;

        logger_request_data.add<LoggerRequestObjectType::keyset>(_keyset.id);

        logger_request_data.add<LoggerRequestProperty::name>(_domain.handle);
        logger_request_data.add<LoggerRequestProperty::keyset>(_keyset.handle);

        logger_request_.close_on_success(logger_request_data);

    }

    void close_on_failure() const
    {
        logger_request_.close_on_failure();
    }

    unsigned long long get_request_id() const
    {
        return logger_request_.get_request_id();
    }

private:
    LoggerRequest<LoggerRequestType::akm_turn_off, LoggerServiceType::admin> logger_request_;
};

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
