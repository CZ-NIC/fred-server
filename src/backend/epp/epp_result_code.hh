/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef EPP_RESULT_CODE_HH_BA46C8C650584E8DA43361AD39D5D131
#define EPP_RESULT_CODE_HH_BA46C8C650584E8DA43361AD39D5D131

#include "src/backend/epp/exception.hh"

namespace Epp {

/**
 * EppResultCode represents STD 69 EPP result codes.
 * It is split into Success (1xxx) and Failure (2xxx) codes.
 */
struct EppResultCode
{
    enum Success
    {
        command_completed_successfully                   = 1000, ///< EPP error code 1000
        command_completed_successfully_action_pending    = 1001, ///< EPP error code 1001
        command_completed_successfully_no_messages       = 1300, ///< EPP error code 1300
        command_completed_successfully_ack_to_dequeue    = 1301, ///< EPP error code 1301
        command_completed_successfully_ending_session    = 1500, ///< EPP error code 1500
    };

    enum Failure
    {
        unknown_command                                  = 2000, ///< EPP error code 2000
        command_syntax_error                             = 2001, ///< EPP error code 2001
        command_use_error                                = 2002, ///< EPP error code 2002
        required_parameter_missing                       = 2003, ///< EPP error code 2003
        parameter_value_range_error                      = 2004, ///< EPP error code 2004
        parameter_value_syntax_error                     = 2005, ///< EPP error code 2005
        unimplemented_protocol_version                   = 2100, ///< EPP error code 2100
        unimplemented_command                            = 2101, ///< EPP error code 2101
        unimplemented_option                             = 2102, ///< EPP error code 2102
        unimplemented_extension                          = 2103, ///< EPP error code 2103
        billing_failure                                  = 2104, ///< EPP error code 2104
        object_is_not_eligible_for_renewal               = 2105, ///< EPP error code 2105
        object_is_not_eligible_for_transfer              = 2106, ///< EPP error code 2106
        authentication_error                             = 2200, ///< EPP error code 2200
        authorization_error                              = 2201, ///< EPP error code 2201
        invalid_authorization_information                = 2202, ///< EPP error code 2202
        object_pending_transfer                          = 2300, ///< EPP error code 2300
        object_not_pending_transfer                      = 2301, ///< EPP error code 2301
        object_exists                                    = 2302, ///< EPP error code 2302
        object_does_not_exist                            = 2303, ///< EPP error code 2303
        object_status_prohibits_operation                = 2304, ///< EPP error code 2304
        object_association_prohibits_operation           = 2305, ///< EPP error code 2305
        parameter_value_policy_error                     = 2306, ///< EPP error code 2306
        unimplemented_object_service                     = 2307, ///< EPP error code 2307
        data_management_policy_violation                 = 2308, ///< EPP error code 2308
        command_failed                                   = 2400, ///< EPP error code 2400
        command_failed_server_closing_connection         = 2500, ///< EPP error code 2500
        authentication_error_server_closing_connection   = 2501, ///< EPP error code 2501
        session_limit_exceeded_server_closing_connection = 2502, ///< EPP error code 2502
    };

    static bool is_valid(Success _value)
    {
        switch (_value)
        {
            case command_completed_successfully:
            case command_completed_successfully_action_pending:
            case command_completed_successfully_no_messages:
            case command_completed_successfully_ack_to_dequeue:
            case command_completed_successfully_ending_session:
                return true;
        }
        return false;
    }

    static bool is_valid(Failure _value)
    {
        switch (_value)
        {
            case unknown_command:
            case command_syntax_error:
            case command_use_error:
            case required_parameter_missing:
            case parameter_value_range_error:
            case parameter_value_syntax_error:
            case unimplemented_protocol_version:
            case unimplemented_command:
            case unimplemented_option:
            case unimplemented_extension:
            case billing_failure:
            case object_is_not_eligible_for_renewal:
            case object_is_not_eligible_for_transfer:
            case authentication_error:
            case authorization_error:
            case invalid_authorization_information:
            case object_pending_transfer:
            case object_not_pending_transfer:
            case object_exists:
            case object_does_not_exist:
            case object_status_prohibits_operation:
            case object_association_prohibits_operation:
            case parameter_value_policy_error:
            case unimplemented_object_service:
            case data_management_policy_violation:
            case command_failed:
            case command_failed_server_closing_connection:
            case authentication_error_server_closing_connection:
            case session_limit_exceeded_server_closing_connection:
                return true;
        }
        return false;
    }

    static const char* c_str(EppResultCode::Success _epp_result_code)
    {
        switch (_epp_result_code)
        {
            case command_completed_successfully:                   return "Command completed successfully";
            case command_completed_successfully_action_pending:    return "Command completed successfully; action pending";
            case command_completed_successfully_no_messages:       return "Command completed successfully; no messages";
            case command_completed_successfully_ack_to_dequeue:    return "Command completed successfully; ack to dequeue";
            case command_completed_successfully_ending_session:    return "Command completed successfully; ending session";
        }
        return "unknown result"; // should not happen
    }

    static const char* c_str(EppResultCode::Failure _epp_result_code)
    {
        switch (_epp_result_code)
        {
            case unknown_command:                                  return "Unknown command";
            case command_syntax_error:                             return "Command syntax error";
            case command_use_error:                                return "Command use error";
            case required_parameter_missing:                       return "Required parameter missing";
            case parameter_value_range_error:                      return "Parameter value range error";
            case parameter_value_syntax_error:                     return "Parameter value syntax error";
            case unimplemented_protocol_version:                   return "Unimplemented protocol version";
            case unimplemented_command:                            return "Unimplemented command";
            case unimplemented_option:                             return "Unimplemented option";
            case unimplemented_extension:                          return "Unimplemented extension";
            case billing_failure:                                  return "Billing failure";
            case object_is_not_eligible_for_renewal:               return "Object is not eligible for renewal";
            case object_is_not_eligible_for_transfer:              return "Object is not eligible for transfer";
            case authentication_error:                             return "Authentication error";
            case authorization_error:                              return "Authorization error";
            case invalid_authorization_information:                return "Invalid authorization information";
            case object_pending_transfer:                          return "Object pending transfer";
            case object_not_pending_transfer:                      return "Object not pending transfer";
            case object_exists:                                    return "Object exists";
            case object_does_not_exist:                            return "Object does not exist";
            case object_status_prohibits_operation:                return "Object status prohibits operation";
            case object_association_prohibits_operation:           return "Object association prohibits operation";
            case parameter_value_policy_error:                     return "Parameter value policy error";
            case unimplemented_object_service:                     return "Unimplemented object service";
            case data_management_policy_violation:                 return "Data management policy violation";
            case command_failed:                                   return "Command failed";
            case command_failed_server_closing_connection:         return "Command failed; server closing connection";
            case authentication_error_server_closing_connection:   return "Authentication error; server closing connection";
            case session_limit_exceeded_server_closing_connection: return "Session limit exceeded; server closing connection";
        }
        return "unknown error"; // should not happen
    }

    static int to_description_db_id(const Success _epp_result_code) {
        return is_valid(_epp_result_code) ? static_cast<int>(_epp_result_code)
                                          : throw InvalidEppResultCodeValue();
    }

    static int to_description_db_id(const Failure _epp_result_code) {
        return is_valid(_epp_result_code) ? static_cast<int>(_epp_result_code)
                                          : throw InvalidEppResultCodeValue();
    }
};

} // namespace Epp

#endif
