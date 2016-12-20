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
    { }

    virtual const char* what() const throw() = 0;

    EppResultCode::Enum epp_result_code() const throw() {
        return epp_result_code_;
    }

protected:

    EppResultCode::Enum epp_result_code_;
    //const char* to_string() const throw();

};

#define EPP_EXCEPTION(exception_name, epp_result_code, what_message) \
    struct exception_name : EppException \
    { \
        exception_name() \
        :   EppException(EppResultCode::epp_result_code) \
        { } \
        const char* what() const throw() { return what_message; } \
    };

EPP_EXCEPTION(CommandCompletedSuccessfully                , command_completed_successfully                   , "command completed successfully")
EPP_EXCEPTION(CommandCompletedSuccessfullyActionPending   , command_completed_successfully_action_pending    , "command completed successfully; action pending")
EPP_EXCEPTION(CommandCompletedSuccessfullyNoMessages      , command_completed_successfully_no_messages       , "command completed successfully; no messages")
EPP_EXCEPTION(CommandCompletedSuccessfullyAckToDequeue    , command_completed_successfully_ack_to_dequeue    , "command completed successfully; ack to dequeue")
EPP_EXCEPTION(CommandCompletedSuccessfullyEndingSession   , command_completed_successfully_ending_session    , "command completed successfully; ending session")
EPP_EXCEPTION(UnknownCommand                              , unknown_command                                  , "unknown command")
EPP_EXCEPTION(CommandSyntaxError                          , command_syntax_error                             , "command syntax error")
EPP_EXCEPTION(CommandUseError                             , command_use_error                                , "command use error")
EPP_EXCEPTION(RequiredParameterMissing                    , required_parameter_missing                       , "required parameter missing")
//EPP_EXCEPTION(ParameterValueRangeError                  , parameter_value_range_error                      , "parameter value range error")
//EPP_EXCEPTION(ParameterValueSyntaxError                 , parameter_value_syntax_error                     , "parameter value syntax error")
EPP_EXCEPTION(UnimplementedProtocolVersion                , unimplemented_protocol_version                   , "unimplemented protocol version")
EPP_EXCEPTION(UnimplementedCommand                        , unimplemented_command                            , "unimplemented command")
EPP_EXCEPTION(UnimplementedOption                         , unimplemented_option                             , "unimplemented option")
EPP_EXCEPTION(UnimplementedExtension                      , unimplemented_extension                          , "unimplemented extension")
EPP_EXCEPTION(BillingFailure                              , billing_failure                                  , "Billing failure")
EPP_EXCEPTION(ObjectIsNotEligibleForRenewal               , object_is_not_eligible_for_renewal               , "object is not eligible for renewal")
EPP_EXCEPTION(ObjectIsNotEligibleForTransfer              , object_is_not_eligible_for_transfer              , "object is not eligible for transfer")
EPP_EXCEPTION(AuthenticationError                         , authentication_error                             , "authentication error")
EPP_EXCEPTION(AuthorizationError                          , authorization_error                              , "authorization error")
EPP_EXCEPTION(InvalidAuthorizationInformation             , invalid_authorization_information                , "invalid authorization information")
EPP_EXCEPTION(ObjectPendingTransfer                       , object_pending_transfer                          , "object pending transfer")
EPP_EXCEPTION(ObjectNotPendingTransfer                    , object_not_pending_transfer                      , "object not pending transfer")
EPP_EXCEPTION(ObjectExists                                , object_exists                                    , "object exists")
EPP_EXCEPTION(ObjectDoesNotExist                          , object_does_not_exist                            , "object does not exist")
EPP_EXCEPTION(ObjectStatusProhibitsOperation              , object_status_prohibits_operation                , "object status prohibits operation")
EPP_EXCEPTION(ObjectAssociationProhibitsOperation         , object_association_prohibits_operation           , "object association prohibits operation")
//EPP_EXCEPTION(ParameterValuePolicyError                 , parameter_value_policy_error                     , "parameter value policy error")
EPP_EXCEPTION(UnimplementedObjectService                  , unimplemented_object_service                     , "unimplemented object service")
EPP_EXCEPTION(DataManagementPolicyViolation               , data_management_policy_violation                 , "data management policy violation")
EPP_EXCEPTION(CommandFailed                               , command_failed                                   , "command failed")
EPP_EXCEPTION(CommandFailedServerClosingConnection        , command_failed_server_closing_connection         , "command failed; server closing connection")
EPP_EXCEPTION(AuthenticationErrorServerClosingConnection  , authentication_error_server_closing_connection   , "authentication error; server closing connection")
EPP_EXCEPTION(SessionLimitExceededServerClosingConnection , session_limit_exceeded_server_closing_connection , "session limit exceeded; server closing connection")

} // namespace Epp

#endif
