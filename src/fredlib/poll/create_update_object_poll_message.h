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


private:
    ObjectHistoryId history_id_;
};


class CreateUpdateObjectPollMessageException
    : public OperationExceptionImpl<CreateUpdateObjectPollMessageException, 8192>
{
public:
    CreateUpdateObjectPollMessageException(
            const char *_file,
            const int _line,
            const char *_function,
            const char *_data)
        : OperationExceptionImpl<CreateUpdateObjectPollMessageException, 8192>
                (_file, _line, _function, _data)
    {
    }

    ConstArr get_fail_param_impl() throw()
    {
        static const char *list[] = {
            "not found:object history"
        };
        return ConstArr(list, sizeof(list) / sizeof(char*));
    }
};


typedef CreateUpdateObjectPollMessageException::OperationErrorType CreateUpdateObjectPollMessageError;

}
}


#endif /*CREATE_UPDATE_OBJECT_POLL_MESSAGE_H__*/

