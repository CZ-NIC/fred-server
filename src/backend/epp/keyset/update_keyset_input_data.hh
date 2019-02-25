/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef UPDATE_KEYSET_INPUT_DATA_HH_254A2B82C5C845418AC5724CF762CA7C
#define UPDATE_KEYSET_INPUT_DATA_HH_254A2B82C5C845418AC5724CF762CA7C

#include "src/backend/epp/keyset/dns_key.hh"
#include "src/backend/epp/keyset/ds_record.hh"
#include "util/optional_value.hh"

#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

struct UpdateKeysetInputData
{
    const std::string keyset_handle;
    const Optional<std::string> authinfopw;
    const std::vector<std::string> tech_contacts_add;
    const std::vector<std::string> tech_contacts_rem;
    const std::vector<Keyset::DsRecord> ds_records_add;
    const std::vector<Keyset::DsRecord> ds_records_rem;
    const std::vector<Keyset::DnsKey> dns_keys_add;
    const std::vector<Keyset::DnsKey> dns_keys_rem;


    UpdateKeysetInputData(
            const std::string& _keyset_handle,
            const Optional<std::string>& _authinfopw,
            const std::vector<std::string>& _tech_contacts_add,
            const std::vector<std::string>& _tech_contacts_rem,
            const std::vector<Keyset::DsRecord>& _ds_records_add,
            const std::vector<Keyset::DsRecord>& _ds_records_rem,
            const std::vector<Keyset::DnsKey>& _dns_keys_add,
            const std::vector<Keyset::DnsKey>& _dns_keys_rem)
        : keyset_handle(_keyset_handle),
          authinfopw(_authinfopw),
          tech_contacts_add(_tech_contacts_add),
          tech_contacts_rem(_tech_contacts_rem),
          ds_records_add(_ds_records_add),
          ds_records_rem(_ds_records_rem),
          dns_keys_add(_dns_keys_add),
          dns_keys_rem(_dns_keys_rem)
    {
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
