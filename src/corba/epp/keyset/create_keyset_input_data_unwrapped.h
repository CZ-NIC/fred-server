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

#ifndef CREATE_KEYSET_INPUT_DATA_UNWRAPPED_H_B08116E5B57143D9AC3BF03E7A5D82C3
#define CREATE_KEYSET_INPUT_DATA_UNWRAPPED_H_B08116E5B57143D9AC3BF03E7A5D82C3

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/keyset/keyset_corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/keyset/dns_key.h"
#include "src/epp/keyset/ds_record.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

namespace Fred {
namespace Corba {
namespace Epp {
namespace Keyset {

struct CreateKeysetInputDataUnwrapped
{
    std::string keyset_handle;
    Optional<std::string> authinfopw;
    std::vector<std::string> tech_contacts;
    std::vector< ::Epp::Keyset::DsRecord> ds_records;
    std::vector< ::Epp::Keyset::DnsKey> dns_keys;


    CreateKeysetInputDataUnwrapped(
            const char* _keyset_handle,
            const char* _authinfopw,
            const ccReg::TechContact& _tech_contacts,
            const ccReg::DSRecord& _ds_records,
            const ccReg::DNSKey& _dns_keys)
        : keyset_handle(Fred::Corba::unwrap_string_from_const_char_ptr(_keyset_handle)),
          tech_contacts(unwrap_TechContact_to_vector_string(_tech_contacts)),
          ds_records(unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(_ds_records)),
          dns_keys(unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(_dns_keys))
    {
        const std::string authinfopw_value = Corba::unwrap_string_from_const_char_ptr(_authinfopw);
        authinfopw = authinfopw_value.empty() ? Optional<std::string>() : Optional<std::string>(authinfopw_value);
    }


};

} // namespace Fred::Corba::Epp::Keyset
} // namespace Fred::Corba::Epp
} // namespace Fred::Corba
} // namespace Fred

#endif
