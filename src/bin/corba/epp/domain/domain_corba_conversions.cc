/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/bin/corba/epp/domain/domain_corba_conversions.hh"

#include "src/bin/corba/EPP.hh"

#include "src/bin/corba/epp/corba_conversions.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/backend/epp/domain/check_domain_localized.hh"
#include "src/backend/epp/domain/domain_registration_obstruction.hh"
#include "util/db/nullable.hh"
#include "util/map_at.hh"
#include "util/optional_value.hh"

#include <boost/optional.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace LibFred {
namespace Corba {
namespace Epp {
namespace Domain {

Optional<Nullable<std::string> >
unwrap_string_for_change_or_remove_to_Optional_Nullable_string(const char* _src)
{
    const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

    /* Defined by convention. Could be substituted by more explicit means in IDL interface
     * (like using _add and _rem elements, not just _chg for all operations). */
    static const char char_for_value_deleting = '\b';

    return unwrapped_src.empty()
                   ? Optional<Nullable<std::string> >() // do not set
                   : unwrapped_src.at(0) == char_for_value_deleting
                             ? Optional<Nullable<std::string> >(Nullable<std::string>()) // set NULL
                             : unwrapped_src;
}


namespace {


ccReg::CheckAvail
wrap_Epp_Domain_DomainLocalizedRegistrationObstruction(
        const boost::optional< ::Epp::Domain::DomainLocalizedRegistrationObstruction>& obstruction)
{
    if (!obstruction)
    {
        return ccReg::NotExist;
    }

    switch (obstruction.get().state)
    {
        case ::Epp::Domain::DomainRegistrationObstruction::registered:
            return ccReg::Exist;

        case ::Epp::Domain::DomainRegistrationObstruction::blacklisted:
            return ccReg::BlackList;

        case ::Epp::Domain::DomainRegistrationObstruction::zone_not_in_registry:
            return ccReg::NotApplicable;

        case ::Epp::Domain::DomainRegistrationObstruction::invalid_fqdn:
            return ccReg::BadFormat;
    }

    throw std::logic_error("Unexpected ::Epp::Domain::DomainRegistrationObstruction::Enum value.");
}


} // namespace LibFred::Corba::Epp::{anonymous}


/**
 * @returns check results in the same order as input handles
 */
ccReg::CheckResp
wrap_Epp_Domain_CheckDomainLocalizedResponse(
        const std::vector<std::string>& _domain_fqdns,
        const std::map<std::string,
                boost::optional< ::Epp::Domain::DomainLocalizedRegistrationObstruction> >& _domain_fqdn_to_domain_localized_registration_obstruction)
{
    ccReg::CheckResp result;
    result.length(_domain_fqdns.size());

    CORBA::ULong result_idx = 0;
    for (std::vector<std::string>::const_iterator domain_fqdn_ptr = _domain_fqdns.begin();
         domain_fqdn_ptr != _domain_fqdns.end();
         ++domain_fqdn_ptr, ++result_idx)
    {
        const boost::optional< ::Epp::Domain::DomainLocalizedRegistrationObstruction>
                domain_localized_registration_obstruction =
                        map_at(_domain_fqdn_to_domain_localized_registration_obstruction, *domain_fqdn_ptr);

        result[result_idx].avail =
                wrap_Epp_Domain_DomainLocalizedRegistrationObstruction(domain_localized_registration_obstruction);

        result[result_idx].reason =
                LibFred::Corba::wrap_string_to_corba_string(
                        domain_localized_registration_obstruction
                                ? domain_localized_registration_obstruction.get().description
                                : "");
    }

    return result;
}

namespace {

void
wrap_Epp_Domain_EnumValidationExtension_to_ccReg_ExtensionList(
        const Nullable< ::Epp::Domain::EnumValidationExtension>& _src,
        ccReg::ExtensionList& _dst)
{
    if (!_src.isnull())
    {
        ccReg::ENUMValidationExtension_var enumVal = new ccReg::ENUMValidationExtension();
        enumVal->valExDate = LibFred::Corba::wrap_string_to_corba_string(
                boost::gregorian::to_iso_extended_string(_src.get_value().get_valexdate()));
        enumVal->publish = _src.get_value().get_publish();
        _dst.length(1);
        _dst[0] <<= enumVal._retn();
    }
}

}//namespace LibFred::Corba::{anonymous}

void
wrap_Epp_Domain_InfoDomainLocalizedOutputData(
        const ::Epp::Domain::InfoDomainLocalizedOutputData& _src,
        ccReg::Domain& _dst)
{
    _dst.ROID = LibFred::Corba::wrap_string_to_corba_string(_src.roid);
    _dst.name = LibFred::Corba::wrap_string_to_corba_string(_src.fqdn);

    _dst.Registrant = LibFred::Corba::wrap_string_to_corba_string(_src.registrant);
    _dst.nsset = LibFred::Corba::wrap_string_to_corba_string(_src.nsset.get_value_or_default());
    _dst.keyset = LibFred::Corba::wrap_string_to_corba_string(_src.keyset.get_value_or_default());

    wrap_Epp_ObjectStatesLocalized< ::Epp::Domain::StatusValue>(_src.localized_external_states, _dst.stat);

    _dst.ClID = LibFred::Corba::wrap_string_to_corba_string(_src.sponsoring_registrar_handle);
    _dst.CrID = LibFred::Corba::wrap_string_to_corba_string(_src.creating_registrar_handle);
    _dst.UpID = LibFred::Corba::wrap_string_to_corba_string(_src.last_update_registrar_handle.get_value_or_default());

    _dst.CrDate = wrap_boost_posix_time_ptime_to_string(_src.crdate);
    _dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_update);
    _dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_transfer);

