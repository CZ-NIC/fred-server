/*
 * Copyright (C) 2017-2022  CZ.NIC, z. s. p. o.
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

#include "src/bin/corba/epp/keyset/keyset_corba_conversions.hh"

#include "corba/EPP.hh"

#include "src/bin/corba/epp/corba_conversions.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"

#include <string>
#include <vector>

namespace LibFred {
namespace Corba {
namespace {

void
unwrap_ccReg_DSRecord_str(
        const ccReg::DSRecord_str& _src,
        Epp::Keyset::DsRecord& _dst)
{
    long key_tag;
    LibFred::Corba::unwrap_int(_src.keyTag, key_tag);
    long alg;
    LibFred::Corba::unwrap_int(_src.alg, alg);
    long digest_type;
    LibFred::Corba::unwrap_int(_src.digestType, digest_type);
    const std::string digest = unwrap_string(_src.digest);
    long max_sig_life;
    LibFred::Corba::unwrap_int(_src.maxSigLife, max_sig_life);
    _dst = Epp::Keyset::DsRecord(key_tag, alg, digest_type, digest, max_sig_life);
}

void
unwrap_ccReg_DNSKey_str(
        const ccReg::DNSKey_str& _src,
        Epp::Keyset::DnsKey& _dst)
{
    unsigned short flags;
    LibFred::Corba::unwrap_int(_src.flags, flags);
    unsigned short protocol;
    LibFred::Corba::unwrap_int(_src.protocol, protocol);
    unsigned short alg;
    LibFred::Corba::unwrap_int(_src.alg, alg);
    const std::string key = unwrap_string(_src.key);
    _dst = Epp::Keyset::DnsKey(flags, protocol, alg, key);
}

ccReg::CheckAvail
wrap_keyset_handle_check_result(
        const Nullable<Epp::Keyset::CheckKeysetLocalizedResponse::Result>& _check_result)
{
    if (_check_result.isnull())
    {
        return ccReg::NotExist;
    }

    switch (_check_result.get_value().state)
    {
        case Epp::Keyset::KeysetHandleRegistrationObstruction::invalid_handle:
            return ccReg::BadFormat;

        case Epp::Keyset::KeysetHandleRegistrationObstruction::protected_handle:
            return ccReg::DelPeriod;

        case Epp::Keyset::KeysetHandleRegistrationObstruction::registered_handle:
            return ccReg::Exist;
    }

    throw std::runtime_error("unknown keyset handle check result");
}

void
wrap_Epp_InfoKeysetOutputData_TechContacts(
        const Epp::Keyset::InfoKeysetOutputData::TechContacts& _src,
        ccReg::TechContact& _dst)
{
    _dst.length(_src.size());
    ::size_t idx = 0;
    for (Epp::Keyset::InfoKeysetOutputData::TechContacts::const_iterator data_ptr = _src.begin();
         data_ptr != _src.end(); ++data_ptr, ++idx)
    {
        _dst[idx] = data_ptr->c_str();
    }
}

void
wrap_Epp_InfoKeysetOutputData_DnsKeys(
        const Epp::Keyset::InfoKeysetOutputData::DnsKeys& _src,
        ccReg::DNSKey& _dst)
{
    _dst.length(_src.size());
    ::size_t idx = 0;
    for (Epp::Keyset::InfoKeysetOutputData::DnsKeys::const_iterator data_ptr = _src.begin();
         data_ptr != _src.end(); ++data_ptr, ++idx)
    {
        LibFred::Corba::wrap_int(data_ptr->get_flags(),    _dst[idx].flags);
        LibFred::Corba::wrap_int(data_ptr->get_protocol(), _dst[idx].protocol);
        LibFred::Corba::wrap_int(data_ptr->get_alg(),      _dst[idx].alg);
        _dst[idx].key = data_ptr->get_key().c_str();
    }
}

} // namespace LibFred::Corba::{anonymous}

std::vector<std::string>
unwrap_TechContact_to_vector_string(const ccReg::TechContact& _tech_contacts)
{
    std::vector<std::string> result;
    result.reserve(_tech_contacts.length());

    for (CORBA::ULong idx = 0; idx < _tech_contacts.length(); ++idx)
    {
        result.push_back(unwrap_string_from_const_char_ptr(_tech_contacts[idx]));
    }

    return result;
}

std::vector<Epp::Keyset::DsRecord>
unwrap_ccReg_DSRecord_to_vector_Epp_Keyset_DsRecord(const ccReg::DSRecord& _ds_records)
{
    std::vector<Epp::Keyset::DsRecord> result;
    result.reserve(_ds_records.length());

    for (CORBA::ULong idx = 0; idx < _ds_records.length(); ++idx)
    {
        Epp::Keyset::DsRecord ds_record;
        unwrap_ccReg_DSRecord_str(_ds_records[idx], ds_record);
        result.push_back(ds_record);
    }

    return result;
}

std::vector<Epp::Keyset::DnsKey>
unwrap_ccReg_DNSKey_to_vector_Epp_Keyset_DnsKey(const ccReg::DNSKey& _dns_keys)
{
    std::vector<Epp::Keyset::DnsKey> result;
    result.reserve(_dns_keys.length());

    for (CORBA::ULong idx = 0; idx < _dns_keys.length(); ++idx)
    {
        Epp::Keyset::DnsKey dns_key;
        unwrap_ccReg_DNSKey_str(_dns_keys[idx], dns_key);
        result.push_back(dns_key);
    }

    return result;
}

void
wrap_Epp_Keyset_Localized_CheckKeysetLocalizedResponse_Results(
        const std::vector<std::string>& handles,
        const Epp::Keyset::CheckKeysetLocalizedResponse::Results& check_results,
        ccReg::CheckResp& dst)
{
    dst.length(handles.size());

    typedef std::vector<std::string> Handles;

    ::size_t idx = 0;
    for (Handles::const_iterator handle_ptr = handles.begin();
         handle_ptr != handles.end();
         ++handle_ptr, ++idx)
    {
        typedef Epp::Keyset::CheckKeysetLocalizedResponse::Results CheckResults;

        const CheckResults::const_iterator result_ptr = check_results.find(*handle_ptr);
        if (result_ptr == check_results.end())
        {
            throw std::out_of_range("handle " + (*handle_ptr) + " not found");
        }
        dst[idx].avail = wrap_keyset_handle_check_result(result_ptr->second);
        dst[idx].reason = result_ptr->second.isnull()
                                  ? ""
                                  : result_ptr->second.get_value().description.c_str();
    }
}

void
wrap_Epp_Keyset_Localized_InfoKeysetLocalizedOutputData(
        const Epp::Keyset::InfoKeysetLocalizedOutputData& _src,
        ccReg::KeySet& _dst)
{
    _dst.handle = _src.handle.c_str();
    _dst.ROID = _src.roid.c_str();
    _dst.ClID = _src.sponsoring_registrar_handle.c_str();
    _dst.CrID = _src.creating_registrar_handle.c_str();
    _dst.UpID = wrap_Nullable_string_to_string(_src.last_update_registrar_handle);
    wrap_Epp_ObjectStatesLocalized< ::Epp::Keyset::StatusValue>(_src.localized_external_states, _dst.stat);
    _dst.CrDate = wrap_boost_posix_time_ptime_to_string(_src.crdate);
    _dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_update);
    _dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_transfer);
    _dst.AuthInfoPw = wrap_string_to_corba_string("");
    _dst.dsrec.length(0); // has to be empty
    wrap_Epp_InfoKeysetOutputData_DnsKeys(_src.dns_keys, _dst.dnsk);
    wrap_Epp_InfoKeysetOutputData_TechContacts(_src.tech_contacts, _dst.tech);
}

} // namespace LibFred::Cobra
} // namespace Cobra
