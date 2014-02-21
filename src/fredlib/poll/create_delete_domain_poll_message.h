#ifndef CREATE_DELETE_DOMAIN_POLL_MESSAGE_H_468854341212_
#define CREATE_DELETE_DOMAIN_POLL_MESSAGE_H_468854341212_

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/printable.h"

/**
 *  @file
 *  create delete domain poll message
 */

namespace Fred {
namespace Poll {


class CreateDeleteDomainPollMessage : public Util::Printable {
public:
    typedef unsigned long long ObjectHistoryId;

    DECLARE_EXCEPTION_DATA(domain_not_found, unsigned long long);
    DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
    struct Exception
    :
        virtual Fred::OperationException,
        ExceptionData_domain_not_found<Exception>,
        ExceptionData_object_history_not_found<Exception>
    { };

    /**
    * @param _history_id specific history version of domain to which the new message shall be related
    */
    CreateDeleteDomainPollMessage(ObjectHistoryId _history_id);

    /**
    * @return id of newly created message
    * @throws Exception
    */
    unsigned long long exec(Fred::OperationContext &_ctx);

    /**
    * @return string with description of the instance state
    */
    std::string to_string() const;

private:
    ObjectHistoryId history_id_;

    static std::string message_type_handle() { return "delete_domain"; }
};

}
}


#endif

