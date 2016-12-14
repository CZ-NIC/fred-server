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

/**
 *  @file epp_exception.h
 *  <++>
 */

#ifndef EPP_EXCEPTION_H_31397E48DE5D4189A8C4A2E84F180DA9
#define EPP_EXCEPTION_H_31397E48DE5D4189A8C4A2E84F180DA9

#include "src/epp/impl/epp_result_code.h"

#include <exception>

namespace Epp {

class EppException : std::exception
{

public:

    /** Every EppException needs valid epp_result_code_ */
    EppException(EppResultCode::Enum _epp_result_code)
        : epp_result_code_(_epp_result_code)
    {
    }

    virtual const char* what() const throw() = 0;

    EppResultCode::Enum epp_result_code() const throw() {
        return epp_result_code_;
    }

protected:

    EppResultCode::Enum epp_result_code_;
    //const char* to_string() const throw();

};

struct CommandCompletedSuccessfully                : EppException { CommandCompletedSuccessfully()                : EppException(EppResultCode::command_completed_successfully)                   {} const char* what() const throw() { return "command completed successfully"; } };
struct CommandCompletedSuccessfullyActionPending   : EppException { CommandCompletedSuccessfullyActionPending()   : EppException(EppResultCode::command_completed_successfully_action_pending)    {} const char* what() const throw() { return "command completed successfully; action pending"; } };
struct CommandCompletedSuccessfullyNoMessages      : EppException { CommandCompletedSuccessfullyNoMessages()      : EppException(EppResultCode::command_completed_successfully_no_messages)       {} const char* what() const throw() { return "command completed successfully; no messages"; } };
struct CommandCompletedSuccessfullyAckToDequeue    : EppException { CommandCompletedSuccessfullyAckToDequeue()    : EppException(EppResultCode::command_completed_successfully_ack_to_dequeue)    {} const char* what() const throw() { return "command completed successfully; ack to dequeue"; } };
struct CommandCompletedSuccessfullyEndingSession   : EppException { CommandCompletedSuccessfullyEndingSession()   : EppException(EppResultCode::command_completed_successfully_ending_session)    {} const char* what() const throw() { return "command completed successfully; ending session"; } };
struct UnknownCommand                              : EppException { UnknownCommand()                              : EppException(EppResultCode::unknown_command)                                  {} const char* what() const throw() { return "unknown command"; } };
struct CommandSyntaxError                          : EppException { CommandSyntaxError()                          : EppException(EppResultCode::command_syntax_error)                             {} const char* what() const throw() { return "command syntax error"; } };
struct CommandUseError                             : EppException { CommandUseError()                             : EppException(EppResultCode::command_use_error)                                {} const char* what() const throw() { return "command use error"; } };
 struct RequiredParameterMissing                    : EppException { RequiredParameterMissing()                    : EppException(EppResultCode::required_parameter_missing)                       {} const char* what() const throw() { return "required parameter missing"; } };
//struct ParameterValueRangeError                    : EppException { ParameterValueRangeError()                    : EppException(EppResultCode::parameter_value_range_error)                      {} const char* what() const throw() { return "parameter value range error"; } };
struct ParameterValueSyntaxError                   : EppException { ParameterValueSyntaxError()                   : EppException(EppResultCode::parameter_value_syntax_error)                     {} const char* what() const throw() { return "parameter value syntax error"; } };
struct UnimplementedProtocolVersion                : EppException { UnimplementedProtocolVersion()                : EppException(EppResultCode::unimplemented_protocol_version)                   {} const char* what() const throw() { return "unimplemented protocol version"; } };
struct UnimplementedCommand                        : EppException { UnimplementedCommand()                        : EppException(EppResultCode::unimplemented_command)                            {} const char* what() const throw() { return "unimplemented command"; } };
struct UnimplementedOption                         : EppException { UnimplementedOption()                         : EppException(EppResultCode::unimplemented_option)                             {} const char* what() const throw() { return "unimplemented option"; } };
struct UnimplementedExtension                      : EppException { UnimplementedExtension()                      : EppException(EppResultCode::unimplemented_extension)                          {} const char* what() const throw() { return "unimplemented extension"; } };
struct BillingFailure                              : EppException { BillingFailure()                              : EppException(EppResultCode::billing_failure)                                  {} const char* what() const throw() { return "billing failure"; } };
struct ObjectIsNotEligibleForRenewal               : EppException { ObjectIsNotEligibleForRenewal()               : EppException(EppResultCode::object_is_not_eligible_for_renewal)               {} const char* what() const throw() { return "object is not eligible for renewal"; } };
struct ObjectIsNotEligibleForTransfer              : EppException { ObjectIsNotEligibleForTransfer()              : EppException(EppResultCode::object_is_not_eligible_for_transfer)              {} const char* what() const throw() { return "object is not eligible for transfer"; } };
struct AuthenticationError                         : EppException { AuthenticationError()                         : EppException(EppResultCode::authentication_error)                             {} const char* what() const throw() { return "authentication error"; } };
 struct AuthorizationError                          : EppException { AuthorizationError()                          : EppException(EppResultCode::authorization_error)                              {} const char* what() const throw() { return "authorization error"; } };
struct InvalidAuthorizationInformation             : EppException { InvalidAuthorizationInformation()             : EppException(EppResultCode::invalid_authorization_information)                {} const char* what() const throw() { return "invalid authorization information"; } };
struct ObjectPendingTransfer                       : EppException { ObjectPendingTransfer()                       : EppException(EppResultCode::object_pending_transfer)                          {} const char* what() const throw() { return "object pending transfer"; } };
struct ObjectNotPendingTransfer                    : EppException { ObjectNotPendingTransfer()                    : EppException(EppResultCode::object_not_pending_transfer)                      {} const char* what() const throw() { return "object not pending transfer"; } };
 struct ObjectExists                                : EppException { ObjectExists()                                : EppException(EppResultCode::object_exists)                                    {} const char* what() const throw() { return "object exists"; } };
struct ObjectDoesNotExist                          : EppException { ObjectDoesNotExist()                          : EppException(EppResultCode::object_does_not_exist)                            {} const char* what() const throw() { return "object does not exist"; } };
struct ObjectStatusProhibitsOperation              : EppException { ObjectStatusProhibitsOperation()              : EppException(EppResultCode::object_status_prohibits_operation)                {} const char* what() const throw() { return "object status prohibits operation"; } };
struct ObjectAssociationProhibitsOperation         : EppException { ObjectAssociationProhibitsOperation()         : EppException(EppResultCode::object_association_prohibits_operation)           {} const char* what() const throw() { return "object association prohibits operation"; } };
//struct ParameterValuePolicyError                   : EppException { ParameterValuePolicyError()                   : EppException(EppResultCode::parameter_value_policy_error)                     {} const char* what() const throw() { return "parameter value policy error"; } };
struct UnimplementedObjectService                  : EppException { UnimplementedObjectService()                  : EppException(EppResultCode::unimplemented_object_service)                     {} const char* what() const throw() { return "unimplemented object service"; } };
struct DataManagementPolicyViolation               : EppException { DataManagementPolicyViolation()               : EppException(EppResultCode::data_management_policy_violation)                 {} const char* what() const throw() { return "data management policy violation"; } };
struct CommandFailed                               : EppException { CommandFailed()                               : EppException(EppResultCode::command_failed)                                   {} const char* what() const throw() { return "command failed"; } };
struct CommandFailedServerClosingConnection        : EppException { CommandFailedServerClosingConnection()        : EppException(EppResultCode::command_failed_server_closing_connection)         {} const char* what() const throw() { return "command failed; server closing connection"; } };
struct AuthenticationErrorServerClosingConnection  : EppException { AuthenticationErrorServerClosingConnection()  : EppException(EppResultCode::authentication_error_server_closing_connection)   {} const char* what() const throw() { return "authentication error; server closing connection"; } };
struct SessionLimitExceededServerClosingConnection : EppException { SessionLimitExceededServerClosingConnection() : EppException(EppResultCode::session_limit_exceeded_server_closing_connection) {} const char* what() const throw() { return "session limit exceeded; server closing connection"; } };

} // namespace Epp

#endif
