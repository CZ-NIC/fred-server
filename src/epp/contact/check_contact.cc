#include "src/epp/contact/check_contact.h"

#include "src/fredlib/contact/check_contact.h"
#include "src/epp/contact/impl/contact_handle_state_to_check_result.h"

#include <boost/foreach.hpp>

namespace Epp {

std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > contact_check_impl(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _contact_handles
) {
    std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > result;

    BOOST_FOREACH(const std::string& handle, _contact_handles) {

        result[handle] = contact_handle_state_to_check_result(
            Fred::Contact::get_handle_syntax_validity(handle),
            Fred::Contact::get_handle_registrability(_ctx, handle)
        );
    }

    return result;
}

}
