#include "src/epp/disclose_policy.h"
#include "src/epp/contact/contact_info_impl.h"

#include "src/fredlib/object_state/get_object_states.h"
#include "src/epp/exception.h"
#include "src/epp/impl/util.h"

#include <fredlib/contact.h>
#include <fredlib/registrar.h>

#include <boost/foreach.hpp>

namespace Epp {

namespace {

std::set< ContactInfoOutputData::State > convert_object_states(
    const std::vector< Fred::ObjectStateData > &_object_states)
{
    std::set< ContactInfoOutputData::State > result;

    for (std::vector< Fred::ObjectStateData >::const_iterator state_ptr = _object_states.begin();
         state_ptr != _object_states.end(); ++state_ptr)
    {
        result.insert(ContactInfoOutputData::State(state_ptr->state_name, state_ptr->is_external));
    }

    return result;
}

void insert_discloseflags(const Fred::InfoContactData &src, std::set< ContactDisclose::Enum > &dst, bool value)
{
    if (src.disclosename == value) {
        dst.insert(ContactDisclose::name);
    }
    if (src.discloseorganization == value) {
        dst.insert(ContactDisclose::organization);
    }
    if (src.discloseaddress == value) {
        dst.insert(ContactDisclose::address);
    }
    if (src.disclosetelephone == value) {
        dst.insert(ContactDisclose::telephone);
    }
    if (src.disclosefax == value) {
        dst.insert(ContactDisclose::fax);
    }
    if (src.discloseemail == value) {
        dst.insert(ContactDisclose::email);
    }
    if (src.disclosevat == value) {
        dst.insert(ContactDisclose::vat);
    }
    if (src.discloseident == value) {
        dst.insert(ContactDisclose::ident);
    }
    if (src.disclosenotifyemail == value) {
        dst.insert(ContactDisclose::notify_email);
    }
}

void set_discloseflags(const Fred::InfoContactData &src, ContactInfoOutputData &dst)
{
    dst.to_hide.clear();
    dst.to_disclose.clear();
    if (is_the_default_policy_to_disclose()) {
        insert_discloseflags(src, dst.to_hide, false);
    }
    else {
        insert_discloseflags(src, dst.to_disclose, true);
    }
}

}//namespace Epp::{anonymous}

ContactInfoOutputData contact_info_impl(
    Fred::OperationContext &_ctx,
    const std::string &_handle,
    const SessionLang::Enum _object_state_description_lang,
    const unsigned long long _session_registrar_id)
{
    const bool registrar_is_authenticated = _session_registrar_id != 0;
    if (!registrar_is_authenticated) {
        throw AuthErrorServerClosingConnection();
    }

    try {
        const Fred::InfoContactData info = Fred::InfoContactByHandle(_handle).exec(_ctx, "UTC").info_contact_data;

        ContactInfoOutputData output_data;
        output_data.handle            = info.handle;
        output_data.roid              = info.roid;
        output_data.sponsoring_registrar_handle  = info.sponsoring_registrar_handle;
        output_data.creating_registrar_handle    = info.create_registrar_handle;
        output_data.last_update_registrar_handle = info.update_registrar_handle;
        output_data.states            = convert_object_states(Fred::GetObjectStates(info.id).exec(_ctx));
        output_data.crdate            = info.creation_time;
        output_data.last_update       = info.update_time;
        output_data.last_transfer     = info.transfer_time;
        output_data.name              = info.name;
        output_data.organization      = info.organization;
        output_data.street1           = info.place.isnull()
                ? Nullable< std::string >()
                : info.place.get_value().street1;
        output_data.street2           = info.place.isnull()
                ? Nullable< std::string >()
                : ! info.place.get_value().street2.isset()
                    ? Nullable< std::string >()
                    : info.place.get_value().street2.get_value();
        output_data.street3           = info.place.isnull()
                ? Nullable< std::string >()
                : ! info.place.get_value().street3.isset()
                    ? Nullable< std::string >()
                    : info.place.get_value().street3.get_value();
        output_data.city              = info.place.isnull()
                ? Nullable< std::string >()
                : info.place.get_value().city;
        output_data.state_or_province = info.place.isnull()
                ? Nullable< std::string >()
                : info.place.get_value().stateorprovince.isset()
                    ? info.place.get_value().stateorprovince.get_value()
                    : Nullable< std::string >();
        output_data.postal_code       = info.place.isnull()
                ? Nullable< std::string >()
                : info.place.get_value().postalcode;
        output_data.country_code      = info.place.isnull()
                ? Nullable< std::string >()
                : info.place.get_value().country;
        output_data.telephone         = info.telephone;
        output_data.fax               = info.fax;
        output_data.email             = info.email;
        output_data.notify_email      = info.notifyemail;
        output_data.VAT               = info.vat;
        output_data.ident             = info.ssn;
        output_data.identtype         = info.ssntype.isnull()
                                        ? Nullable< IdentType::Enum> ()
                                        : from_db_handle< IdentType >(info.ssntype.get_value());
        output_data.auth_info_pw      = info.authinfopw;
        set_discloseflags(info, output_data);
        return output_data;

    } catch (const Fred::InfoContactByHandle::Exception& e) {

        if(e.is_set_unknown_contact_handle()) {
            throw NonexistentHandle();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

}
