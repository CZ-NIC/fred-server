#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/util/corba_conversions_int.h"
#include "util/corba_conversion.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"
#include "util/map_at.h"

#include <map>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Corba {

namespace {

bool
is_contact_change_string_meaning_to_delete(const char *value)
{
    return value[0] == '\b';
}

bool
is_contact_change_string_meaning_not_to_touch(const char *value)
{
    return value[0] == '\0';
}

boost::optional<Nullable<std::string> >
convert_contact_update_or_delete_string_and_trim(const char* src)
{
    const bool src_has_special_meaning_to_delete = is_contact_change_string_meaning_to_delete(src);
    if (src_has_special_meaning_to_delete) {
        return Nullable< std::string >();
    }
    const bool src_has_special_meaning_not_to_touch = is_contact_change_string_meaning_not_to_touch(src);
    if (src_has_special_meaning_not_to_touch) {
        return boost::optional< Nullable< std::string > >();
    }
    const std::string value_to_set = boost::trim_copy(Corba::unwrap_string(src));
    const bool value_to_set_means_not_to_touch = value_to_set.empty();
    if (value_to_set_means_not_to_touch) {
        return boost::optional< Nullable< std::string > >();
    }
    return Nullable< std::string >(value_to_set);
}

boost::optional<std::string>
convert_contact_update_string_and_trim(const char* src)
{
    const bool src_has_special_meaning_not_to_touch = is_contact_change_string_meaning_not_to_touch(src);
    if (src_has_special_meaning_not_to_touch) {
        return boost::optional< std::string >();
    }
    const std::string value_to_set = boost::trim_copy(Corba::unwrap_string(src));
    const bool value_to_set_means_not_to_touch = value_to_set.empty();
    return value_to_set_means_not_to_touch ? boost::optional< std::string >()
                                           : value_to_set;
}

Epp::Contact::ContactDisclose
convert_ContactChange_to_ContactDisclose(
        const ccReg::ContactChange& src,
        Epp::Contact::ContactDisclose::Flag::Enum meaning)
{
    Epp::Contact::ContactDisclose result(meaning);
    if (wrap_int< bool >(src.DiscloseName)) {
        result.add< Epp::Contact::ContactDisclose::Item::name >();
    }
    if (wrap_int< bool >(src.DiscloseOrganization)) {
        result.add< Epp::Contact::ContactDisclose::Item::organization >();
    }
    if (wrap_int< bool >(src.DiscloseAddress)) {
        result.add< Epp::Contact::ContactDisclose::Item::address >();
    }
    if (wrap_int< bool >(src.DiscloseTelephone)) {
        result.add< Epp::Contact::ContactDisclose::Item::telephone >();
    }
    if (wrap_int< bool >(src.DiscloseFax)) {
        result.add< Epp::Contact::ContactDisclose::Item::fax >();
    }
    if (wrap_int< bool >(src.DiscloseEmail)) {
        result.add< Epp::Contact::ContactDisclose::Item::email >();
    }
    if (wrap_int< bool >(src.DiscloseVAT)) {
        result.add< Epp::Contact::ContactDisclose::Item::vat >();
    }
    if (wrap_int< bool >(src.DiscloseIdent)) {
        result.add< Epp::Contact::ContactDisclose::Item::ident >();
    }
    if (wrap_int< bool >(src.DiscloseNotifyEmail)) {
        result.add< Epp::Contact::ContactDisclose::Item::notify_email >();
    }
    return result;
}

boost::optional<Epp::Contact::ContactDisclose>
unwrap_ContactChange_to_ContactDisclose(const ccReg::ContactChange& src)
{
    switch (src.DiscloseFlag)
    {
        case ccReg::DISCL_EMPTY:
            return boost::optional< Epp::Contact::ContactDisclose >();
        case ccReg::DISCL_HIDE:
            return convert_ContactChange_to_ContactDisclose(src, Epp::Contact::ContactDisclose::Flag::hide);
        case ccReg::DISCL_DISPLAY:
            return convert_ContactChange_to_ContactDisclose(src, Epp::Contact::ContactDisclose::Flag::disclose);
    }
    throw std::runtime_error("Invalid DiscloseFlag value;");
}

Nullable<Epp::Contact::ContactChange::IdentType::Enum>
unwrap_identtyp(ccReg::identtyp type)
{
    switch (type)
    {
        case ccReg::EMPTY:    return Nullable< Epp::Contact::ContactChange::IdentType::Enum >();
        case ccReg::OP:       return Epp::Contact::ContactChange::IdentType::op;
        case ccReg::PASS:     return Epp::Contact::ContactChange::IdentType::pass;
        case ccReg::ICO:      return Epp::Contact::ContactChange::IdentType::ico;
        case ccReg::MPSV:     return Epp::Contact::ContactChange::IdentType::mpsv;
        case ccReg::BIRTHDAY: return Epp::Contact::ContactChange::IdentType::birthday;
    }
    throw std::runtime_error("Invalid identtyp value.");
}

}//namespace Corba::{anonymous}

