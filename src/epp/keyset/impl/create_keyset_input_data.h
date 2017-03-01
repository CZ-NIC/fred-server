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

#ifndef CREATE_KEYSET_INPUT_DATA_H_E792AB9CC4F04542B0680C2BC2B7B952
#define CREATE_KEYSET_INPUT_DATA_H_E792AB9CC4F04542B0680C2BC2B7B952

#include "src/epp/keyset/impl/dns_key.h"
#include "src/epp/keyset/impl/ds_record.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

struct CreateKeysetInputData
{
    const std::string& keyset_handle;
    const Optional<std::string>& authinfopw;
    const std::vector<std::string>& tech_contacts;
    const std::vector<Keyset::DsRecord>& ds_records;
    const std::vector<Keyset::DnsKey>& dns_keys;


    CreateKeysetInputData(
            const std::string& _keyset_handle,
            const Optional<std::string>& _authinfopw,
            const std::vector<std::string>& _tech_contacts,
            const std::vector<Keyset::DsRecord>& _ds_records,
            const std::vector<Keyset::DnsKey>& _dns_keys)
        : keyset_handle(_keyset_handle),
          authinfopw(_authinfopw),
          tech_contacts(_tech_contacts),
          ds_records(_ds_records),
          dns_keys(_dns_keys)
    {
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
