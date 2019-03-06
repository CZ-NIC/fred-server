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
#ifndef LOGGER_REQUEST_HH_642A390DFFAC4875AB0A57716CA9593F
#define LOGGER_REQUEST_HH_642A390DFFAC4875AB0A57716CA9593F

#include "src/backend/automatic_keyset_management/impl/logger_request_data.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_result.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_type.hh"
#include "src/backend/automatic_keyset_management/impl/logger_service_type.hh"
#include "src/deprecated/libfred/logger_client.hh"

#include <string>

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

/* wrapper around logger_client.createRequest / logger_client.closeRequest methods */
template <LoggerRequestType::Enum R, LoggerServiceType::Enum S>
class LoggerRequest
{
public:
    LoggerRequest(
            LibFred::Logger::LoggerClient& _logger_client,
            const LoggerRequestData& _logger_request_data)
        : logger_client_(_logger_client),
          request_id_(
                  create_request(
                          _logger_client,
                          _logger_request_data.get_request_properties(),
                          _logger_request_data.get_object_references()))
    {
        if (request_id_ == 0)
        {
            LOGGER.error("unable to log " + to_fred_logger_request_type_name<R>() + " request");
        }
    }

    void close_on_success(const LoggerRequestData& _logger_request_data) const
    {
        const std::string no_content = "";
        const unsigned long long no_session_id = 0;
        LoggerRequestData logger_request_data = _logger_request_data;
        logger_request_data.add<LoggerRequestProperty::op_tr_id>(request_id_);
        logger_client_.closeRequest(
                request_id_,
                to_fred_logger_service_type_name<S>(),
                no_content,
                logger_request_data.get_request_properties(),
                logger_request_data.get_object_references(),
                to_fred_logger_request_result_name<LoggerRequestResult::success>(),
                no_session_id);
    }

    void close_on_failure() const
    {
        LoggerRequestData logger_request_data;
        logger_request_data.add<LoggerRequestProperty::op_tr_id>(request_id_);
        const std::string no_content = "";
        const unsigned long long no_session_id = 0;
        logger_client_.closeRequest(
                request_id_,
                to_fred_logger_service_type_name<S>(),
                no_content,
                logger_request_data.get_request_properties(),
                logger_request_data.get_object_references(),
                to_fred_logger_request_result_name<LoggerRequestResult::fail>(),
                no_session_id);
    }

    unsigned long long get_request_id() const
    {
        return request_id_;
    }

private:
    static unsigned long long create_request(
            LibFred::Logger::LoggerClient& _logger_client,
            const Logger::RequestProperties& _properties,
            const Logger::ObjectReferences& _references)
    {
        const std::string no_src_ip = "";
        const std::string no_content = "";
        const unsigned long long no_session_id = 0;
        return _logger_client.createRequest(
                no_src_ip,
                to_fred_logger_service_type_name<S>(),
                no_content,
                _properties,
                _references,
                to_fred_logger_request_type_name<R>(),
                no_session_id);
    }

    LibFred::Logger::LoggerClient& logger_client_;
    const unsigned long long request_id_;
};

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
