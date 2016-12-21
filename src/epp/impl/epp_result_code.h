/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef EPP_RESULT_CODE_H_BFC3F7DA349447F9B0A921B0174E9923
#define EPP_RESULT_CODE_H_BFC3F7DA349447F9B0A921B0174E9923

struct EppResultCode
{
    enum Enum
    {
        command_completed_successfully                   = 1000,
        command_completed_successfully_action_pending    = 1001,
        command_completed_successfully_no_messages       = 1300,
        command_completed_successfully_ack_to_dequeue    = 1301,
        command_completed_successfully_ending_session    = 1500,
        unknown_command                                  = 2000,
        command_syntax_error                             = 2001,
        command_use_error                                = 2002,
        required_parameter_missing                       = 2003,
        parameter_value_range_error                      = 2004,
        parameter_value_syntax_error                     = 2005,
        unimplemented_protocol_version                   = 2100,
        unimplemented_command                            = 2101,
        unimplemented_option                             = 2102,
        unimplemented_extension                          = 2103,
        billing_failure                                  = 2104,
        object_is_not_eligible_for_renewal               = 2105,
        object_is_not_eligible_for_transfer              = 2106,
        authentication_error                             = 2200,
        authorization_error                              = 2201,
        invalid_authorization_information                = 2202,
        object_pending_transfer                          = 2300,
        object_not_pending_transfer                      = 2301,
        object_exists                                    = 2302,
        object_does_not_exist                            = 2303,
        object_status_prohibits_operation                = 2304,
        object_association_prohibits_operation           = 2305,
        parameter_value_policy_error                     = 2306,
        unimplemented_object_service                     = 2307,
        data_management_policy_violation                 = 2308,
        command_failed                                   = 2400,
        command_failed_server_closing_connection         = 2500,
        authentication_error_server_closing_connection   = 2501,
        session_limit_exceeded_server_closing_connection = 2502,
    };

    static const char* to_string(EppResultCode::Enum _epp_result_code)
    {
        switch (_epp_result_code)
        {
            case command_completed_successfully:                   return "command completed successfully";
            case command_completed_successfully_action_pending:    return "command completed successfully; action pending";
            case command_completed_successfully_no_messages:       return "command completed successfully; no messages";
            case command_completed_successfully_ack_to_dequeue:    return "command completed successfully; ack to dequeue";
            case command_completed_successfully_ending_session:    return "command completed successfully; ending session";
            case unknown_command:                                  return "unknown command";
            case command_syntax_error:                             return "command syntax error";
            case command_use_error:                                return "command use error";
            case required_parameter_missing:                       return "required parameter missing";
            case parameter_value_range_error:                      return "parameter value range error";
            case parameter_value_syntax_error:                     return "parameter value syntax error";
            case unimplemented_protocol_version:                   return "unimplemented protocol version";
            case unimplemented_command:                            return "unimplemented command";
            case unimplemented_option:                             return "unimplemented option";
            case unimplemented_extension:                          return "unimplemented extension";
            case billing_failure:                                  return "Billing failure";
            case object_is_not_eligible_for_renewal:               return "object is not eligible for renewal";
            case object_is_not_eligible_for_transfer:              return "object is not eligible for transfer";
            case authentication_error:                             return "authentication error";
            case authorization_error:                              return "authorization error";
            case invalid_authorization_information:                return "invalid authorization information";
            case object_pending_transfer:                          return "object pending transfer";
            case object_not_pending_transfer:                      return "object not pending transfer";
            case object_exists:                                    return "object exists";
            case object_does_not_exist:                            return "object does not exist";
            case object_status_prohibits_operation:                return "object status prohibits operation";
            case object_association_prohibits_operation:           return "object association prohibits operation";
            case parameter_value_policy_error:                     return "parameter value policy error";
            case unimplemented_object_service:                     return "unimplemented object service";
            case data_management_policy_violation:                 return "data management policy violation";
            case command_failed:                                   return "command failed";
            case command_failed_server_closing_connection:         return "command failed; server closing connection";
            case authentication_error_server_closing_connection:   return "authentication error; server closing connection";
            case session_limit_exceeded_server_closing_connection: return "session limit exceeded; server closing connection";
        }
    }
};

#endif
