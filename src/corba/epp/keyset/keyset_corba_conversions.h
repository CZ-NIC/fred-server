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

#ifndef KEYSET_CORBA_CONVERSIONS_H_4A12976A448146678886FC17E4DB0731
#define KEYSET_CORBA_CONVERSIONS_H_4A12976A448146678886FC17E4DB0731

#include "src/corba/EPP.hh"

#include "src/epp/keyset/check_keyset_localized.h"
#include "src/epp/keyset/impl/dns_key.h"
#include "src/epp/keyset/impl/ds_record.h"
#include "src/epp/keyset/impl/info_keyset_localized_output_data.h"
#include "src/epp/keyset/impl/info_keyset_output_data.h"

#include <string>
#include <vector>

namespace Corba {

std::vector<std::string>
unwrap_TechContact_to_vector_string(const ccReg::TechContact& _tech_contacts);


std::vector<Epp::Keyset::DsRecord>
unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(const ccReg::DSRecord& _ds_records);


void
unwrap_ccReg_DSRecord_str(
        const ccReg::DSRecord_str& _src,
        Epp::Keyset::DsRecord& _dst);


std::vector<Epp::Keyset::DnsKey>
unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(const ccReg::DNSKey& _dns_keys);


/**
 * @returns data ordered the same way as input handles
 */
void
wrap_Epp_Keyset_Localized_CheckKeysetLocalizedResponse_Results(
        const std::vector<std::string>& handles,
        const Epp::Keyset::CheckKeysetLocalizedResponse::Results& check_results,
        ccReg::CheckResp& dst);


void
wrap_Epp_InfoKeysetOutputData_TechContacts(
        const Epp::Keyset::InfoKeysetOutputData::TechContacts& _src,
        ccReg::TechContact& _dst);


void
wrap_Epp_Keyset_Localized_InfoKeysetLocalizedOutputData(
        const Epp::Keyset::InfoKeysetLocalizedOutputData& _src,
        ccReg::KeySet& _dst);


} // namespace Corba

#endif
