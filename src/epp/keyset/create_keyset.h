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

#ifndef CREATE_KEYSET_H_40D13A301E374731AE4EA4486456DAF6
#define CREATE_KEYSET_H_40D13A301E374731AE4EA4486456DAF6

#include "src/fredlib/opcontext.h"
#include "src/epp/keyset/impl/info_keyset_data.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Keyset {

struct CreateKeysetResult
{
    unsigned long long id;
    unsigned long long create_history_id;
    boost::posix_time::ptime crdate;
};

/**
 * @throws AuthErrorServerClosingConnection
 * @throws ParameterErrors
 */
CreateKeysetResult create_keyset(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts,
    const std::vector< Keyset::DsRecord > &_ds_records,
    const std::vector< Keyset::DnsKey > &_dns_keys,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id);

} // namespace Epp::Keyset
} // namespace Epp

#endif
