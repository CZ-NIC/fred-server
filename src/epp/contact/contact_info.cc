#include "src/epp/contact/contact_info.h"

#include "src/epp/contact/contact_info_impl.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/localization.h"
#include "src/epp/action.h"

#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/contact/info_contact.h"

#include "util/log/context.h"

#include <algorithm>
#include <set>
#include <vector>
#include <boost/foreach.hpp>

namespace Epp {

LocalizedInfoContactResponse contact_info(
    const std::string& _handle,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
) {
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
    Logging::Context logging_ctx3(_server_transaction_handle);
    Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::ContactInfo)));

    /* since no changes are comitted this transaction is reused for everything */

    Fred::OperationContextCreator ctx;

    try {

        ContactInfoOutputData payload = contact_info_impl(ctx, _handle, _lang, _registrar_id);

        /* show object authinfo only to sponsoring registrar */
        if(payload.sponsoring_registrar_handle != Fred::InfoRegistrarById(_registrar_id).exec(ctx).info_registrar_data.handle) {
            payload.auth_info_pw = std::string();
        }

        // hide internal states
        {
            std::set<std::string> filtered_states;

            // TODO udelat pres ziskani externich stavu, zatim na to neni ve fredlibu rozhrani
            const std::vector<Fred::ObjectStateData> state_definitions =
                Fred::GetObjectStates(
                    Fred::InfoContactByHandle(payload.handle).exec(ctx).info_contact_data.id
                ).exec(ctx);

            BOOST_FOREACH(const std::string& state, payload.states) {
                BOOST_FOREACH(const Fred::ObjectStateData& state_def, state_definitions) {
                    if(state_def.is_external) {
                        filtered_states.insert(state);
                    }
                }
            }

            payload.states = filtered_states;
        }

        /* XXX HACK: OK state */
        if( payload.states.empty() ) {
            payload.states.insert("ok");
        }

        /* XXX HACK: Ticket #10053 - temporary hack until changed xml schemas are released upon poor registrars
         * Do not propagate admin contact verification states.
         */
        {
            std::set<std::string> filtered_states;

            const std::vector<std::string> admin_contact_verification_states = Admin::AdminContactVerificationObjectStates::get_all();
            BOOST_FOREACH(const std::string& state, payload.states) {
                if( std::find(
                        admin_contact_verification_states.begin(),
                        admin_contact_verification_states.end(),
                        state
                    ) == admin_contact_verification_states.end()
                ) {
                    filtered_states.insert(state);
                }
            }

            payload.states = filtered_states;
        }

        return LocalizedInfoContactResponse(
            create_localized_success_response(Response::ok, ctx, _lang),
            LocalizedContactInfoOutputData(
                payload.handle,
                payload.roid,
                payload.sponsoring_registrar_handle,
                payload.creating_registrar_handle,
                payload.last_update_registrar_handle,
                get_object_state_descriptions(ctx, payload.states, _lang),
                payload.crdate,
                payload.last_update,
                payload.last_transfer,
                payload.name,
                payload.organization,
                payload.street1,
                payload.street2,
                payload.street3,
                payload.city,
                payload.state_or_province,
                payload.postal_code,
                payload.country_code,
                payload.telephone,
                payload.fax,
                payload.email,
                payload.notify_email,
                payload.VAT,
                payload.ident,
                payload.identtype,
                payload.auth_info_pw,
                payload.disclose_name,
                payload.disclose_organization,
                payload.disclose_address,
                payload.disclose_telephone,
                payload.disclose_fax,
                payload.disclose_email,
                payload.disclose_VAT,
                payload.disclose_ident,
                payload.disclose_notify_email
            )
        );

    } catch (const AuthErrorServerClosingConnection& e) {
        throw create_localized_fail_response(
            ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );

    } catch (const InvalidHandle& e) {
        throw create_localized_fail_response(
            ctx,
            Response::parametr_error,
            Error( Param::contact_handle, 1, Reason::bad_format_contact_handle ),
            _lang
        );

    } catch (const NonexistentHandle& e) {
        throw create_localized_fail_response(
            ctx,
            Response::object_not_exist,
            std::set<Error>(),
            _lang
        );
    }
}

}
