#ifndef CREATE_UPDATE_OBJECT_POLL_MESSAGE_HH_DAC36D20F83C4552BA16FD307CD80D1F
#define CREATE_UPDATE_OBJECT_POLL_MESSAGE_HH_DAC36D20F83C4552BA16FD307CD80D1F

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
