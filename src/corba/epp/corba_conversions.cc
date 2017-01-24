#include "src/corba/epp/corba_conversions.h"

#include "src/corba/EPP.hh"
#include "src/corba/epp/epp_legacy_compatibility.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/create_contact_localized.h"
#include "src/epp/error.h"
#include "src/epp/impl/param.h"
#include "src/epp/keyset/check_keyset_localized.h"
#include "src/epp/keyset/info_keyset_localized.h"
#include "src/epp/nsset/info_nsset_localized.h"

#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/epp_result_failure_localized.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/impl/epp_extended_error_localized.h"

#include "util/corba_conversion.h"
#include "util/db/nullable.h"
#include "util/map_at.h"
#include "util/optional_value.h"

#include "src/old_utils/util.h" // for convert_rfc3339_timestamp() // FIXME replace with info_domain_corba_conversions.cc version

#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/integer_traits.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>

#include <string>
#include <set>
#include <vector>

namespace Corba {

CORBA::String_var
wrap_Nullable_string_to_string(const Nullable<std::string>& src)
{
    return src.isnull() ? "" : src.get_value().c_str();
}

CORBA::String_var
wrap_boost_posix_time_ptime_to_string(const boost::posix_time::ptime& _src)
{
    static const unsigned size_enough_for_string_representation_of_time = 100;//2016-04-18T13:00:00+02:00
    char time[size_enough_for_string_representation_of_time];
    const std::string iso_extended_time = boost::posix_time::to_iso_extended_string(_src);
    convert_rfc3339_timestamp(time, size_enough_for_string_representation_of_time, iso_extended_time.c_str()); // FIXME use boost variant
    return const_cast< const char* >(time);
}

CORBA::String_var
wrap_Nullable_boost_posix_time_ptime_to_string(const Nullable<boost::posix_time::ptime>& _src)
{
    return _src.isnull() ? "" : wrap_boost_posix_time_ptime_to_string(_src.get_value());
}

std::vector<std::string>
unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles)
{
    std::vector<std::string> result;

    result.reserve(handles.length());

    for(CORBA::ULong i = 0; i < handles.length(); ++i) {
        result.push_back(unwrap_string_from_const_char_ptr(handles[i]) );
    }

    return result;
}

Epp::RequestParams
unwrap_EppParams(const ccReg::EppParams& _epp_request_params)
{
    Epp::RequestParams result;
    CorbaConversion::unwrap_int(_epp_request_params.loginID, result.session_id);
    result.client_transaction_id = unwrap_string(_epp_request_params.clTRID);
    unsigned long long log_request_id;
    CorbaConversion::unwrap_int(_epp_request_params.requestID, log_request_id);
    if (log_request_id != 0) {
        result.log_request_id = log_request_id;
    }
    return result;
}

struct ExceptionInvalidIdentType {};

struct ExceptionInvalidParam {};

namespace {

/**
 * @throws ExceptionInvalidParam
 */
ccReg::ParamError
wrap_param_error(Epp::Param::Enum _param)
{

    switch(_param) {
        case Epp::Param::poll_msg_id:            return ccReg::poll_msgID;
        case Epp::Param::contact_handle:         return ccReg::contact_handle;
        case Epp::Param::contact_cc:             return ccReg::contact_cc;
        case Epp::Param::nsset_handle:           return ccReg::nsset_handle;
        case Epp::Param::nsset_tech:             return ccReg::nsset_tech;
        case Epp::Param::nsset_dns_name:         return ccReg::nsset_dns_name;
        case Epp::Param::nsset_dns_addr:         return ccReg::nsset_dns_addr;
        case Epp::Param::nsset_dns_name_add:     return ccReg::nsset_dns_name_add;
        case Epp::Param::nsset_dns_name_rem:     return ccReg::nsset_dns_name_rem;
        case Epp::Param::nsset_tech_add:         return ccReg::nsset_tech_add;
        case Epp::Param::nsset_tech_rem:         return ccReg::nsset_tech_rem;
        case Epp::Param::domain_fqdn:            return ccReg::domain_fqdn;
        case Epp::Param::domain_registrant:      return ccReg::domain_registrant;
        case Epp::Param::domain_nsset:           return ccReg::domain_nsset;
        case Epp::Param::domain_keyset:          return ccReg::domain_keyset;
        case Epp::Param::domain_period:          return ccReg::domain_period;
        case Epp::Param::domain_admin:           return ccReg::domain_admin;
        case Epp::Param::domain_tmpcontact:      return ccReg::domain_tmpcontact;
        case Epp::Param::domain_ext_val_date:    return ccReg::domain_ext_valDate;
        case Epp::Param::domain_ext_val_date_missing: return ccReg::domain_ext_valDate_missing;
        case Epp::Param::domain_cur_exp_date:    return ccReg::domain_curExpDate;
        case Epp::Param::domain_admin_add:       return ccReg::domain_admin_add;
        case Epp::Param::domain_admin_rem:       return ccReg::domain_admin_rem;
        case Epp::Param::keyset_handle:          return ccReg::keyset_handle;
        case Epp::Param::keyset_tech:            return ccReg::keyset_tech;
        case Epp::Param::keyset_dsrecord:        return ccReg::keyset_dsrecord;
        case Epp::Param::keyset_dsrecord_add:    return ccReg::keyset_dsrecord_add;
        case Epp::Param::keyset_dsrecord_rem:    return ccReg::keyset_dsrecord_rem;
        case Epp::Param::keyset_tech_add:        return ccReg::keyset_tech_add;
        case Epp::Param::keyset_tech_rem:        return ccReg::keyset_tech_rem;
        case Epp::Param::registrar_autor:        return ccReg::registrar_autor;
        case Epp::Param::keyset_dnskey:          return ccReg::keyset_dnskey;
        case Epp::Param::keyset_dnskey_add:      return ccReg::keyset_dnskey_add;
        case Epp::Param::keyset_dnskey_rem:      return ccReg::keyset_dnskey_rem;
    };

    throw ExceptionInvalidParam();
}

} // namespace {anonymous}

