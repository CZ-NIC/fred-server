#include "src/admin/contact/verification/contact_states/delete_all.h"
#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"

#include <boost/foreach.hpp>

namespace Admin
{
namespace AdminContactVerificationObjectStates
{
    void delete_all(Fred::OperationContext& _ctx, unsigned long long _contact_id) {

        _ctx.get_conn().exec("SAVEPOINT state_savepoint");

        // cancel one state at a time because when exception is thrown, all changes would be ROLLBACKed
        BOOST_FOREACH(
            const std::string& object_state,
            Admin::AdminContactVerificationObjectStates::get_all()
        ) {
            std::set<std::string> object_states_to_erase = boost::assign::list_of(object_state);
            try {
                Fred::CancelObjectStateRequestId(
                        _contact_id,
                    object_states_to_erase
                ).exec(_ctx);
                _ctx.get_conn().exec("RELEASE SAVEPOINT state_savepoint");
                _ctx.get_conn().exec("SAVEPOINT state_savepoint");
            } catch(Fred::CancelObjectStateRequestId::Exception& e) {
                // in case it throws with unknown cause
                if(e.is_set_state_not_found() == false) {
                    throw;
                } else {
                    _ctx.get_conn().exec("ROLLBACK TO state_savepoint");
                }
            }
        }
    }
}
}

