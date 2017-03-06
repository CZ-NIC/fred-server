/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_H_EE5157F1F74A49D28A6B8F4CC22BB363
#define POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_H_EE5157F1F74A49D28A6B8F4CC22BB363

#include "src/fredlib/opcontext.h"
#include "src/fredlib/domain/info_domain.h"

#include "src/epp/domain/impl/domain_enum_validation.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>

namespace Epp {
namespace Poll {

// could be merged with code from info_domain?
struct InfoDomainOutputData {
    typedef std::set<Fred::Object_State::Enum> States;

    std::string roid;
    std::string fqdn;
    std::string registrant;
    Nullable<std::string> nsset;
    Nullable<std::string> keyset;
    States states;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    boost::posix_time::ptime crdate;
    Nullable<boost::posix_time::ptime> last_update;
    Nullable<boost::posix_time::ptime> last_transfer;
    boost::gregorian::date exdate;
    boost::optional<std::string> authinfopw;
    std::set<std::string> admin;
    Nullable<Epp::Domain::EnumValidationExtension> ext_enum_domain_validation;
    std::set<std::string> tmpcontact;
};

struct PollRequestUpdateDomainOutputData
{
    InfoDomainOutputData old_data;
    InfoDomainOutputData new_data;
};

PollRequestUpdateDomainOutputData poll_request_get_update_domain_details(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
