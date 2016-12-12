#include "src/epp/contact/info_contact_localized.h"

#include "src/epp/contact/info_contact.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/util.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/action.h"

#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/contact/info_contact.h"

#include "util/log/context.h"

#include <algorithm>
#include <set>
#include <vector>
#include <boost/foreach.hpp>

namespace Epp {
namespace Contact {

namespace {

class FilterOut
{
public:
    static FilterOut what(const std::vector< std::string > &_disallowed) { return FilterOut(_disallowed); }
    std::set< std::string >& from(std::set< std::string > &_values)const
    {
        for (std::vector< std::string >::const_iterator disallowed_value_ptr = disallowed_.begin();
             disallowed_value_ptr != disallowed_.end(); ++disallowed_value_ptr)
        {
            std::set< std::string >::iterator value_to_remove = _values.find(*disallowed_value_ptr);
            const bool remove_it = value_to_remove != _values.end();
            if (remove_it) {
                _values.erase(value_to_remove);
            }
        }
        return _values;
    }
private:
    FilterOut(const std::vector< std::string > &_disallowed):disallowed_(_disallowed) { }
    const std::vector< std::string > disallowed_;
};

}//namespace Epp::{anonymous}

InfoContactLocalizedOutputData::InfoContactLocalizedOutputData(const boost::optional< ContactDisclose > &_disclose)
:   disclose(_disclose)
{
}

InfoContactLocalizedResponse::InfoContactLocalizedResponse(
    const LocalizedSuccessResponse &_ok_response,
    const InfoContactLocalizedOutputData &_payload)
:   ok_response(_ok_response),
    payload(_payload)
{
}

InfoContactLocalizedResponse info_contact_localized(
    const std::string &_handle,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string &_server_transaction_handle)
{
    try {
        Logging::Context logging_ctx1("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>(Action::InfoContact)));

        Fred::OperationContextCreator ctx;

        const InfoContactOutputData info = info_contact(ctx, _handle, _lang, _registrar_id);

        const std::string callers_registrar_handle = Fred::InfoRegistrarById(_registrar_id).exec(ctx).info_registrar_data.handle;
        const bool callers_is_sponsoring_registrar = info.sponsoring_registrar_handle == callers_registrar_handle;
        const bool authinfo_has_to_be_hidden = !callers_is_sponsoring_registrar;

        InfoContactLocalizedOutputData output_data(info.disclose);
        output_data.handle                       = info.handle;
        output_data.roid                         = info.roid;
        output_data.sponsoring_registrar_handle  = info.sponsoring_registrar_handle;
        output_data.creating_registrar_handle    = info.creating_registrar_handle;
        output_data.last_update_registrar_handle = info.last_update_registrar_handle;
        {//compute output_data.localized_external_states
            const std::vector< std::string > admin_contact_verification_states =
                Admin::AdminContactVerificationObjectStates::get_all();

            std::set< std::string > filtered_states = info.states;
            /* XXX HACK: Ticket #10053 - temporary hack until changed xml schemas are released upon poor registrars
             * Do not propagate admin contact verification states.
             */
            FilterOut::what(admin_contact_verification_states).from(filtered_states);

            if (filtered_states.empty()) {//XXX HACK: OK state
                static const char *const ok_state_name = "ok";
                filtered_states.insert(ok_state_name);
            }
            output_data.localized_external_states = get_object_state_descriptions(ctx, filtered_states, _lang);
        }

        output_data.crdate            = info.crdate;
        output_data.last_update       = info.last_update;
        output_data.last_transfer     = info.last_transfer;
        output_data.name              = info.name;
        output_data.organization      = info.organization;
        output_data.street1           = info.street1;
        output_data.street2           = info.street2;
        output_data.street3           = info.street3;
        output_data.city              = info.city;
        output_data.state_or_province = info.state_or_province;
        output_data.postal_code       = info.postal_code;
        output_data.country_code      = info.country_code;
        output_data.telephone         = info.telephone;
        output_data.fax               = info.fax;
        output_data.email             = info.email;
        output_data.notify_email      = info.notify_email;
        output_data.VAT               = info.VAT;
        if (info.personal_id.is_initialized()) {
            output_data.ident         = info.personal_id->get();
            if (info.personal_id->get_type() == Fred::PersonalIdUnion::get_OP("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::op;
            }
            else if (info.personal_id->get_type() == Fred::PersonalIdUnion::get_PASS("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::pass;
            }
            else if (info.personal_id->get_type() == Fred::PersonalIdUnion::get_ICO("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::ico;
            }
            else if (info.personal_id->get_type() == Fred::PersonalIdUnion::get_MPSV("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::mpsv;
            }
            else if (info.personal_id->get_type() == Fred::PersonalIdUnion::get_BIRTHDAY("").get_type())
            {
                output_data.identtype = InfoContactLocalizedOutputData::IdentType::birthday;
            }
            else
            {
                throw std::runtime_error("Invalid ident type.");
            }
        }
        output_data.auth_info_pw      = authinfo_has_to_be_hidden ? Nullable< std::string >() : info.auth_info_pw;

        return InfoContactLocalizedResponse(
            create_localized_success_response(ctx, Response::ok, _lang),
            output_data);

    } catch (const AuthErrorServerClosingConnection& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );

    } catch (const NonexistentHandle& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::object_not_exist,
            std::set<Error>(),
            _lang
        );

    } catch(const LocalizedFailResponse&) {
        throw;

    } catch(...) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::failed,
            std::set<Error>(),
            _lang
        );
    }
}

} // namespace Epp::Contact
} // namespace Epp
