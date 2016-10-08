#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/domain/domain_info_corba_conversions.h"
#include "src/corba/EPP.hh"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/domain/domain_info.h"
#include "src/old_utils/util.h" // for convert_rfc3339_timestamp()

#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace CorbaConversion {

namespace {

// FIXME
static std::string formatTime(const boost::posix_time::ptime& tm) {
    char buffer[100];
    convert_rfc3339_timestamp(buffer, sizeof(buffer), boost::posix_time::to_iso_extended_string(tm).c_str());
    return buffer;
}
// FIXME
static std::string formatDate(const boost::gregorian::date tm) {
    char buffer[100];
    convert_rfc3339_date(buffer, sizeof(buffer), boost::gregorian::to_iso_extended_string(tm).c_str());
    return buffer;
}

// TODO wrap_string_vector_to_string_sequence

} // namespace CorbaConversion::{anonymous}


void wrap_Fred_ENUMValidationExtension_to_ccReg_ExtensionList(Fred::ENUMValidationExtension src, ccReg::ExtensionList& dst) {
    return; // TODO
}

void wrap_Epp_Domain_DomainInfoLocalizedOutputData(
    const Epp::Domain::DomainInfoLocalizedOutputData& src,
    ccReg::Domain& dst
) {

    dst.ROID = Corba::wrap_string_to_corba_string(src.roid);
    dst.name = Corba::wrap_string_to_corba_string(src.fqdn);

    dst.Registrant = Corba::wrap_string_to_corba_string(src.registrant);
    dst.nsset = Corba::wrap_string_to_corba_string(src.nsset.get_value_or(std::string()));
    dst.keyset = Corba::wrap_string_to_corba_string(src.keyset.get_value_or(std::string()));

    Corba::wrap_Epp_LocalizedStates(src.localized_external_states, dst.stat);

    dst.ClID = Corba::wrap_string_to_corba_string(src.sponsoring_registrar_handle);
    dst.CrID = Corba::wrap_string_to_corba_string(src.creating_registrar_handle);
    dst.UpID = Corba::wrap_string_to_corba_string(src.last_update_registrar_handle.get_value_or(std::string()));

    dst.CrDate = Corba::wrap_string_to_corba_string(formatTime(src.crdate));
    dst.UpDate = Corba::wrap_string_to_corba_string(
        src.last_update.isnull() ? std::string()
                                 : formatTime(src.last_update.get_value()));
    dst.TrDate = Corba::wrap_string_to_corba_string(
        src.last_transfer.isnull() ? std::string()
                                   : formatTime(src.last_transfer.get_value()));

    dst.ExDate = Corba::wrap_string_to_corba_string(formatDate(src.exdate));

    dst.AuthInfoPw = Corba::wrap_string_to_corba_string(src.auth_info_pw.get_value_or_default());

    {
        //dst.admin = new ccReg::AdminContact();
        dst.admin.length(src.admin.size());
        unsigned long dst_index = 0;
        for(std::set<std::string>::const_iterator admin_ptr = src.admin.begin(); admin_ptr != src.admin.end(); ++admin_ptr) {
            dst.admin[dst_index] = CorbaConversion::wrap_string(*admin_ptr);
            ++dst_index;
        }
    }

    CorbaConversion::wrap_Fred_ENUMValidationExtension_to_ccReg_ExtensionList(
            src.ext_enum_domain_validation.get_value_or(Fred::ENUMValidationExtension()),
            dst.ext);

    {
        //dst.tmpcontact = new ccReg::AdminContact();
        dst.tmpcontact.length(src.tmpcontact.size());
        unsigned long dst_index = 0;
        for(std::set<std::string>::const_iterator tmpcontact_ptr = src.tmpcontact.begin(); tmpcontact_ptr != src.tmpcontact.end(); ++tmpcontact_ptr) {
            dst.tmpcontact[dst_index] = CorbaConversion::wrap_string(*tmpcontact_ptr);
            ++dst_index;
        }
    }

    return;
}

}
