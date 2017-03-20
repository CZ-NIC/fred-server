#ifndef CREATE_UPDATE_OBJECT_POLL_MESSAGE_H_72EDC4E50CA9441B8CFEB009CE910DC9
#define CREATE_UPDATE_OBJECT_POLL_MESSAGE_H_72EDC4E50CA9441B8CFEB009CE910DC9

#include "src/fredlib/opcontext.h"
#include "src/fredlib/poll/exception.h"

/**
 *  @file
 *  create update object poll message
 */

namespace Fred {
namespace Poll {

class CreateUpdateObjectPollMessage
{
public:
    struct Exception:virtual Fred::Poll::Exception
    {
        struct ObjectHistoryNotFound:virtual Fred::Poll::Exception
        {
            ObjectHistoryNotFound(unsigned long long _history_id):history_id(_history_id) { }
            const unsigned long long history_id;
        };
        struct ObjectOfUnsupportedType:virtual Fred::Poll::Exception { };
    };

    /**
     * @param _ctx operation context
     * @param _history_id specific history version of object to which the new message shall be related
     * @throws Exception all operation specific exceptions are derived from this class
     * @throws Exception::ObjectHistoryNotFound when history_id_ is not related to any update message type
     * @throws Exception::ObjectOfUnsupportedType when type of updated object does not belong to the set
     *         of supported types
     */
    void exec(Fred::OperationContext& _ctx, unsigned long long _history_id)const;
};

}//namespace Fred::Poll
}//namespace Fred

#endif
