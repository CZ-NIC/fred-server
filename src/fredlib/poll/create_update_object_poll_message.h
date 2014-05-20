#ifndef CREATE_UPDATE_OBJECT_POLL_MESSAGE_H__
#define CREATE_UPDATE_OBJECT_POLL_MESSAGE_H__

#include "fredlib/opcontext.h"
#include "fredlib/opexception.h"


namespace Fred {
namespace Poll {


class CreateUpdateObjectPollMessage
{
public:
    typedef unsigned long long ObjectHistoryId;

    CreateUpdateObjectPollMessage(const ObjectHistoryId &_history_id);

    void exec(Fred::OperationContext &_ctx);

    DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
    struct Exception
    : virtual Fred::OperationException
    , ExceptionData_object_history_not_found<Exception>
    {
        const char* what() const throw()
        {
            return "CreateUpdateObjectPollMessage::Exception";
        }
    };

private:
    ObjectHistoryId history_id_;
};

}
}


#endif /*CREATE_UPDATE_OBJECT_POLL_MESSAGE_H__*/

