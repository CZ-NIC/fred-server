/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
#ifndef KEYSET_CORBA_CONVERSIONS_HH_26FBBF05C7634EEC8B29B07F3AF20127
#define KEYSET_CORBA_CONVERSIONS_HH_26FBBF05C7634EEC8B29B07F3AF20127

#include "corba/EPP.hh"

#include "src/backend/epp/keyset/check_keyset_localized.hh"
#include "src/backend/epp/keyset/dns_key.hh"
#include "src/backend/epp/keyset/ds_record.hh"
#include "src/backend/epp/keyset/info_keyset_localized_output_data.hh"
#include "src/backend/epp/keyset/info_keyset_output_data.hh"

#include <string>
#include <vector>

namespace LibFred {
namespace Corba {

std::vector<std::string>
unwrap_TechContact_to_vector_string(const ccReg::TechContact& _tech_contacts);


std::vector< ::Epp::Keyset::DsRecord>
unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(const ccReg::DSRecord& _ds_records);


std::vector< ::Epp::Keyset::DnsKey>
unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(const ccReg::DNSKey& _dns_keys);


/**
 * @returns data ordered the same way as input handles
 */
void
wrap_Epp_Keyset_Localized_CheckKeysetLocalizedResponse_Results(
        const std::vector<std::string>& handles,
        const ::Epp::Keyset::CheckKeysetLocalizedResponse::Results& check_results,
        ccReg::CheckResp& dst);


void
wrap_Epp_Keyset_Localized_InfoKeysetLocalizedOutputData(
        const ::Epp::Keyset::InfoKeysetLocalizedOutputData& _src,
        ccReg::KeySet& _dst);


} // namespace LibFred::Corba
} // namespace Corba

#endif
