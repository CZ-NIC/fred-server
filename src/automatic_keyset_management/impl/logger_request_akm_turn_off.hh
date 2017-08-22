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

#ifndef LOGGER_REQUEST_AKM_TURN_OFF_HH_F202EBBD883D4D4B9A3AE01D0334D496
#define LOGGER_REQUEST_AKM_TURN_OFF_HH_F202EBBD883D4D4B9A3AE01D0334D496

#include "src/epp/keyset/dns_key.h"
#include "src/fredlib/logger_client.h"
#include "src/fredlib/object/object_id_handle_pair.h"

namespace Fred {
namespace AutomaticKeysetManagement {

class LoggerRequestAkmTurnOff
{
public:
    LoggerRequestAkmTurnOff(
            Fred::Logger::LoggerClient& _logger_client,
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

        logger_request_data.add<LoggerRequestObjectType::domain>(_domain.id);
        logger_request_data.add<LoggerRequestObjectType::keyset>(_keyset.id);

        logger_request_data.add<LoggerRequestProperty::name>(_domain.handle);
        logger_request_data.add<LoggerRequestProperty::keyset>(_keyset.handle);

        logger_request_.close_on_success(logger_request_data);

    }

    void close_on_failure() const
    {
        LoggerRequestData logger_request_data;

        logger_request_.close_on_failure(logger_request_data);
    }

    unsigned long long get_request_id() const
    {
        return logger_request_.get_request_id();
    }

private:
    LoggerRequest<LoggerRequestType::akm_turn_off, LoggerServiceType::admin> logger_request_;
};

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