void
unwrap_ContactChange(
        const ccReg::ContactChange& src,
        Epp::Contact::ContactChange& dst)
{
    dst.name              = convert_contact_update_or_delete_string_and_trim(src.Name);
    dst.organization      = convert_contact_update_or_delete_string_and_trim(src.Organization);
    for (unsigned idx = 0; idx < src.Streets.length(); ++idx) {
        dst.streets.push_back(convert_contact_update_or_delete_string_and_trim(src.Streets[idx]));
    }
    dst.city              = convert_contact_update_or_delete_string_and_trim(src.City);
    dst.state_or_province = convert_contact_update_or_delete_string_and_trim(src.StateOrProvince);
    dst.postal_code       = convert_contact_update_or_delete_string_and_trim(src.PostalCode);
    dst.country_code      = convert_contact_update_string_and_trim(src.CC);
    dst.telephone         = convert_contact_update_or_delete_string_and_trim(src.Telephone);
    dst.fax               = convert_contact_update_or_delete_string_and_trim(src.Fax);
    dst.email             = convert_contact_update_or_delete_string_and_trim(src.Email);
    dst.notify_email      = convert_contact_update_or_delete_string_and_trim(src.NotifyEmail);
    dst.vat               = convert_contact_update_or_delete_string_and_trim(src.VAT);
    dst.ident             = convert_contact_update_or_delete_string_and_trim(src.ident);
    dst.ident_type        = unwrap_identtyp(src.identtype);
    dst.authinfopw        = convert_contact_update_or_delete_string_and_trim(src.AuthInfoPw);
    dst.disclose          = unwrap_ContactChange_to_ContactDisclose(src);
}

namespace {

static ccReg::CheckAvail wrap_contact_handle_check_result(const boost::optional< Epp::Contact::ContactHandleRegistrationObstructionLocalized >& _obstruction) {
    if (!_obstruction.is_initialized()) {
        return ccReg::NotExist;
    }

    switch (_obstruction.get().state)
    {
        case Epp::Contact::ContactHandleRegistrationObstruction::invalid_handle      : return ccReg::BadFormat;
        case Epp::Contact::ContactHandleRegistrationObstruction::protected_handle    : return ccReg::DelPeriod; // XXX oh my
        case Epp::Contact::ContactHandleRegistrationObstruction::registered_handle   : return ccReg::Exist;
    }

    throw std::runtime_error("unknown_contact_state");
}

ccReg::identtyp wrap_identtyp(const Nullable< Epp::Contact::InfoContactLocalizedOutputData::IdentType::Enum >& type)
{
    if (type.isnull()) {
        return ccReg::EMPTY;
    }
    switch (type.get_value())
    {
        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::op:       return ccReg::OP;
        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::pass:     return ccReg::PASS;
        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::ico:      return ccReg::ICO;
        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::mpsv:     return ccReg::MPSV;
        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::birthday: return ccReg::BIRTHDAY;
    }
    throw std::runtime_error("Invalid Epp::Contact::InfoContactLocalizedOutputData::IdentType::Enum value.");
}

} // namespace {anonymous}

/**
 * @returns check results in the same order as input handles
 */
ccReg::CheckResp
wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map<std::string, boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized> >& contact_handle_check_results)
{
    ccReg::CheckResp result;
    result.length(contact_handles.size());

    CORBA::ULong i = 0;
    for(
        std::vector<std::string>::const_iterator it = contact_handles.begin();
        it != contact_handles.end();
        ++it, ++i
    ) {
        const boost::optional< Epp::Contact::ContactHandleRegistrationObstructionLocalized > check_result = map_at(contact_handle_check_results, *it);

        result[i].avail = wrap_contact_handle_check_result(check_result);
        result[i].reason = wrap_string_to_corba_string(check_result.is_initialized() ? check_result.get().description : "");
    }

    return result;
}

namespace {

template < Epp::Contact::ContactDisclose::Item::Enum ITEM >
CORBA::Boolean presents(const Epp::Contact::ContactDisclose& src)
{
    return wrap_int< CORBA::Boolean >(src.presents< ITEM >());
}

}//namespace Corba::{anonymous}