void
wrap_Epp_EppResponseSuccessLocalized(
        const Epp::EppResponseSuccessLocalized& _epp_response,
        const std::string& _server_transaction_handle,
        ccReg::Response& _dst)
{
    const Epp::EppResultSuccessLocalized epp_result = _epp_response.epp_result();

    CorbaConversion::wrap_int(Epp::EppResultCode::to_description_db_id(epp_result.epp_result_code()), _dst.code);
    _dst.svTRID = wrap_string_to_corba_string(_server_transaction_handle);
    _dst.msg    = wrap_string_to_corba_string(epp_result.epp_result_description());
}

ccReg::Response
wrap_Epp_EppResponseSuccessLocalized(
        const Epp::EppResponseSuccessLocalized& _epp_response,
        const std::string& _server_transaction_handle)
{
    ccReg::Response response;

    const Epp::EppResultSuccessLocalized epp_result = _epp_response.epp_result();

    CorbaConversion::wrap_int(Epp::EppResultCode::to_description_db_id(epp_result.epp_result_code()), response.code);
    response.svTRID = wrap_string_to_corba_string(_server_transaction_handle);
    response.msg    = wrap_string_to_corba_string(epp_result.epp_result_description());

    return response;
}

ccReg::EPP::EppError
wrap_Epp_EppResponseFailureLocalized(
        const Epp::EppResponseFailureLocalized& _epp_response,
        const std::string& _server_transaction_handle)
{
    ccReg::EPP::EppError result;

    const Epp::EppResultFailureLocalized& epp_result = _epp_response.epp_result();

    CorbaConversion::wrap_int(Epp::EppResultCode::to_description_db_id(epp_result.epp_result_code()), result.errCode);
    result.svTRID = wrap_string_to_corba_string(_server_transaction_handle);
    result.errMsg = wrap_string_to_corba_string(epp_result.epp_result_description());

    const boost::optional<std::set<Epp::EppExtendedErrorLocalized> >& epp_extended_errors =
            epp_result.extended_errors();

    if(epp_extended_errors) {
        const std::set<Epp::EppExtendedErrorLocalized>::size_type size = epp_extended_errors->size();
        result.errorList.length(size);

        int i = 0;
        for(std::set<Epp::EppExtendedErrorLocalized>::const_iterator epp_extended_error = epp_extended_errors->begin();
            epp_extended_error != epp_extended_errors->end();
            ++epp_extended_error, ++i)
        {
            result.errorList[i].code = wrap_param_error(epp_extended_error->param());
            CorbaConversion::wrap_int(epp_extended_error->position(), result.errorList[i].position);
            result.errorList[i].reason = wrap_string_to_corba_string(epp_extended_error->reason_description());
        }
    }
    return result;
}

Optional<std::string>
unwrap_string_for_change_to_Optional_string(const char* _src)
{
    const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

    return
        unwrapped_src.empty()
        ? Optional<std::string>() // do not set
        : boost::trim_copy(unwrapped_src);
}

Optional<std::string>
unwrap_string_for_change_or_remove_to_Optional_string(const char* _src)
{
    const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

    /* Defined by convention. Could be substituted by more explicit means in IDL interface
     * (like using _add and _rem elements, not just _chg for all operations). */
    static const char char_for_value_deleting = '\b';

    return
        unwrapped_src.empty()
        ? Optional<std::string>() // do not set
        : unwrapped_src.at(0) == char_for_value_deleting
            ? std::string() // set empty
            : boost::trim_copy(unwrapped_src);
}

void
wrap_Epp_ObjectStatesLocalized(const Epp::ObjectStatesLocalized& _src, ccReg::Status& _dst)
{
    if (_src.descriptions.empty()) {
        _dst.length(1);
        _dst[0].value = "ok";
        _dst[0].text = _src.success_state_localized_description.c_str();
        return;
    }
    _dst.length(_src.descriptions.size());
    ::size_t idx = 0;
    for (Epp::ObjectStatesLocalized::Descriptions::const_iterator state_ptr = _src.descriptions.begin();
         state_ptr != _src.descriptions.end(); ++state_ptr, ++idx)
    {
        _dst[idx].value = Conversion::Enums::to_db_handle(state_ptr->first).c_str();
        _dst[idx].text = state_ptr->second.c_str();
    }
}

} // namespace Corba
