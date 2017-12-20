#ifndef CREATE_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C
#define CREATE_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C

#include "src/libfred/opcontext.hh"
#include "src/libfred/opexception.hh"
#include "src/libfred/poll/message_type.hh"

/**
 *  @file
 *  common implementation for creating epp action poll messages
 */

namespace LibFred {
namespace Poll {

/**
 * @tparam message_type create message of given type
 */
template <MessageType::Enum message_type>
struct CreatePollMessage
{
    /**
     * @param _ctx operation context
     * @param _history_id specific history version of registry object to which the new message shall be related
     * @return id of newly created message
     * @throws LibFred::OperationException an operation specific exception
     * @throws LibFred::InternalError an unexpected exception
     */
    unsigned long long exec(LibFred::OperationContext& _ctx, unsigned long long _history_id)const;
};

} // namespace LibFred::Poll
} // namespace LibFred

#endif//CREATE_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C