    _dst.ExDate = LibFred::Corba::wrap_string_to_corba_string(boost::gregorian::to_iso_extended_string(_src.exdate));

    _dst.AuthInfoPw = LibFred::Corba::wrap_string_to_corba_string(
            _src.authinfopw ? *_src.authinfopw : std::string());

    _dst.admin.length(_src.admin.size());
    unsigned long dst_admin_index = 0;
    for (std::set<std::string>::const_iterator admin_ptr = _src.admin.begin();
         admin_ptr != _src.admin.end();
         ++admin_ptr, ++dst_admin_index)
    {
        _dst.admin[dst_admin_index] = LibFred::Corba::wrap_string_to_corba_string(*admin_ptr);
    }

    wrap_Epp_Domain_EnumValidationExtension_to_ccReg_ExtensionList(
            _src.ext_enum_domain_validation,
            _dst.ext);

    _dst.tmpcontact.length(_src.tmpcontact.size());
    unsigned long dst_tmpcontact_index = 0;
    for (std::set<std::string>::const_iterator tmpcontact_ptr = _src.tmpcontact.begin();
         tmpcontact_ptr != _src.tmpcontact.end();
         ++tmpcontact_ptr, ++dst_tmpcontact_index)
    {
        _dst.tmpcontact[dst_tmpcontact_index] = LibFred::Corba::wrap_string_to_corba_string(*tmpcontact_ptr);
    }
}


::Epp::Domain::DomainRegistrationTime
unwrap_domain_registration_period(const ccReg::Period_str& period)
{
    switch (period.unit)
    {
        case ccReg::unit_month:
            return ::Epp::Domain::DomainRegistrationTime(
                    period.count,
                    ::Epp::Domain::DomainRegistrationTime::Unit::month);

        case ccReg::unit_year:
            return ::Epp::Domain::DomainRegistrationTime(
                    period.count,
                    ::Epp::Domain::DomainRegistrationTime::Unit::year);
    }
    throw std::runtime_error("unwrap_domain_registration_period internal error");
}


std::vector<std::string>
unwrap_ccreg_admincontacts_to_vector_string(const ccReg::AdminContact& in)
{
    std::vector<std::string> ret;
    ret.reserve(in.length());
    for (unsigned long long i = 0; i < in.length(); ++i)
    {
        if (in[i] == nullptr)
        {
            throw std::runtime_error("null char ptr");
        }
        ret.push_back(std::string(in[i]));
    }

    return ret;
}


boost::optional<::Epp::Domain::EnumValidationExtension>
unwrap_enum_validation_extension_list(const ccReg::ExtensionList& ext)
{
    if (ext.length() == 0)
    {
        return boost::optional<::Epp::Domain::EnumValidationExtension>();
    }

    if (ext.length() > 1)
    {
        throw std::runtime_error("ext too long");
    }

    const ccReg::ENUMValidationExtension* enum_ext = nullptr;

    const bool is_ENUMValidationExtension = (ext[0] >>= enum_ext);

    if (!is_ENUMValidationExtension)
    {
        throw std::runtime_error("unknown extension found when extracting domain enum extension");
    }

    const std::string temp_valexdate_string = LibFred::Corba::unwrap_string_from_const_char_ptr(
            enum_ext->valExDate);
    boost::gregorian::date valexdate;
    try
    {
        valexdate = boost::gregorian::from_simple_string(temp_valexdate_string);
    }
    catch (...)
    {
        // conversion errors ignored here, implementation must later handle `not-a-date-time` value correctly
    }

    return ::Epp::Domain::EnumValidationExtension(valexdate, enum_ext->publish);
}


} // namespace LibFred::Corba::Epp::Domain
} // namespace LibFred::Corba::Epp
} // namespace LibFred::Corba
} // namespace LibFred
