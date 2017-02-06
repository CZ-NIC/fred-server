#include "src/corba/epp/domain/domain_corba_conversions.h"

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/domain/check_domain_localized.h"
#include "src/epp/domain/impl/domain_registration_obstruction.h"
#include "util/db/nullable.h"
#include "util/map_at.h"
#include "util/optional_value.h"

#include <boost/optional.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <map>
#include <set>
#include <vector>
#include <string>

namespace Corba {

Optional<Nullable<std::string> >
unwrap_string_for_change_or_remove_to_Optional_Nullable_string(const char* _src)
{
    const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

    /* Defined by convention. Could be substituted by more explicit means in IDL interface
     * (like using _add and _rem elements, not just _chg for all operations). */
    static const char char_for_value_deleting = '\b';

    return
        unwrapped_src.empty()
        ? Optional<Nullable<std::string> >() // do not set
        : unwrapped_src.at(0) == char_for_value_deleting
            ? Optional<Nullable<std::string> >(Nullable<std::string>()) // set NULL
            : boost::trim_copy(unwrapped_src);
}

Optional<Nullable<std::string> >
unwrap_string_for_change_or_remove_to_Optional_Nullable_string_no_trim(const char* _src)
{
    const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

    /* Defined by convention. Could be substituted by more explicit means in IDL interface
     * (like using _add and _rem elements, not just _chg for all operations). */
    static const char char_for_value_deleting = '\b';

    return
        unwrapped_src.empty()
        ? Optional<Nullable<std::string> >() // do not set
        : unwrapped_src.at(0) == char_for_value_deleting
            ? Optional<Nullable<std::string> >(Nullable<std::string>()) // set NULL
            : unwrapped_src;
}

namespace {

ccReg::CheckAvail
wrap_Epp_Domain_DomainLocalizedRegistrationObstruction(
        const boost::optional<Epp::Domain::DomainLocalizedRegistrationObstruction>& obstruction)
{

    if (!obstruction) {
        return ccReg::NotExist;
    }

    switch (obstruction.get().state) {
        case Epp::Domain::DomainRegistrationObstruction::registered           : return ccReg::Exist;
        case Epp::Domain::DomainRegistrationObstruction::blacklisted          : return ccReg::BlackList;
        case Epp::Domain::DomainRegistrationObstruction::zone_not_in_registry : return ccReg::NotApplicable;
        case Epp::Domain::DomainRegistrationObstruction::invalid_fqdn         : return ccReg::BadFormat;
    }

    throw std::logic_error("Unexpected Epp::Domain::DomainRegistrationObstruction::Enum value.");
}

} // namespace Corba::{anonymous}

/**
 * @returns check results in the same order as input handles
 */
ccReg::CheckResp
wrap_Epp_Domain_CheckDomainLocalizedResponse(
        const std::vector<std::string>& _domain_fqdns,
        const std::map<std::string, boost::optional<Epp::Domain::DomainLocalizedRegistrationObstruction> >& _domain_fqdn_to_domain_localized_registration_obstruction)
{
    ccReg::CheckResp result;
    result.length(_domain_fqdns.size());

    CORBA::ULong result_idx = 0;
    for (std::vector<std::string>::const_iterator domain_fqdn_ptr = _domain_fqdns.begin();
         domain_fqdn_ptr != _domain_fqdns.end();
         ++domain_fqdn_ptr, ++result_idx)
    {
        const boost::optional<Epp::Domain::DomainLocalizedRegistrationObstruction> domain_localized_registration_obstruction =
            map_at(_domain_fqdn_to_domain_localized_registration_obstruction, *domain_fqdn_ptr);

        result[result_idx].avail =
            wrap_Epp_Domain_DomainLocalizedRegistrationObstruction(domain_localized_registration_obstruction);

        result[result_idx].reason =
            Corba::wrap_string_to_corba_string(
                domain_localized_registration_obstruction ? domain_localized_registration_obstruction.get().description : ""
            );
    }

    return result;
}

namespace {

// represents RFC3339 time offset
// append to boost::posix_time::to_iso_extended_string() to get RFC3339 timestamp
struct TimeZoneOffset
{
    boost::optional<boost::posix_time::time_duration> time_zone_offset_;

    explicit TimeZoneOffset(const boost::posix_time::ptime& _utc_time)
    {
        time_zone_offset_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(_utc_time) - _utc_time;
    }

