#ifndef CREATE_POLL_MESSAGE_H__
#define CREATE_POLL_MESSAGE_H__

#include "fredlib/opcontext.h"
#include "fredlib/opexception.h"
#include "util/types/optional.h"


namespace Fred {
namespace Poll {


class CreatePollMessage
{
public:
    CreatePollMessage(
            const std::string &_registrar_handle,
            const std::string &_msg_type);

    unsigned long long exec(Fred::OperationContext &_ctx);


private:
    std::string registrar_handle_;
    std::string msg_type_;
};



class CreatePollMessageException
    : public OperationExceptionImpl<CreatePollMessageException, 8192>
{
public:
    CreatePollMessageException(
            const char *_file,
            const int _line,
            const char *_function,
            const char *_data)
        : OperationExceptionImpl<CreatePollMessageException, 8192>
                (_file, _line, _function, _data)
    {
    }

    ConstArr get_fail_param_impl() throw()
    {
        static const char* list[] = {
            "not found:registrar",
            "not found:poll message type"
        };
        return ConstArr(list, sizeof(list) / sizeof(char*));
    }
};


typedef CreatePollMessageException::OperationErrorType CreatePollMessageError;

}
}


#endif /*CREATE_POLL_MESSAGE_H__*/

