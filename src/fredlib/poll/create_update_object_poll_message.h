#ifndef CREATE_UPDATE_OBJECT_POLL_MESSAGE_H_72EDC4E50CA9441B8CFEB009CE910DC9
#define CREATE_UPDATE_OBJECT_POLL_MESSAGE_H_72EDC4E50CA9441B8CFEB009CE910DC9

#include "src/fredlib/opcontext.h"

/**
 *  @file
 *  create update object poll message
 */

namespace Fred {
namespace Poll {

class CreateUpdateObjectPollMessage
{
public:
    /**
     * @param _ctx operation context
     * @param _history_id specific history version of object to which the new message shall be related
     * @throws Fred::OperationException an operation specific exception
     * @throws Fred::InternalError an unexpected exception
     */
    void exec(Fred::OperationContext& _ctx, unsigned long long _history_id)const;
};

}//namespace Fred::Poll
}//namespace Fred

#endif