    std::string to_rfc3339_string()
    {
        if (time_zone_offset_) {
            if (time_zone_offset_->hours() || time_zone_offset_->minutes()) {
                return boost::str(boost::format("%1$+03d:%2$02d")
                    % time_zone_offset_->hours()
                    % boost::date_time::absolute_value(time_zone_offset_->minutes()));
            }
            else {
                return std::string("Z");
            }
        }
        else {
            return std::string("-00:00");
        }
    }
};

/**
 * Converts and formats time
 *
 * @param _utc_ptime time in UTC
 *
 * @return time converted to local time zone, with seconds fraction trimmed, formatted as RFC3339 string
 */
std::string
format_time_to_rfc3339(const boost::posix_time::ptime& _utc_ptime)
{
    // _utc_ptime converted to local ptime with seconds fraction trimmed
    const boost::posix_time::ptime local_ptime =
        boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(
            boost::posix_time::ptime(_utc_ptime.date(), boost::posix_time::seconds(_utc_ptime.time_of_day().total_seconds())));
    return
        boost::posix_time::to_iso_extended_string(local_ptime) +
        TimeZoneOffset(local_ptime).to_rfc3339_string();
}

void
wrap_Epp_Domain_EnumValidationExtension_to_ccReg_ExtensionList(
        const Nullable<Epp::Domain::EnumValidationExtension>& _src,
        ccReg::ExtensionList& _dst)
{
    if (!_src.isnull()) {
        ccReg::ENUMValidationExtension_var enumVal = new ccReg::ENUMValidationExtension();
        enumVal->valExDate = Corba::wrap_string_to_corba_string(
            boost::gregorian::to_iso_extended_string(_src.get_value().get_valexdate())
        );
        enumVal->publish = _src.get_value().get_publish() ? ccReg::DISCL_DISPLAY : ccReg::DISCL_HIDE;
        _dst.length(1);
        _dst[0] <<= enumVal._retn();
    }
}

} // namespace Corba::{anonymous}

void
wrap_Epp_Domain_InfoDomainLocalizedOutputData(
        const Epp::Domain::InfoDomainLocalizedOutputData& _src,
        ccReg::Domain& _dst)
{

    _dst.ROID = Corba::wrap_string_to_corba_string(_src.roid);
    _dst.name = Corba::wrap_string_to_corba_string(_src.fqdn);

    _dst.Registrant = Corba::wrap_string_to_corba_string(_src.registrant);
    _dst.nsset = Corba::wrap_string_to_corba_string(_src.nsset.get_value_or_default());
    _dst.keyset = Corba::wrap_string_to_corba_string(_src.keyset.get_value_or_default());

    Corba::wrap_Epp_ObjectStatesLocalized(_src.localized_external_states, _dst.stat);

    _dst.ClID = Corba::wrap_string_to_corba_string(_src.sponsoring_registrar_handle);
    _dst.CrID = Corba::wrap_string_to_corba_string(_src.creating_registrar_handle);
    _dst.UpID = Corba::wrap_string_to_corba_string(_src.last_update_registrar_handle.get_value_or_default());

    _dst.CrDate = Corba::wrap_string_to_corba_string(format_time_to_rfc3339(_src.crdate));
    _dst.UpDate = Corba::wrap_string_to_corba_string(
        _src.last_update.isnull() ? std::string()
                                 : format_time_to_rfc3339(_src.last_update.get_value()));
    _dst.TrDate = Corba::wrap_string_to_corba_string(
        _src.last_transfer.isnull() ? std::string()
                                   : format_time_to_rfc3339(_src.last_transfer.get_value()));

    _dst.ExDate = Corba::wrap_string_to_corba_string(boost::gregorian::to_iso_extended_string(_src.exdate));

    _dst.AuthInfoPw = Corba::wrap_string_to_corba_string(_src.authinfopw ? _src.authinfopw.value() : std::string());

    _dst.admin.length(_src.admin.size());
    unsigned long dst_admin_index = 0;
    for(std::set<std::string>::const_iterator admin_ptr = _src.admin.begin(); admin_ptr != _src.admin.end(); ++admin_ptr) {
        _dst.admin[dst_admin_index] = Corba::wrap_string_to_corba_string(*admin_ptr);
        ++dst_admin_index;
    }

    Corba::wrap_Epp_Domain_EnumValidationExtension_to_ccReg_ExtensionList(
        _src.ext_enum_domain_validation,
        _dst.ext
    );

    _dst.tmpcontact.length(_src.tmpcontact.size());
    unsigned long dst_tmpcontact_index = 0;
    for(std::set<std::string>::const_iterator tmpcontact_ptr = _src.tmpcontact.begin(); tmpcontact_ptr != _src.tmpcontact.end(); ++tmpcontact_ptr) {
        _dst.tmpcontact[dst_tmpcontact_index] = Corba::wrap_string_to_corba_string(*tmpcontact_ptr);
        ++dst_tmpcontact_index;
    }

}

Epp::Domain::DomainRegistrationTime
unwrap_domain_registration_period(const ccReg::Period_str& period)
{
    switch(period.unit)
    {
        case ccReg::unit_month:
            return Epp::Domain::DomainRegistrationTime(period.count, Epp::Domain::DomainRegistrationTime::Unit::month);
        case ccReg::unit_year:
            return Epp::Domain::DomainRegistrationTime(period.count, Epp::Domain::DomainRegistrationTime::Unit::year);
    };
    throw std::runtime_error("unwrap_domain_registration_period internal error");
}

std::vector<std::string>
unwrap_ccreg_admincontacts_to_vector_string(const ccReg::AdminContact& in)
{
    std::vector<std::string> ret;
    ret.reserve(in.length());
    for(unsigned long long i = 0; i < in.length(); ++i)
    {
        if(in[i] == 0)
        {
            throw std::runtime_error("null char ptr");
        }
        ret.push_back(std::string(in[i]));
    }
    return ret;
}

std::vector<Epp::Domain::EnumValidationExtension>
unwrap_enum_validation_extension_list(const ccReg::ExtensionList& ext)
{
    const ccReg::ENUMValidationExtension *enum_ext = 0;
    std::vector<Epp::Domain::EnumValidationExtension> ret;
    ret.reserve(ext.length());
    for(unsigned i = 0; i < ext.length(); ++i)
    {
        if (ext[i] >>= enum_ext)
        {
            const std::string temp_valexdate_string = Corba::unwrap_string_from_const_char_ptr(enum_ext->valExDate);
            boost::gregorian::date valexdate;
            try {
                valexdate = boost::gregorian::from_simple_string(temp_valexdate_string);
            }
            catch(...)
            {
                // conversion errors ignored here, implementation must later handle `not-a-date-time` value correctly
            }

            ret.push_back(Epp::Domain::EnumValidationExtension(valexdate,
                enum_ext->publish == ccReg::DISCL_DISPLAY)
            );
        }
        else
        {
            throw std::runtime_error("unknown extension found when extracting domain enum extension");
        }
    }
    return ret;
}

} // namespace Corba
