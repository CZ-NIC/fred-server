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

#ifndef UPDATE_KEYSET_INPUT_DATA_UNWRAPPED_H_A780BF014DC0455192BBA5CCD95461CB
#define UPDATE_KEYSET_INPUT_DATA_UNWRAPPED_H_A780BF014DC0455192BBA5CCD95461CB

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

struct UpdateKeysetInputDataUnwrapped
{
    const std::string keyset_handle;
    const Optional<std::string> authinfopw;
    const std::vector<std::string> tech_contacts_add;
    const std::vector<std::string> tech_contacts_rem;
    const std::vector< ::Epp::Keyset::DsRecord> ds_records_add;
    const std::vector< ::Epp::Keyset::DsRecord> ds_records_rem;
    const std::vector< ::Epp::Keyset::DnsKey> dns_keys_add;
    const std::vector< ::Epp::Keyset::DnsKey> dns_keys_rem;


    UpdateKeysetInputDataUnwrapped(
            const char* _keyset_handle,
            const char* _authinfopw,
            const ccReg::TechContact& _tech_contacts_add,
            const ccReg::TechContact& _tech_contacts_rem,
            const ccReg::DSRecord& _ds_records_add,
            const ccReg::DSRecord& _ds_records_rem,
            const ccReg::DNSKey& _dns_keys_add,
            const ccReg::DNSKey& _dns_keys_rem)
        : keyset_handle(Fred::Corba::unwrap_string_from_const_char_ptr(_keyset_handle)),
          authinfopw(Fred::Corba::unwrap_string_for_change_or_remove_to_Optional_string(_authinfopw)),
          tech_contacts_add(unwrap_TechContact_to_vector_string(_tech_contacts_add)),
          tech_contacts_rem(unwrap_TechContact_to_vector_string(_tech_contacts_rem)),
          ds_records_add(unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(_ds_records_add)),
          ds_records_rem(unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(_ds_records_rem)),
          dns_keys_add(unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(_dns_keys_add)),
          dns_keys_rem(unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(_dns_keys_rem))
    {
    }


};

} // namespace Fred::Corba::Epp::Keyset
} // namespace Fred::Corba::Epp
} // namespace Fred::Corba
} // namespace Fred

#endif
