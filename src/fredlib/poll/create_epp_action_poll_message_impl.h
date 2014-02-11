#ifndef CREATE_EPP_ACTION_POLL_MESSAGE_H_215465531212_
#define CREATE_EPP_ACTION_POLL_MESSAGE_H_215465531212_

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "util/printable.h"

#include <exception>
#include <utility>

namespace Fred {
namespace Poll {

enum object_type {domain, contact, keyset, nsset};

inline std::string to_registry_handle(object_type _in) {
    switch(_in) {
        case domain:
            return "domain";
        case contact:
            return "contact";
        case keyset:
            return "keyset";
        case nsset:
            return "nsset";
        default:
            throw std::runtime_error("uknonwn registry object type");
    };
}

class CreateEppActionPollMessage : public Util::Printable
{
    public:
        typedef unsigned long long ObjectHistoryId;
        typedef std::pair<std::string, unsigned long long> string_ull_pair;

        DECLARE_EXCEPTION_DATA(object_history_not_found, unsigned long long);
        DECLARE_EXCEPTION_DATA(object_type_not_found, string_ull_pair);
        struct Exception
        :
            virtual Fred::OperationException,
            ExceptionData_object_history_not_found<Exception>,
            ExceptionData_object_type_not_found<Exception>
        { };

        CreateEppActionPollMessage(
            ObjectHistoryId     _history_id,
            object_type         _object_type,
            const std::string&  _message_type_handle);

        unsigned long long exec(Fred::OperationContext &_ctx);

        virtual std::string to_string() const;

    private:
        const ObjectHistoryId   history_id_;
        const object_type       object_type_;
        const std::string       message_type_handle_;
};

}
}


#endif

