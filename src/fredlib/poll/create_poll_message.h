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

    DECLARE_EXCEPTION_DATA(registrar_not_found, std::string);
    DECLARE_EXCEPTION_DATA(poll_message_type_not_found, std::string);
    struct Exception
    : virtual Fred::OperationException
    , ExceptionData_registrar_not_found<Exception>
    , ExceptionData_poll_message_type_not_found<Exception>
    {};

private:
    std::string registrar_handle_;
    std::string msg_type_;
};

}
}


#endif /*CREATE_POLL_MESSAGE_H__*/

