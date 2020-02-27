/*
 * Copyright (C) 2016-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/corba/epp/corba_conversions.hh"

#include "src/bin/corba/epp/epp_legacy_compatibility.hh"
#include "corba/EPP.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/create_contact_localized.hh"
#include "src/backend/epp/param.hh"
#include "src/backend/epp/keyset/check_keyset_localized.hh"
#include "src/backend/epp/keyset/info_keyset_localized.hh"
#include "src/backend/epp/nsset/info_nsset_localized.hh"

#include "src/backend/epp/epp_extended_error_localized.hh"
#include "src/backend/epp/epp_response_failure_localized.hh"
#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure_localized.hh"

#include "util/db/nullable.hh"
#include "util/map_at.hh"
#include "util/optional_value.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/integer_traits.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace LibFred {
namespace Corba {

namespace {

/**
 * @throws std::logic_error
 */
ccReg::ParamError
wrap_param_error(Epp::Param::Enum _param)
{

    switch (_param)
    {
        case Epp::Param::poll_msg_id:
            return ccReg::poll_msgID;

        case Epp::Param::contact_handle:
            return ccReg::contact_handle;

        case Epp::Param::contact_cc:
            return ccReg::contact_cc;

        case Epp::Param::nsset_handle:
            return ccReg::nsset_handle;

        case Epp::Param::nsset_tech:
            return ccReg::nsset_tech;

        case Epp::Param::nsset_dns_name:
            return ccReg::nsset_dns_name;

        case Epp::Param::nsset_dns_addr:
            return ccReg::nsset_dns_addr;

        case Epp::Param::nsset_dns_name_add:
            return ccReg::nsset_dns_name_add;

        case Epp::Param::nsset_dns_name_rem:
            return ccReg::nsset_dns_name_rem;

        case Epp::Param::nsset_tech_add:
            return ccReg::nsset_tech_add;

        case Epp::Param::nsset_tech_rem:
            return ccReg::nsset_tech_rem;

        case Epp::Param::domain_fqdn:
            return ccReg::domain_fqdn;

        case Epp::Param::domain_registrant:
            return ccReg::domain_registrant;

        case Epp::Param::domain_nsset:
            return ccReg::domain_nsset;

        case Epp::Param::domain_keyset:
            return ccReg::domain_keyset;

        case Epp::Param::domain_period:
            return ccReg::domain_period;

        case Epp::Param::domain_admin:
            return ccReg::domain_admin;

        case Epp::Param::domain_tmpcontact:
            return ccReg::domain_tmpcontact;

        case Epp::Param::domain_ext_val_date:
            return ccReg::domain_ext_valDate;

        case Epp::Param::domain_ext_val_date_missing:
            return ccReg::domain_ext_valDate_missing;

        case Epp::Param::domain_cur_exp_date:
            return ccReg::domain_curExpDate;

        case Epp::Param::domain_admin_add:
            return ccReg::domain_admin_add;

        case Epp::Param::domain_admin_rem:
            return ccReg::domain_admin_rem;

        case Epp::Param::keyset_handle:
            return ccReg::keyset_handle;

        case Epp::Param::keyset_tech:
            return ccReg::keyset_tech;

        case Epp::Param::keyset_dsrecord:
            return ccReg::keyset_dsrecord;

        case Epp::Param::keyset_dsrecord_add:
            return ccReg::keyset_dsrecord_add;

        case Epp::Param::keyset_dsrecord_rem:
            return ccReg::keyset_dsrecord_rem;

        case Epp::Param::keyset_tech_add:
            return ccReg::keyset_tech_add;

        case Epp::Param::keyset_tech_rem:
            return ccReg::keyset_tech_rem;

        case Epp::Param::registrar_autor:
            return ccReg::registrar_autor;

        case Epp::Param::keyset_dnskey:
            return ccReg::keyset_dnskey;

        case Epp::Param::keyset_dnskey_add:
            return ccReg::keyset_dnskey_add;

        case Epp::Param::keyset_dnskey_rem:
            return ccReg::keyset_dnskey_rem;
    }

    throw std::logic_error("Unexpected Epp::Param::Enum value.");
}


// represents RFC 3339 time offset formatted with respect to RFC 5731
// append to boost::posix_time::to_iso_extended_string() to get RFC3339 timestamp
struct TimeZoneOffset
{
    boost::optional<boost::posix_time::time_duration> time_zone_offset_;


    explicit TimeZoneOffset(const boost::posix_time::ptime& _utc_time)
    {
        try {
            time_zone_offset_ =
                    boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(_utc_time) - _utc_time;
        }
        catch (...) {
            // time zone offset will be unknown
        }
    }

