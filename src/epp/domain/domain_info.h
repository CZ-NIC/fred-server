/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

/**
 *  @file domain_info.h
 *  <++>
 */

#ifndef SRC_EPP_DOMAIN_DOMAIN_INFO_H
#define SRC_EPP_DOMAIN_DOMAIN_INFO_H

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/object/object_state.h"
#include "util/db/nullable.h"

#include <map>
#include <set>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/optional.hpp>

namespace Epp {

namespace Domain {

struct DomainInfoLocalizedOutputData {
    typedef std::set<Fred::Object_State::Enum> States;
    std::string roid; ///< Domain repository ID
    std::string fqdn; ///< Domain FQDN
    std::string registrant;
    Nullable<std::string> nsset;
    Nullable<std::string> keyset;
    LocalizedStates localized_external_states; ///< Domain states list
    std::string sponsoring_registrar_handle; ///< Registrar which has to right for change
    std::string creating_registrar_handle; ///< Registrar which created contact
    Nullable<std::string> last_update_registrar_handle; ///< Registrar which realized changes
    boost::posix_time::ptime crdate; ///< Creation date and time
    Nullable<boost::posix_time::ptime> last_update; ///< Date and time of last change
    Nullable<boost::posix_time::ptime> last_transfer; ///< Date and time of last transfer
    boost::gregorian::date exdate;
    Nullable<std::string> auth_info_pw; ///< Password for keyset transfer
    std::set<std::string> admin; ///< List of contacts identifier
    Nullable<Fred::ENUMValidationExtension> ext_enum_domain_validation; ///< ENUM domain validation extension info
    std::set<std::string> tmpcontact; ///< List of contacts identifier OBSOLETE
};

struct DomainInfoResponse {
    const LocalizedSuccessResponse localized_success_response;
    const DomainInfoLocalizedOutputData localized_domain_info_output_data;

    DomainInfoResponse(
        const LocalizedSuccessResponse& localized_success_response,
        const DomainInfoLocalizedOutputData& localized_domain_info_output_data
    ) :
        localized_success_response(localized_success_response),
        localized_domain_info_output_data(localized_domain_info_output_data)
    { }
};

DomainInfoResponse domain_info(
    const std::string& domain_fqdn,
    unsigned long long registrar_id,
    SessionLang::Enum lang,
    const std::string& server_transaction_handle
);

}

}

#endif
