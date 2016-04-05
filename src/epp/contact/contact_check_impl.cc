#include "src/epp/contact/contact_check_impl.h"

#include "src/fredlib/contact/check_contact.h"

#include <boost/foreach.hpp>

namespace Epp {

std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > contact_check_impl(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _contact_handles
) {
    std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > result;

    BOOST_FOREACH(const std::string& handle, _contact_handles) {
        Fred::CheckContact check(handle);

        if(check.is_invalid_handle()) {
            result[handle] = ContactHandleRegistrationObstruction::invalid_handle;

        } else if(check.is_protected(_ctx)) {
            result[handle] = ContactHandleRegistrationObstruction::protected_handle;

        } else if(check.is_registered(_ctx)) {
            result[handle] = ContactHandleRegistrationObstruction::registered_handle;

        } else {
            result[handle] = Nullable<ContactHandleRegistrationObstruction::Enum>();

        }
    }

    return result;
}

}
