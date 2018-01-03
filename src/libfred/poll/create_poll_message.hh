#ifndef CREATE_POLL_MESSAGE_HH_EB853FB4A0A44A39A8A8E8EBC25BB3CC
#define CREATE_POLL_MESSAGE_HH_EB853FB4A0A44A39A8A8E8EBC25BB3CC

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

#endif
