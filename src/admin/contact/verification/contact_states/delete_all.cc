#include "src/admin/contact/verification/contact_states/delete_all.h"
#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"

#include <boost/foreach.hpp>

namespace Admin
{
namespace AdminContactVerificationObjectStates
{

    bool conditionally_delete_all(
        Fred::OperationContext& ctx,
        unsigned long long contact_id,
        const Optional<std::string>& name,
        const Optional<std::string>& organization,
        const Optional<std::string>& street1,
        const Optional<std::string>& street2,
        const Optional<std::string>& street3,
        const Optional<std::string>& city,
        const Optional<std::string>& stateorprovince,
        const Optional<std::string>& postalcode,
        const Optional<std::string>& country,
        const Optional<std::string>& telephone,
        const Optional<std::string>& fax,
        const Optional<std::string>& email,
        const Optional<std::string>& notifyemail,
        const Optional<std::string>& vat,
        const Optional<std::string>& ssntype,
        const Optional<std::string>& ssn
    ) {
        if(    name.isset()
            || organization.isset()
            || street1.isset()
            || street2.isset()
            || street3.isset()
            || city.isset()
            || stateorprovince.isset()
            || postalcode.isset()
            || country.isset()
            || telephone.isset()
            || fax.isset()
            || email.isset()
            || notifyemail.isset()
            || vat.isset()
            || ssntype.isset()
            || ssn.isset()
        ) {
            delete_all(ctx, contact_id);

            return true;
        }

        return false;
    }

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

