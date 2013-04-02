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


private:
    ObjectHistoryId history_id_;
};


class CreateDeleteContactPollMessageException
    : public OperationExceptionImpl<CreateDeleteContactPollMessageException, 8192>
{
public:
    CreateDeleteContactPollMessageException(
            const char *_file,
            const int _line,
            const char *_function,
            const char *_data)
        : OperationExceptionImpl<CreateDeleteContactPollMessageException, 8192>
                (_file, _line, _function, _data)
    {
    }

    ConstArr get_fail_param_impl() throw()
    {
        static const char *list[] = {
            "not found:object history",
            "not found:contact"
        };
        return ConstArr(list, sizeof(list) / sizeof(char*));
    }
};


}
}


#endif /*CREATE_DELETE_CONTACT_POLL_MESSAGE_H__*/