    std::string to_rfc3339_string()
    {
        if (time_zone_offset_) // local time zone offset is known
        {
            if (time_zone_offset_->hours() || time_zone_offset_->minutes()) // local time zone offset is != 0
            {
                return boost::str(
                        boost::format("%1$+03d:%2$02d")
                                % time_zone_offset_->hours()
                                % boost::date_time::absolute_value(time_zone_offset_->minutes()));
            }
            else // local time zone is UTC
            {
                // uppercase "Z" MUST be used according to RFC 5731 section 2.4. "Dates and Times"
                return std::string("Z");
            }
        }
        else // local time zone offset is unknown
        {
            return std::string("-00:00"); // unknown local zone
        }
    }

};

} // namespace LibFred::Corba::{anonymous}

std::string
convert_time_to_local_rfc3339(const boost::posix_time::ptime& utc_ptime)
{
    // _utc_ptime converted to local ptime with seconds fraction trimmed
    const boost::posix_time::ptime local_ptime =
            boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(
                    boost::posix_time::ptime(
                            utc_ptime.date(),
                            boost::posix_time::seconds(utc_ptime.time_of_day().total_seconds())));

    return
        boost::posix_time::to_iso_extended_string(local_ptime) +
        TimeZoneOffset(local_ptime).to_rfc3339_string();
}

CORBA::String_var
wrap_Nullable_string_to_string(const Nullable<std::string>& src)
{
    return src.isnull() ? "" : src.get_value().c_str();
}

CORBA::String_var
wrap_boost_posix_time_ptime_to_string(const boost::posix_time::ptime& _src)
{
    return convert_time_to_local_rfc3339(_src).c_str();
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

    for (CORBA::ULong i = 0; i < handles.length(); ++i)
    {
        result.push_back(unwrap_string_from_const_char_ptr(handles[i]));
    }

    return result;
}

Epp::RequestParams
unwrap_EppParams(const ccReg::EppParams& _epp_request_params)
{
    Epp::RequestParams result;
    unwrap_int(_epp_request_params.loginID, result.session_id);
    result.client_transaction_id = unwrap_string(_epp_request_params.clTRID);
    unsigned long long log_request_id;
    unwrap_int(_epp_request_params.requestID, log_request_id);
    if (log_request_id != 0)
    {
        result.log_request_id = log_request_id;
    }

    return result;
}

void
wrap_Epp_EppResponseSuccessLocalized(
        const Epp::EppResponseSuccessLocalized& _epp_response,
        const std::string& _server_transaction_handle,
        ccReg::Response& _dst)
{
    const Epp::EppResultSuccessLocalized epp_result = _epp_response.epp_result();

    wrap_int(Epp::EppResultCode::to_description_db_id(epp_result.epp_result_code()), _dst.code);
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

    wrap_int(Epp::EppResultCode::to_description_db_id(epp_result.epp_result_code()), response.code);
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

    wrap_int(Epp::EppResultCode::to_description_db_id(epp_result.epp_result_code()), result.errCode);
    result.svTRID = wrap_string_to_corba_string(_server_transaction_handle);
    result.errMsg = wrap_string_to_corba_string(epp_result.epp_result_description());

    const boost::optional<std::set<Epp::EppExtendedErrorLocalized> >& epp_extended_errors =
        epp_result.extended_errors();

    if (epp_extended_errors)
    {
        const std::set<Epp::EppExtendedErrorLocalized>::size_type size = epp_extended_errors->size();
        result.errorList.length(size);

        CORBA::ULong i = 0;
        for (std::set<Epp::EppExtendedErrorLocalized>::const_iterator epp_extended_error =
                 epp_extended_errors->begin();
             epp_extended_error != epp_extended_errors->end();
             ++epp_extended_error, ++i)
        {
            result.errorList[i].code = wrap_param_error(epp_extended_error->param());
            wrap_int(epp_extended_error->position(), result.errorList[i].position);
            result.errorList[i].reason =
                wrap_string_to_corba_string(epp_extended_error->reason_description());
        }
    }

    return result;
}

Optional<std::string>
unwrap_string_for_change_to_Optional_string(const char* _src)
{
    const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

    return unwrapped_src.empty()
                   ? Optional<std::string>() // do not set
                   : unwrapped_src;
}

Optional<std::string>
unwrap_string_for_change_or_remove_to_Optional_string(const char* _src)
{
    const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

    /* Defined by convention. Could be substituted by more explicit means in IDL interface
     * (like using _add and _rem elements, not just _chg for all operations). */
    static const char char_for_value_deleting = '\b';

    return unwrapped_src.empty()
                   ? Optional<std::string>() // do not set
                   : unwrapped_src.at(0) == char_for_value_deleting
                             ? std::string() // set empty
                             : unwrapped_src;
}

} // namespace LibFred::Corba
} // namespace LibFred
