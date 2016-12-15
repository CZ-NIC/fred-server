#include "src/epp/impl/disclose_policy.h"
#include "src/epp/contact/info_contact.h"

#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/util.h"
#include "util/db/nullable.h"
#include "src/fredlib/contact.h"
#include "src/fredlib/registrar.h"

#include <boost/foreach.hpp>

#include <string>


namespace Epp {
namespace Contact {

namespace {

std::set< std::string > convert_object_states(const std::vector< Fred::ObjectStateData > &_object_states)
{
    std::set< std::string > result;

    for (std::vector< Fred::ObjectStateData >::const_iterator state_ptr = _object_states.begin();
         state_ptr != _object_states.end(); ++state_ptr)
    {
        const bool state_is_internal = !state_ptr->is_external;
        if (!state_is_internal) {
            result.insert(state_ptr->state_name);
        }
    }

    return result;
}

void insert_discloseflags(const Fred::InfoContactData &src, ContactDisclose &dst)
{
    const bool meaning_of_present_discloseflag = dst.does_present_item_mean_to_disclose();
    if (src.disclosename == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::name >();
    }
    if (src.discloseorganization == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::organization >();
    }
    if (src.discloseaddress == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::address >();
    }
    if (src.disclosetelephone == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::telephone >();
    }
    if (src.disclosefax == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::fax >();
    }
    if (src.discloseemail == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::email >();
    }
    if (src.disclosevat == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::vat >();
    }
    if (src.discloseident == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::ident >();
    }
    if (src.disclosenotifyemail == meaning_of_present_discloseflag) {
        dst.add< ContactDisclose::Item::notify_email >();
    }
}

boost::optional< ContactDisclose > get_discloseflags(const Fred::InfoContactData &src)
{
    ContactDisclose disclose(is_the_default_policy_to_disclose() ? ContactDisclose::Flag::hide
                                                                 : ContactDisclose::Flag::disclose);
    insert_discloseflags(src, disclose);
    const bool discloseflags_conform_to_the_default_policy = disclose.is_empty();
    return discloseflags_conform_to_the_default_policy ? boost::optional< ContactDisclose >()
                                                       : disclose;
}

boost::optional< Fred::PersonalIdUnion > get_personal_id(const Nullable< std::string > &_value,
                                                         const Nullable< std::string > &_type)
{
    if (_value.isnull() || _type.isnull()) {
        return boost::optional< Fred::PersonalIdUnion >();
    }
    const std::string value = _value.get_value();
    const std::string type = _type.get_value();
    Fred::PersonalIdUnion result = Fred::PersonalIdUnion::get_OP(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_PASS(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_ICO(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_MPSV(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_BIRTHDAY(value);
    if (result.get_type() == type) {
        return result;
    }
    result = Fred::PersonalIdUnion::get_RC(value);
    if (result.get_type() == type) {
        return boost::optional< Fred::PersonalIdUnion >();
    }
    throw std::runtime_error("Invalid ident type.");
}

}//namespace Epp::{anonymous}

InfoContactOutputData::InfoContactOutputData(const boost::optional< ContactDisclose > &_disclose)
:   disclose(_disclose)
{
}

InfoContactOutputData info_contact(
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

        InfoContactOutputData output_data(get_discloseflags(info));
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
        output_data.personal_id       = get_personal_id(info.ssn, info.ssntype);

        // show object authinfo only to sponsoring registrar
        const std::string callers_registrar_handle = Fred::InfoRegistrarById(_session_registrar_id).exec(_ctx).info_registrar_data.handle;
        const bool callers_is_sponsoring_registrar = info.sponsoring_registrar_handle == callers_registrar_handle;
        const bool authinfo_has_to_be_hidden = !callers_is_sponsoring_registrar;
        output_data.auth_info_pw      = authinfo_has_to_be_hidden ? boost::optional<std::string>() : info.authinfopw;

        return output_data;

    } catch (const Fred::InfoContactByHandle::Exception& e) {

        if(e.is_set_unknown_contact_handle()) {
            throw NonexistentHandle();
        }

        /* in the improbable case that exception is incorrectly set */
        throw;
    }
}

} // namespace Epp::Contact
} // namespace Epp
