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

#ifndef LOGGER_REQUEST_AKM_ROLLOVER_HH_096FAB3789414D39BAF2DC4DE172E597
#define LOGGER_REQUEST_AKM_ROLLOVER_HH_096FAB3789414D39BAF2DC4DE172E597

#include "src/backend/automatic_keyset_management/dns_key.hh"
#include "src/backend/automatic_keyset_management/keyset.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_data.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_property.hh"
#include "src/backend/automatic_keyset_management/impl/util.hh"
#include "src/backend/epp/keyset/dns_key.hh"
#include "libfred/registrable_object/keyset/keyset_dns_key.hh"
#include "src/deprecated/libfred/logger_client.hh"
#include "libfred/object/object_id_handle_pair.hh"

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

class LoggerRequestAkmRollover
{
public:
    LoggerRequestAkmRollover(
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
            const ObjectIdHandlePair& _keyset,
            const std::vector<LibFred::DnsKey>& _old_keyset,
            const Keyset& _new_keyset) const
    {
        LoggerRequestData logger_request_data;

        logger_request_data.add<LoggerRequestObjectType::keyset>(_keyset.id);

        logger_request_data.add<LoggerRequestProperty::name>(_domain.handle);
        logger_request_data.add<LoggerRequestProperty::keyset>(_keyset.handle);
        for (std::vector<LibFred::DnsKey>::const_iterator old_dns_key = _old_keyset.begin();
                old_dns_key != _old_keyset.end();
                ++old_dns_key)
        {
            logger_request_data.add<LoggerRequestProperty::old_dns_key>(to_string(*old_dns_key));
        }
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
        logger_request_.close_on_failure();
    }

    unsigned long long get_request_id() const
    {
        return logger_request_.get_request_id();
    }

private:
    LoggerRequest<LoggerRequestType::akm_rollover, LoggerServiceType::admin> logger_request_;
};

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
