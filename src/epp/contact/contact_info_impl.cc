#include "src/epp/contact/contact_info_impl.h"

#include <fredlib/contact.h>
#include <fredlib/registrar.h>
#include "src/fredlib/object_state/get_object_states.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"

#include <boost/foreach.hpp>

namespace Epp {

static std::set<std::string> convert_object_states(const std::vector<Fred::ObjectStateData>& _object_states) {
    std::set<std::string> result;

    BOOST_FOREACH(const Fred::ObjectStateData& state, _object_states) {
        result.insert(state.state_name);
    }

    return result;
}

ContactInfoOutputData contact_info_impl(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    const SessionLang::Enum _object_state_description_lang,
    const unsigned long long _session_registrar_id
) {
    if( _session_registrar_id == 0 ) {
        throw AuthErrorServerClosingConnection();
    }

    if( Fred::Contact::is_handle_valid(_handle) != Fred::ContactHandleState::SyntaxValidity::valid ) {
        throw InvalidHandle();
    }

    try {
        const Fred::InfoContactData contact_info_data = Fred::InfoContactByHandle(_handle).exec(_ctx, "UTC").info_contact_data;

        return ContactInfoOutputData(
            contact_info_data.handle,
            contact_info_data.roid,
            contact_info_data.sponsoring_registrar_handle,
            contact_info_data.create_registrar_handle,
            contact_info_data.update_registrar_handle,
            convert_object_states( Fred::GetObjectStates(contact_info_data.id).exec(_ctx) ),
            contact_info_data.creation_time,
            contact_info_data.update_time,
            contact_info_data.transfer_time,
            contact_info_data.name,
            contact_info_data.organization,
            contact_info_data.place.isnull()
                ? Nullable<std::string>()
                : contact_info_data.place.get_value().street1,
            contact_info_data.place.isnull()
                ? Nullable<std::string>()
                : ! contact_info_data.place.get_value().street2.isset()
                    ? Nullable<std::string>()
                    : contact_info_data.place.get_value().street2.get_value(),
            contact_info_data.place.isnull()
                ? Nullable<std::string>()
                : ! contact_info_data.place.get_value().street3.isset()
                    ? Nullable<std::string>()
                    : contact_info_data.place.get_value().street3.get_value(),
            contact_info_data.place.isnull()
                ? Nullable<std::string>()
                : contact_info_data.place.get_value().city,
            contact_info_data.place.isnull()
                ? Nullable<std::string>()
                : contact_info_data.place.get_value().stateorprovince.isset()
                    ? contact_info_data.place.get_value().stateorprovince.get_value()
                    : Nullable<std::string>(),
            contact_info_data.place.isnull()
                ? Nullable<std::string>()
                : contact_info_data.place.get_value().postalcode,
            contact_info_data.place.isnull()
                ? Nullable<std::string>()
                : contact_info_data.place.get_value().country,
            contact_info_data.telephone,
            contact_info_data.fax,
            contact_info_data.email,
            contact_info_data.notifyemail,
            contact_info_data.vat,
            contact_info_data.ssn,
            contact_info_data.ssntype.isnull() ? Nullable<IdentType::Enum>() : from_db_handle<IdentType>(contact_info_data.ssntype.get_value()),
            contact_info_data.authinfopw,
            contact_info_data.disclosename,
            contact_info_data.discloseorganization,
            contact_info_data.discloseaddress,
            contact_info_data.disclosetelephone,
            contact_info_data.disclosefax,
            contact_info_data.discloseemail,
            contact_info_data.disclosevat,
            contact_info_data.discloseident,
            contact_info_data.disclosenotifyemail
        );

    } catch (const Fred::InfoContactByHandle::Exception& e) {

        if(e.is_set_unknown_contact_handle()) {
            throw NonexistentHandle();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
