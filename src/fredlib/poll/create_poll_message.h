#ifndef CREATE_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C
#define CREATE_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/poll/message_type.h"

/**
 *  @file
 *  common implementation for creating epp action poll messages
 */

namespace Fred {
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
     * @throws Fred::OperationException an operation specific exception
     * @throws Fred::InternalError an unexpected exception
     */
    unsigned long long exec(Fred::OperationContext& _ctx, unsigned long long _history_id)const;
};

}//namespace Fred::Poll
}//namespace Fred

#endif//CREATE_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C
