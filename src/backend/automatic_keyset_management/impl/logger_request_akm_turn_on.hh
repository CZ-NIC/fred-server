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
#ifndef LOGGER_REQUEST_AKM_TURN_ON_HH_63B5F847808546D4A2FC3E8963CC16DF
#define LOGGER_REQUEST_AKM_TURN_ON_HH_63B5F847808546D4A2FC3E8963CC16DF

#include "src/backend/automatic_keyset_management/dns_key.hh"
#include "src/backend/automatic_keyset_management/keyset.hh"
#include "src/backend/automatic_keyset_management/impl/domain_reference.hh"
#include "src/backend/automatic_keyset_management/impl/keyset_reference.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_data.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_object_type.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_property.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_type.hh"
#include "src/backend/epp/keyset/dns_key.hh"
#include "src/deprecated/libfred/logger_client.hh"
#include "libfred/object/object_id_handle_pair.hh"

namespace Fred {
namespace Backend {
namespace AutomaticKeysetManagement {
namespace Impl {

class LoggerRequestAkmTurnOn
{
public:
    LoggerRequestAkmTurnOn(
            LibFred::Logger::LoggerClient& _logger_client,
            const DomainReference& _domain)
        : logger_request_(
                  _logger_client,
                  LoggerRequestData()
                          .add<LoggerRequestObjectType::domain>(_domain.id)
                          .add<LoggerRequestProperty::name>(_domain.fqdn))
    {
    }

    void close_on_success(
            const DomainReference& _domain,
            const KeysetReference& _keyset,
            const Keyset& _new_keyset) const
    {
        LoggerRequestData logger_request_data;

        logger_request_data.add<LoggerRequestObjectType::keyset>(_keyset.id);

        logger_request_data.add<LoggerRequestProperty::name>(_domain.fqdn);
        logger_request_data.add<LoggerRequestProperty::keyset>(_keyset.handle);
        for (const auto& new_dns_key : _new_keyset.dns_keys)
        {
            logger_request_data.add<LoggerRequestProperty::new_dns_key>(to_string(new_dns_key));
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
    LoggerRequest<LoggerRequestType::akm_turn_on, LoggerServiceType::admin> logger_request_;
};

} // namespace Fred::Backend::AutomaticKeysetManagement::Impl
} // namespace Fred::Backend::AutomaticKeysetManagement
} // namespace Fred::Backend
} // namespace Fred

#endif