void
wrap_InfoContactLocalizedOutputData(
        const Epp::Contact::InfoContactLocalizedOutputData& src,
        ccReg::Contact& dst)
{
    dst.handle = wrap_string_to_corba_string(src.handle);
    dst.ROID = wrap_string_to_corba_string(src.roid);
    dst.ClID = wrap_string_to_corba_string(src.sponsoring_registrar_handle);
    dst.CrID = wrap_string_to_corba_string(src.creating_registrar_handle);
    // XXX IDL nonsense
    dst.UpID = wrap_Nullable_string_to_string(src.last_update_registrar_handle);

    dst.stat.length(src.localized_external_states.size());
    unsigned long idx = 0;
    for (std::map< std::string, std::string >::const_iterator value_text_ptr = src.localized_external_states.begin();
        value_text_ptr != src.localized_external_states.end(); ++value_text_ptr, ++idx)
    {
        dst.stat[idx].value = wrap_string_to_corba_string(value_text_ptr->first);
        dst.stat[idx].text  = wrap_string_to_corba_string(value_text_ptr->second);
    }

    dst.CrDate = wrap_boost_posix_time_ptime_to_string(src.crdate);
    // XXX IDL nonsense
    dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(src.last_update);
    // XXX IDL nonsense
    dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(src.last_transfer);
    dst.Name = wrap_Nullable_string_to_string(src.name);
    dst.Organization = wrap_Nullable_string_to_string(src.organization);

    const unsigned number_of_streets =
        !src.street3.isnull() && !src.street3.get_value().empty()
            ? 3
            : !src.street2.isnull() && !src.street2.get_value().empty()
                ? 2
                : !src.street1.isnull() && !src.street1.get_value().empty()
                    ? 1
                    : 0;
    dst.Streets.length(number_of_streets);
    if (0 < number_of_streets) {
        dst.Streets[0] = wrap_Nullable_string_to_string(src.street1);
    }
    if (1 < number_of_streets) {
        dst.Streets[1] = wrap_Nullable_string_to_string(src.street2);
    }
    if (2 < number_of_streets) {
        dst.Streets[2] = wrap_Nullable_string_to_string(src.street3);
    }

    dst.City = wrap_Nullable_string_to_string(src.city);
    dst.StateOrProvince = wrap_Nullable_string_to_string(src.state_or_province);
    dst.PostalCode = wrap_Nullable_string_to_string(src.postal_code);
    dst.CountryCode = wrap_Nullable_string_to_string(src.country_code);
    dst.Telephone = wrap_Nullable_string_to_string(src.telephone);
    dst.Fax = wrap_Nullable_string_to_string(src.fax);
    dst.Email = wrap_Nullable_string_to_string(src.email);
    dst.NotifyEmail = wrap_Nullable_string_to_string(src.notify_email);
    dst.VAT = wrap_Nullable_string_to_string(src.VAT);
    dst.ident = wrap_Nullable_string_to_string(src.ident);
    dst.identtype = wrap_identtyp(src.identtype);
    dst.AuthInfoPw = Corba::wrap_string_to_corba_string(src.authinfopw ? src.authinfopw.value() : std::string());

    if (!src.disclose.is_initialized()) {
        dst.DiscloseFlag           = ccReg::DISCL_EMPTY;
        dst.DiscloseName           = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseOrganization   = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseAddress        = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseTelephone      = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseFax            = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseEmail          = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseVAT            = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseIdent          = wrap_int< CORBA::Boolean >(false);
        dst.DiscloseNotifyEmail    = wrap_int< CORBA::Boolean >(false);
    }
    else {
        dst.DiscloseFlag         = src.disclose->does_present_item_mean_to_disclose() ? ccReg::DISCL_DISPLAY
                                                                                      : ccReg::DISCL_HIDE;
        dst.DiscloseName         = presents< Epp::Contact::ContactDisclose::Item::name         >(*src.disclose);
        dst.DiscloseOrganization = presents< Epp::Contact::ContactDisclose::Item::organization >(*src.disclose);
        dst.DiscloseAddress      = presents< Epp::Contact::ContactDisclose::Item::address      >(*src.disclose);
        dst.DiscloseTelephone    = presents< Epp::Contact::ContactDisclose::Item::telephone    >(*src.disclose);
        dst.DiscloseFax          = presents< Epp::Contact::ContactDisclose::Item::fax          >(*src.disclose);
        dst.DiscloseEmail        = presents< Epp::Contact::ContactDisclose::Item::email        >(*src.disclose);
        dst.DiscloseVAT          = presents< Epp::Contact::ContactDisclose::Item::vat          >(*src.disclose);
        dst.DiscloseIdent        = presents< Epp::Contact::ContactDisclose::Item::ident        >(*src.disclose);
        dst.DiscloseNotifyEmail  = presents< Epp::Contact::ContactDisclose::Item::notify_email >(*src.disclose);
    }
}

} // namespace Corba
