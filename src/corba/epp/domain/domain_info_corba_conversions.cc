#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/domain/domain_info_corba_conversions.h"
#include "src/corba/EPP.hh"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/domain_info.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <string>

namespace Corba {

namespace {

// represents RFC3339 time offset
// append to boost::posix_time::to_iso_extended_string() to get RFC3339 timestamp
struct TimeZoneOffset {
    boost::optional<boost::posix_time::time_duration> time_zone_offset_;

    TimeZoneOffset(const boost::posix_time::ptime& _utc_time) {
        time_zone_offset_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(_utc_time) - _utc_time;
    }

    std::string to_rfc3339_string() {
        if(time_zone_offset_) {
            if(time_zone_offset_->hours() || time_zone_offset_->minutes()) {
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
static std::string format_time_to_rfc3339(const boost::posix_time::ptime& _utc_ptime) {
    // _utc_ptime converted to local ptime with seconds fraction trimmed
    const boost::posix_time::ptime local_ptime =
        boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(
            boost::posix_time::ptime(_utc_ptime.date(), boost::posix_time::seconds(_utc_ptime.time_of_day().total_seconds())));
    return
        boost::posix_time::to_iso_extended_string(local_ptime) +
        TimeZoneOffset(local_ptime).to_rfc3339_string();
}

static std::string format_date_to_iso_extended(const boost::gregorian::date& d) {
    return boost::gregorian::to_iso_extended_string(d);
}

} // namespace Corba::{anonymous}

void wrap_Epp_ENUMValidationExtension_to_ccReg_ExtensionList(Epp::ENUMValidationExtension _src, ccReg::ExtensionList& _dst) {
    ccReg::ENUMValidationExtension_var enumVal = new ccReg::ENUMValidationExtension();
    enumVal->valExDate = Corba::wrap_string_to_corba_string(
        boost::gregorian::to_iso_extended_string(_src.get_valexdate())
    );
    enumVal->publish = _src.get_publish() ? ccReg::DISCL_DISPLAY : ccReg::DISCL_HIDE;
    _dst.length(1);
    _dst[0] <<= enumVal._retn();
}

void wrap_Epp_Domain_DomainInfoLocalizedOutputData(
    const Epp::Domain::DomainInfoLocalizedOutputData& _src,
    ccReg::Domain& _dst
) {

    _dst.ROID = Corba::wrap_string_to_corba_string(_src.roid);
    _dst.name = Corba::wrap_string_to_corba_string(_src.fqdn);

    _dst.Registrant = Corba::wrap_string_to_corba_string(_src.registrant);
    _dst.nsset = Corba::wrap_string_to_corba_string(_src.nsset.get_value_or(std::string()));
    _dst.keyset = Corba::wrap_string_to_corba_string(_src.keyset.get_value_or(std::string()));

    Corba::wrap_Epp_LocalizedStates(_src.localized_external_states, _dst.stat);

    _dst.ClID = Corba::wrap_string_to_corba_string(_src.sponsoring_registrar_handle);
    _dst.CrID = Corba::wrap_string_to_corba_string(_src.creating_registrar_handle);
    _dst.UpID = Corba::wrap_string_to_corba_string(_src.last_update_registrar_handle.get_value_or(std::string()));

    _dst.CrDate = Corba::wrap_string_to_corba_string(format_time_to_rfc3339(_src.crdate));
    _dst.UpDate = Corba::wrap_string_to_corba_string(
        _src.last_update.isnull() ? std::string()
                                 : format_time_to_rfc3339(_src.last_update.get_value()));
    _dst.TrDate = Corba::wrap_string_to_corba_string(
        _src.last_transfer.isnull() ? std::string()
                                   : format_time_to_rfc3339(_src.last_transfer.get_value()));

    _dst.ExDate = Corba::wrap_string_to_corba_string(format_date_to_iso_extended(_src.exdate));

    _dst.AuthInfoPw = Corba::wrap_string_to_corba_string(_src.auth_info_pw.get_value_or_default());

    _dst.admin.length(_src.admin.size());
    unsigned long dst_admin_index = 0;
    for(std::set<std::string>::const_iterator admin_ptr = _src.admin.begin(); admin_ptr != _src.admin.end(); ++admin_ptr) {
        _dst.admin[dst_admin_index] = Corba::wrap_string_to_corba_string(*admin_ptr);
        ++dst_admin_index;
    }

    Corba::wrap_Epp_ENUMValidationExtension_to_ccReg_ExtensionList(
        _src.ext_enum_domain_validation.get_value_or(Epp::ENUMValidationExtension()),
        _dst.ext
    );

    _dst.tmpcontact.length(_src.tmpcontact.size());
    unsigned long dst_tmpcontact_index = 0;
    for(std::set<std::string>::const_iterator tmpcontact_ptr = _src.tmpcontact.begin(); tmpcontact_ptr != _src.tmpcontact.end(); ++tmpcontact_ptr) {
        _dst.tmpcontact[dst_tmpcontact_index] = Corba::wrap_string_to_corba_string(*tmpcontact_ptr);
        ++dst_tmpcontact_index;
    }

    return;
}

}
