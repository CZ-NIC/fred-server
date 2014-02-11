#ifndef CREATE_DELETE_CONTACT_POLL_MESSAGE_H__
#define CREATE_DELETE_CONTACT_POLL_MESSAGE_H__

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/printable.h"


namespace Fred {
namespace Poll {


class CreateDeleteContactPollMessage : public Util::Printable
{
public:
    typedef unsigned long long ObjectHistoryId;

    DECLARE_EXCEPTION_DATA(contact_not_found, unsigned long long);
    DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
    struct Exception
    :
        virtual Fred::OperationException,
        ExceptionData_contact_not_found<Exception>,
        ExceptionData_object_history_not_found<Exception>
    { };

    CreateDeleteContactPollMessage(const ObjectHistoryId &_history_id);

    unsigned long long exec(Fred::OperationContext &_ctx);

    std::string to_string() const;

private:
    ObjectHistoryId history_id_;

    static std::string message_type_handle() { return "delete_contact"; }
};

}
}


#endif /*CREATE_DELETE_CONTACT_POLL_MESSAGE_H__*/

