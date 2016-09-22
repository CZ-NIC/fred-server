#include "src/fredlib/poll/message_types.h"
#include "src/fredlib/poll/create_transfer_domain_poll_message.h"
#include "src/fredlib/poll/create_epp_action_poll_message_impl.h"

#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


namespace Fred {
namespace Poll {

namespace {

std::string message_type_handle() { return TRANSFER_DOMAIN; }

}

CreateTransferDomainPollMessage::CreateTransferDomainPollMessage(ObjectHistoryId _history_id)
:   history_id_(_history_id)
{
}

unsigned long long CreateTransferDomainPollMessage::exec(Fred::OperationContext &_ctx)
{
    try {
        return CreateEppActionPollMessage(history_id_, domain, message_type_handle()).exec(_ctx);
    }
    catch (const CreateEppActionPollMessage::Exception &e) {
        if (e.is_set_object_history_not_found()) {
            Exception my_exception;

            my_exception.set_object_history_not_found(e.get_object_history_not_found())
                        .add_exception_stack_info(this->to_string());

            BOOST_THROW_EXCEPTION(my_exception);
        }

        if (e.is_set_object_type_not_found()) {
            Exception my_exception;

            my_exception.set_domain_not_found(e.get_object_type_not_found().second)
                        .add_exception_stack_info(this->to_string());

            BOOST_THROW_EXCEPTION(my_exception);
        }

        // if (maybe in future) implementation Exception contains some other fragments don't swallow it...
        throw;
    }
}

std::string CreateTransferDomainPollMessage::to_string()const
{
    return Util::format_operation_state(
        "CreateTransferDomainPollMessage",
        boost::assign::list_of(std::make_pair("history_id", boost::lexical_cast< std::string >(history_id_))));
}

}
}

