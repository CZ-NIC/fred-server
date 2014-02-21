#ifndef CREATE_POLL_MESSAGE_H__
#define CREATE_POLL_MESSAGE_H__

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/types/optional.h"
#include "util/printable.h"

/**
 *  @file
 *  common implementation for creating poll messages
 */

namespace Fred {
namespace Poll {


class CreatePollMessage : public Util::Printable
{
public:
    /**
    * @param _registrar_handle handle of registrar for whom the message is
    * @param _msg_type type of message, valid types are in db table "messagetype"
    */
    CreatePollMessage(
            const std::string &_registrar_handle,
            const std::string &_msg_type);

    /**
    * @return id of newly created message
    * @throws Exception
    */
    unsigned long long exec(Fred::OperationContext &_ctx);

    DECLARE_EXCEPTION_DATA(registrar_not_found, std::string);
    DECLARE_EXCEPTION_DATA(poll_message_type_not_found, std::string);
    struct Exception
    : virtual Fred::OperationException
    , ExceptionData_registrar_not_found<Exception>
    , ExceptionData_poll_message_type_not_found<Exception>
    {};

    /**
    * @return string with description of the instance state
    */
    std::string to_string() const;

private:
    std::string registrar_handle_;
    std::string msg_type_;
};

}
}


#endif /*CREATE_POLL_MESSAGE_H__*/

