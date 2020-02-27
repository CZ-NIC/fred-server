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
#include "src/bin/corba/epp/nsset/nsset_corba_conversions.hh"

#include "src/bin/corba/epp/corba_conversions.hh"

#include "corba/EPP.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/backend/epp/nsset/dns_host_input.hh"
#include "src/backend/epp/nsset/dns_host_output.hh"
#include "src/backend/epp/nsset/info_nsset_localized_output_data.hh"
#include "src/backend/epp/nsset/nsset_handle_registration_obstruction_localized.hh"
#include "src/deprecated/util/util.hh"
#include "util/map_at.hh"

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace LibFred {
namespace Corba {
namespace {

std::vector<boost::optional<boost::asio::ip::address> >
unwrap_inet_addr_to_vector_asio_addr(const ccReg::InetAddress& in)
{
    std::vector<boost::optional<boost::asio::ip::address> > ret;
    ret.reserve(in.length());
    for (unsigned long long i = 0; i < in.length(); ++i)
    {
        if (in[i] == 0)
        {
            throw std::runtime_error("null char ptr");
        }
        boost::system::error_code boost_error_code; // invalid ip address is transformed to non-initialized optional
        boost::asio::ip::address ipaddr = boost::asio::ip::address::from_string(in[i], boost_error_code);
        boost::optional<boost::asio::ip::address> optional_ipaddr;
        if (!boost_error_code)
        {
            optional_ipaddr = ipaddr;
        }
        ret.push_back(optional_ipaddr);
    }

    return ret;
}


static ccReg::CheckAvail
wrap_nsset_handle_check_result(
        const boost::optional<Epp::Nsset::NssetHandleRegistrationObstructionLocalized>& _obstruction)
{
    if (!_obstruction.is_initialized())
    {
        return ccReg::NotExist;
    }

    switch (_obstruction.get().state)
    {
        case Epp::Nsset::NssetHandleRegistrationObstruction::invalid_handle:
            return ccReg::BadFormat;

        case Epp::Nsset::NssetHandleRegistrationObstruction::protected_handle:
            return ccReg::DelPeriod; // strange but correct

        case Epp::Nsset::NssetHandleRegistrationObstruction::registered_handle:
            return ccReg::Exist;
    }

    throw std::runtime_error("unknown_nsset_state");
}


/**
 * integral types conversion with overflow detection to be replaced by CORBA wrappers
 */
template <class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE>
void
numeric_cast_by_ref(
        SOURCE_INTEGRAL_TYPE src,
        TARGET_INTEGRAL_TYPE& dst)
{
    typedef boost::integer_traits<SOURCE_INTEGRAL_TYPE> source_integral_type_traits;
    typedef boost::integer_traits<TARGET_INTEGRAL_TYPE> target_integral_type_traits;

    BOOST_MPL_ASSERT_MSG(
            source_integral_type_traits::is_integral,
            source_type_have_to_be_integral,
            (SOURCE_INTEGRAL_TYPE));
    BOOST_MPL_ASSERT_MSG(
            target_integral_type_traits::is_integral,
            target_type_have_to_be_integral,
            (TARGET_INTEGRAL_TYPE));
    dst = boost::numeric_cast<TARGET_INTEGRAL_TYPE>(src);
}


} // namespace {anonymous}


std::vector<std::string>
unwrap_ccreg_techcontacts_to_vector_string(const ccReg::TechContact& in)
{
    std::vector<std::string> ret;
    ret.reserve(in.length());
    for (unsigned long long i = 0; i < in.length(); ++i)
    {
        if (in[i] == 0)
        {
            throw std::runtime_error("null char ptr");
        }
        ret.push_back(std::string(in[i]));
    }

    return ret;
}


std::vector<Epp::Nsset::DnsHostInput>
unwrap_ccreg_dnshosts_to_vector_dnshosts(const ccReg::DNSHost& in)
{
    std::vector<Epp::Nsset::DnsHostInput> ret;
    ret.reserve(in.length());
    for (unsigned long long i = 0; i < in.length(); ++i)
    {
        if (in[i].fqdn == 0)
        {
            throw std::runtime_error("null char ptr");
        }
        ret.push_back(
                Epp::Nsset::DnsHostInput(
                        std::string(in[i].fqdn),
                        unwrap_inet_addr_to_vector_asio_addr(in[i].inet)));
    }

    return ret;
}


boost::optional<short>
unwrap_tech_check_level(CORBA::Short level)
{
    return level < 0
           ? boost::optional<short>()
           : boost::optional<short>(boost::numeric_cast<short>(level));
}


/**
 * @returns check results in the same order as input handles
 */
ccReg::CheckResp
wrap_localized_check_info(
        const std::vector<std::string>& nsset_handles,
        const std::map<std::string,
                boost::optional<Epp::Nsset::NssetHandleRegistrationObstructionLocalized> >& nsset_handle_check_results)
{
    ccReg::CheckResp result;
    result.length(nsset_handles.size());

    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator it = nsset_handles.begin();
         it != nsset_handles.end();
         ++it, ++i)
    {
        const boost::optional<Epp::Nsset::NssetHandleRegistrationObstructionLocalized> check_result = map_at(
                nsset_handle_check_results,
                *it);

        result[i].avail = wrap_nsset_handle_check_result(check_result);
        result[i].reason = Corba::wrap_string_to_corba_string(
                !check_result.is_initialized() ? "" : check_result.get().description);
    }

    return result;
}


ccReg::NSSet
wrap_localized_info_nsset(const Epp::Nsset::InfoNssetLocalizedOutputData& _input)
{
    ccReg::NSSet result;

    result.handle = wrap_string_to_corba_string(_input.handle);
    result.ROID = wrap_string_to_corba_string(_input.roid);
    result.ClID = wrap_string_to_corba_string(_input.sponsoring_registrar_handle);
    result.CrID = wrap_string_to_corba_string(_input.creating_registrar_handle);
    // XXX IDL nonsense
    result.UpID = wrap_string_to_corba_string(
            _input.last_update_registrar_handle.isnull() ? std::string() : _input.last_update_registrar_handle.get_value());
    wrap_Epp_ObjectStatesLocalized< ::Epp::Nsset::StatusValue>(_input.localized_external_states, result.stat);
    result.CrDate = wrap_boost_posix_time_ptime_to_string(_input.crdate);
    // XXX IDL nonsense
    result.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_input.last_update);
    // XXX IDL nonsense
    result.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_input.last_transfer);

    result.AuthInfoPw = Corba::wrap_string_to_corba_string(
            _input.authinfopw ? *_input.authinfopw : std::string());

    {
        result.dns.length(_input.dns_host.size());
        unsigned long i = 0;
        for (std::vector<Epp::Nsset::DnsHostOutput>::const_iterator it = _input.dns_host.begin();
             it != _input.dns_host.end();
             ++it, ++i)
        {
            result.dns[i].fqdn = wrap_string_to_corba_string(it->fqdn);

            result.dns[i].inet.length(it->inet_addr.size());
            unsigned long j = 0;
            for (std::vector<boost::asio::ip::address>::const_iterator ipit = it->inet_addr.begin();
                 ipit != it->inet_addr.end();
                 ++ipit, ++j)
            {
                result.dns[i].inet[j] = wrap_string_to_corba_string(ipit->to_string());
            }
        }
    }

    {
        result.tech.length(_input.tech_contacts.size());
        unsigned long i = 0;
        for (
            std::vector<std::string>::const_iterator it = _input.tech_contacts.begin();
            it != _input.tech_contacts.end();
            ++it, ++i)
        {
            result.tech[i] = wrap_string_to_corba_string(*it);
        }
    }

    // TODO replace with superseder of Corba::int_to_int template
    numeric_cast_by_ref(_input.tech_check_level, result.level);

    return result;
}


} // namespace LibFred::Corba
} // namespace Corba
