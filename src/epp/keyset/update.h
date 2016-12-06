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
 *  @file
 */

#ifndef UPDATE_H_6824E4BD1A024181B0112B319CFC0EE5
#define UPDATE_H_6824E4BD1A024181B0112B319CFC0EE5

#include "src/fredlib/opcontext.h"
#include "src/epp/keyset/info_data.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct KeysetUpdateResult
{
    unsigned long long id;
    unsigned long long update_history_id;
};

/**
 * @throws ObjectStatusProhibitsOperation
 * @throws ParameterErrors
 */
KeysetUpdateResult keyset_update(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts_add,
    const std::vector< std::string > &_tech_contacts_rem,
    const std::vector< Keyset::DsRecord > &_ds_records_add,
    const std::vector< Keyset::DsRecord > &_ds_records_rem,
    const std::vector< Keyset::DnsKey > &_dns_keys_add,
    const std::vector< Keyset::DnsKey > &_dns_keys_rem,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id);

}

#endif
