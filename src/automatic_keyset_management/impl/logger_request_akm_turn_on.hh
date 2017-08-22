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

#ifndef LOGGER_REQUEST_AKM_TURN_ON_HH_63B5F847808546D4A2FC3E8963CC16DF
#define LOGGER_REQUEST_AKM_TURN_ON_HH_63B5F847808546D4A2FC3E8963CC16DF

#include "src/automatic_keyset_management/dns_key.hh"
#include "src/automatic_keyset_management/impl/logger_request_data.hh"
#include "src/automatic_keyset_management/impl/logger_request_object_type.hh"
#include "src/automatic_keyset_management/impl/logger_request_property.hh"
#include "src/automatic_keyset_management/impl/logger_request_type.hh"
#include "src/epp/keyset/dns_key.h"
#include "src/fredlib/logger_client.h"
#include "src/fredlib/object/object_id_handle_pair.h"

namespace Fred {
namespace AutomaticKeysetManagement {

class LoggerRequestAkmTurnOn
{
public:
    LoggerRequestAkmTurnOn(
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
            const ObjectIdHandlePair& _keyset,
            const Keyset& _new_keyset) const
    {
        LoggerRequestData logger_request_data;

        logger_request_data.add<LoggerRequestObjectType::domain>(_domain.id);
        logger_request_data.add<LoggerRequestObjectType::keyset>(_keyset.id);

        logger_request_data.add<LoggerRequestProperty::name>(_domain.handle);
        logger_request_data.add<LoggerRequestProperty::keyset>(_keyset.handle);
        for (DnsKeys::const_iterator new_dns_key = _new_keyset.dns_keys.begin();
                new_dns_key != _new_keyset.dns_keys.end();
                ++new_dns_key)
        {
            logger_request_data.add<LoggerRequestProperty::new_dns_key>(to_string(*new_dns_key));
        }

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
    LoggerRequest<LoggerRequestType::akm_turn_on, LoggerServiceType::admin> logger_request_;
};

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
