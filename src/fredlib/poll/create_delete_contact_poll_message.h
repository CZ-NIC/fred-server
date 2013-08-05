#ifndef CREATE_DELETE_CONTACT_POLL_MESSAGE_H__
#define CREATE_DELETE_CONTACT_POLL_MESSAGE_H__

#include "fredlib/opcontext.h"
#include "fredlib/opexception.h"


namespace Fred {
namespace Poll {


class CreateDeleteContactPollMessage
{
public:
    typedef unsigned long long ObjectHistoryId;

    CreateDeleteContactPollMessage(const ObjectHistoryId &_history_id);

    void exec(Fred::OperationContext &_ctx);

    DECLARE_EXCEPTION_DATA(contact_not_found, unsigned long long);
    DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
    struct Exception
    : virtual Fred::OperationException
    , ExceptionData_contact_not_found<Exception>
    , ExceptionData_object_history_not_found<Exception>
    {};

private:
    ObjectHistoryId history_id_;
};

}
}


#endif /*CREATE_DELETE_CONTACT_POLL_MESSAGE_H__*/

