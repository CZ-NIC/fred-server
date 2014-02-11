#include "create_delete_domain_poll_message.h"
#include "create_epp_action_poll_message_impl.h"

#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


namespace Fred {
namespace Poll {


CreateDeleteDomainPollMessage::CreateDeleteDomainPollMessage(
    ObjectHistoryId _history_id
) :
    history_id_(_history_id)
{ }

unsigned long long CreateDeleteDomainPollMessage::exec(Fred::OperationContext &_ctx) {
    try {
        return
            Fred::Poll::CreateEppActionPollMessage(
                history_id_,
                Fred::Poll::domain,
                message_type_handle()
            ).exec(_ctx);
    } catch (CreateEppActionPollMessage::Exception& e) {
        if(e.is_set_object_history_not_found()) {
            Exception new_e;

            new_e
                .set_object_history_not_found(
                    e.get_object_history_not_found()
                ).add_exception_stack_info(
                    this->to_string() );

            throw new_e;
        }

        if(e.is_set_object_type_not_found()) {
            Exception new_e;

            new_e
                .set_domain_not_found(
                    e.get_object_type_not_found().second
                ).add_exception_stack_info(
                    this->to_string() );

            throw new_e;
        }

        // if (maybe in future) implementation Exception contains some other fragments don't swallow it...
        throw;
    }
}

std::string CreateDeleteDomainPollMessage::to_string() const {

    return Util::format_operation_state(
        "CreateDeleteDomainPollMessage",
        boost::assign::list_of
            (std::make_pair("history_id",boost::lexical_cast<std::string>(history_id_)))
    );
}

}
}

