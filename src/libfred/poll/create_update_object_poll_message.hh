#ifndef CREATE_UPDATE_OBJECT_POLL_MESSAGE_H_72EDC4E50CA9441B8CFEB009CE910DC9
#define CREATE_UPDATE_OBJECT_POLL_MESSAGE_H_72EDC4E50CA9441B8CFEB009CE910DC9

#include "src/libfred/opcontext.hh"

/**
 *  @file
 *  create update object poll message
 */

namespace LibFred {
namespace Poll {

class CreateUpdateObjectPollMessage
{
public:
    /**
     * @param _ctx operation context
     * @param _history_id specific history version of object to which the new message shall be related
     * @throws LibFred::OperationException an operation specific exception
     * @throws LibFred::InternalError an unexpected exception
     */
    void exec(LibFred::OperationContext& _ctx, unsigned long long _history_id)const;
};

} // namespace LibFred::Poll
} // namespace LibFred

#endif
