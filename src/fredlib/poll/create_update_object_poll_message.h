#ifndef CREATE_UPDATE_OBJECT_POLL_MESSAGE_H__
#define CREATE_UPDATE_OBJECT_POLL_MESSAGE_H__

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/printable.h"

/**
 *  @file
 *  create update object poll message
 */

namespace Fred {
namespace Poll {


class CreateUpdateObjectPollMessage : public Util::Printable
{
public:
    typedef unsigned long long ObjectHistoryId;

    /**
    * @param _history_id specific history version of domain to which the new message shall be related
    */
    CreateUpdateObjectPollMessage(const ObjectHistoryId &_history_id);

    /**
    * @throws Exception
    */
    void exec(Fred::OperationContext &_ctx);

    DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
    struct Exception
    : virtual Fred::OperationException
    , ExceptionData_object_history_not_found<Exception>
    {};

    /**
    * @return string with description of the instance state
    */
    std::string to_string() const;

private:
    ObjectHistoryId history_id_;
};

}
}


#endif /*CREATE_UPDATE_OBJECT_POLL_MESSAGE_H__*/

