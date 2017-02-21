/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CORBA_CONVERSIONS_H_0DA22651C0FF48C5A0AF69E77BB4A561
#define CORBA_CONVERSIONS_H_0DA22651C0FF48C5A0AF69E77BB4A561

#include "src/corba/EPP.hh"
#include "src/epp/contact/check_contact_localized.h"
#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/create_contact_localized.h"
#include "src/epp/contact/info_contact_localized.h"
#include "src/epp/contact/update_contact_localized.h"
#include "src/epp/domain/impl/domain_enum_validation.h"
#include "src/epp/domain/impl/domain_registration_time.h"
#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/request_params.h"
#include "src/epp/keyset/check_keyset_localized.h"
#include "src/epp/keyset/info_keyset_localized.h"
#include "src/epp/nsset/check_nsset_localized.h"
#include "src/epp/nsset/delete_nsset_localized.h"
#include "src/epp/nsset/impl/dns_host_input.h"
#include "src/epp/nsset/info_nsset_localized.h"
#include "src/old_utils/util.h"
#include "util/corba_conversion.h"

#include <boost/date_time/posix_time/ptime.hpp>

#include <string>
#include <vector>

namespace Corba {

CORBA::String_var
wrap_Nullable_string_to_string(const Nullable<std::string>& src);


CORBA::String_var
wrap_boost_posix_time_ptime_to_string(const boost::posix_time::ptime& _src);


CORBA::String_var
wrap_Nullable_boost_posix_time_ptime_to_string(const Nullable<boost::posix_time::ptime>& _src);


/**
 * Unwrapper for attributes which can be empty with special meaning
 *
 * @param _src string to be unwrapped, should not be NULL
 *
 * @return Optional() if input string empty, else unwrapped input
 */
Optional<std::string>
unwrap_string_for_change_to_Optional_string(const char* _src);


/**
 * Unwrapper for attributes which can be empty with special meaning and can have control char with special meaning
 *
 * @param _src string to be unwrapped, should not be NULL
 *
 * @return Optional() if input string empty, empty string if input contains special control char, unwrapped input in other cases
 */
Optional<std::string>
unwrap_string_for_change_or_remove_to_Optional_string(const char* _src);


std::vector<std::string>
unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles);


Epp::RequestParams
unwrap_EppParams(const ccReg::EppParams& _epp_request_params);


void
wrap_Epp_EppResponseSuccessLocalized(
        const Epp::EppResponseSuccessLocalized& _input,
        const std::string& _server_transaction_handle,
        ccReg::Response& _dst);


ccReg::Response
wrap_Epp_EppResponseSuccessLocalized(
        const Epp::EppResponseSuccessLocalized& _input,
        const std::string& _server_transaction_handle);


ccReg::EPP::EppError
wrap_Epp_EppResponseFailureLocalized(
        const Epp::EppResponseFailureLocalized& _epp_response_failure,
        const std::string& _server_transaction_handle);


void
wrap_Epp_ObjectStatesLocalized(
        const Epp::ObjectStatesLocalized& _src,
        ccReg::Status& _dst);


/**
 * Converts and formats time
 *
 * @param _utc_ptime time in UTC
 *
 * @return time converted to local time zone, with seconds fraction trimmed, formatted as RFC3339 string
 */
std::string
convert_time_to_local_rfc3339(const boost::posix_time::ptime& _utc_ptime);


} // namespace Corba

#endif
