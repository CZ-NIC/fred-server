#ifndef CREATE_EPP_ACTION_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C
#define CREATE_EPP_ACTION_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/poll/exception.h"
#include "src/fredlib/poll/message_type.h"

#include <stdexcept>

/**
 *  @file
 *  common implementation for creating epp action poll messages
 */

namespace Fred {
namespace Poll {

struct CreateEppActionPollMessage
{
    struct Exception:virtual Fred::Poll::Exception
    {
        struct ObjectHistoryNotFound:virtual Fred::Poll::Exception
        {
            ObjectHistoryNotFound(unsigned long long _history_id):history_id(_history_id) { }
            const unsigned long long history_id;
        };
        struct ObjectNotRequestedType:virtual Fred::Poll::Exception { };
    };
    /**
     * @tparam message_type create message of given type
     */
    template <MessageType::Enum message_type>
    struct Of
    {
        /**
         * @param _ctx operation context
         * @param _history_id specific history version of registry object to which the new message shall be related
         * @return id of newly created message
         * @throws Exception all operation specific exceptions are derived from this class
         * @throws Exception::ObjectHistoryNotFound
         * @throws Exception::ObjectNotRequestedType
         */
        unsigned long long exec(Fred::OperationContext& _ctx, unsigned long long _history_id)const;
    };
};

}//namespace Fred::Poll
}//namespace Fred

#endif//CREATE_EPP_ACTION_POLL_MESSAGE_H_92CDA257099444809C84250555C6178C
