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

#include "src/corba/epp/contact/contact_corba_conversions.h"

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/util/corba_conversions_int.h"
#include "src/corba/util/corba_conversions_string.h"
#include "util/corba_conversion.h"
#include "util/db/nullable.h"
#include "util/map_at.h"
#include "util/optional_value.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace Fred {
namespace Corba {
namespace {

bool is_contact_change_string_meaning_to_delete(const char* value)
{
    return value[0] == '\b';
}


bool is_contact_change_string_meaning_not_to_touch(const char* value)
{
    return value[0] == '\0';
}


boost::optional<Nullable<std::string> > convert_contact_update_or_delete_string(const char* _src)
{
    const bool src_has_special_meaning_to_delete = is_contact_change_string_meaning_to_delete(_src);
    if (src_has_special_meaning_to_delete)
    {
        return Nullable<std::string>();
    }
    const bool src_has_special_meaning_not_to_touch = is_contact_change_string_meaning_not_to_touch(_src);
    if (src_has_special_meaning_not_to_touch)
    {
        return boost::optional<Nullable<std::string> >();
    }
    const std::string value_to_set = Fred::Corba::unwrap_string(_src);
    const bool value_to_set_means_not_to_touch = value_to_set.empty();
    if (value_to_set_means_not_to_touch)
    {
        return boost::optional<Nullable<std::string> >();
    }

    return Nullable<std::string>(value_to_set);
}


boost::optional<std::string> convert_contact_update_string(const char* _src)
{
    const bool src_has_special_meaning_not_to_touch = is_contact_change_string_meaning_not_to_touch(_src);
    if (src_has_special_meaning_not_to_touch)
    {
        return boost::optional<std::string>();
    }
    const std::string value_to_set = Fred::Corba::unwrap_string(_src);
    const bool value_to_set_means_not_to_touch = value_to_set.empty();

    return value_to_set_means_not_to_touch ? boost::optional<std::string>()
           : value_to_set;
}


Epp::Contact::ContactDisclose convert_ContactChange_to_ContactDisclose(
        const ccReg::ContactChange& _src,
        Epp::Contact::ContactDisclose::Flag::Enum _meaning)
{
    Epp::Contact::ContactDisclose result(_meaning);
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseName))
    {
        result.add<Epp::Contact::ContactDisclose::Item::name>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseOrganization))
    {
        result.add<Epp::Contact::ContactDisclose::Item::organization>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseAddress))
    {
        result.add<Epp::Contact::ContactDisclose::Item::address>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseTelephone))
    {
        result.add<Epp::Contact::ContactDisclose::Item::telephone>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseFax))
    {
        result.add<Epp::Contact::ContactDisclose::Item::fax>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseEmail))
    {
        result.add<Epp::Contact::ContactDisclose::Item::email>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseVAT))
    {
        result.add<Epp::Contact::ContactDisclose::Item::vat>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseIdent))
    {
        result.add<Epp::Contact::ContactDisclose::Item::ident>();
    }
    if (Fred::Corba::wrap_int<bool>(_src.DiscloseNotifyEmail))
    {
        result.add<Epp::Contact::ContactDisclose::Item::notify_email>();
    }

    return result;
}


boost::optional<Epp::Contact::ContactDisclose> unwrap_ContactChange_to_ContactDisclose(
        const ccReg::ContactChange& _src)
{
    switch (_src.DiscloseFlag)
    {
        case ccReg::DISCL_EMPTY:
            return boost::optional<Epp::Contact::ContactDisclose>();

        case ccReg::DISCL_HIDE:
            return convert_ContactChange_to_ContactDisclose(_src, Epp::Contact::ContactDisclose::Flag::hide);

        case ccReg::DISCL_DISPLAY:
            return convert_ContactChange_to_ContactDisclose(
                _src,
                Epp::Contact::ContactDisclose::Flag::disclose);
    }

    throw std::runtime_error("Invalid DiscloseFlag value;");
}


Nullable<Epp::Contact::ContactChange::IdentType::Enum> unwrap_identtyp(ccReg::identtyp type)
{
    switch (type)
    {
        case ccReg::EMPTY:
            return Nullable<Epp::Contact::ContactChange::IdentType::Enum>();

        case ccReg::OP:
            return Epp::Contact::ContactChange::IdentType::op;

        case ccReg::PASS:
            return Epp::Contact::ContactChange::IdentType::pass;

        case ccReg::ICO:
            return Epp::Contact::ContactChange::IdentType::ico;

        case ccReg::MPSV:
            return Epp::Contact::ContactChange::IdentType::mpsv;

        case ccReg::BIRTHDAY:
            return Epp::Contact::ContactChange::IdentType::birthday;
    }

    throw std::runtime_error("Invalid identtyp value.");
}


} // namespace Fred::Corba::{anonymous}


void unwrap_ContactChange(
        const ccReg::ContactChange& _src,
        Epp::Contact::ContactChange& _dst)
{
    _dst.name              = convert_contact_update_or_delete_string(_src.Name);
    _dst.organization      = convert_contact_update_or_delete_string(_src.Organization);
    for (unsigned idx = 0; idx < _src.Streets.length(); ++idx)
    {
        _dst.streets.push_back(convert_contact_update_or_delete_string(_src.Streets[idx]));
    }
    _dst.city              = convert_contact_update_or_delete_string(_src.City);
    _dst.state_or_province = convert_contact_update_or_delete_string(_src.StateOrProvince);
    _dst.postal_code       = convert_contact_update_or_delete_string(_src.PostalCode);
    _dst.country_code      = convert_contact_update_string(_src.CC);
    _dst.telephone         = convert_contact_update_or_delete_string(_src.Telephone);
    _dst.fax               = convert_contact_update_or_delete_string(_src.Fax);
    _dst.email             = convert_contact_update_or_delete_string(_src.Email);
    _dst.notify_email      = convert_contact_update_or_delete_string(_src.NotifyEmail);
    _dst.vat               = convert_contact_update_or_delete_string(_src.VAT);
    _dst.ident             = convert_contact_update_or_delete_string(_src.ident);
    _dst.ident_type        = unwrap_identtyp(_src.identtype);
    _dst.authinfopw        = convert_contact_update_or_delete_string(_src.AuthInfoPw);
    _dst.disclose          = unwrap_ContactChange_to_ContactDisclose(_src);
}


namespace {


static ccReg::CheckAvail wrap_contact_handle_check_result(
        const boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized>& _obstruction)
{
    if (!_obstruction.is_initialized())
    {
        return ccReg::NotExist;
    }

    switch (_obstruction.get().state)
    {
        case Epp::Contact::ContactHandleRegistrationObstruction::invalid_handle:
            return ccReg::BadFormat;

        case Epp::Contact::ContactHandleRegistrationObstruction::protected_handle:
            return ccReg::DelPeriod; // strange but correct

        case Epp::Contact::ContactHandleRegistrationObstruction::registered_handle:
            return ccReg::Exist;
    }

    throw std::runtime_error("unknown_contact_state");
}


ccReg::identtyp wrap_identtyp(
        const Nullable<Epp::Contact::InfoContactLocalizedOutputData::IdentType::Enum>& type)
{
    if (type.isnull())
    {
        return ccReg::EMPTY;
    }

    switch (type.get_value())
    {
        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::op:
            return ccReg::OP;

        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::pass:
            return ccReg::PASS;

        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::ico:
            return ccReg::ICO;

        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::mpsv:
            return ccReg::MPSV;

        case Epp::Contact::InfoContactLocalizedOutputData::IdentType::birthday:
            return ccReg::BIRTHDAY;
    }

    throw std::runtime_error("Invalid Epp::Contact::InfoContactLocalizedOutputData::IdentType::Enum value.");
}


} // namespace Fred::Corba::{anonymous}


/**
 * @returns check results in the same order as input handles
 */
ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map<std::string,
                boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized> >& contact_handle_check_results)
{
    ccReg::CheckResp result;
    result.length(contact_handles.size());

    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator it = contact_handles.begin();
         it != contact_handles.end();
         ++it, ++i)
    {
        const boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized> check_result =
                map_at(contact_handle_check_results, *it);

        result[i].avail = wrap_contact_handle_check_result(check_result);
        result[i].reason = wrap_string_to_corba_string(
                check_result.is_initialized() ? check_result.get().description : "");
    }

    return result;
}


namespace {


template <Epp::Contact::ContactDisclose::Item::Enum ITEM>
CORBA::Boolean presents(const Epp::Contact::ContactDisclose& _src)
{
    return wrap_int<CORBA::Boolean>(_src.presents<ITEM>());
}


} // namespace Fred::Corba::{anonymous}


void wrap_InfoContactLocalizedOutputData(
        const Epp::Contact::InfoContactLocalizedOutputData& _src,
        ccReg::Contact& _dst)
{
    _dst.handle = wrap_string_to_corba_string(_src.handle);
    _dst.ROID = wrap_string_to_corba_string(_src.roid);
    _dst.ClID = wrap_string_to_corba_string(_src.sponsoring_registrar_handle);
    _dst.CrID = wrap_string_to_corba_string(_src.creating_registrar_handle);
    // XXX IDL nonsense
    _dst.UpID = wrap_Nullable_string_to_string(_src.last_update_registrar_handle);
    wrap_Epp_ObjectStatesLocalized< ::Epp::Contact::StatusValue>(_src.localized_external_states, _dst.stat);
    _dst.CrDate = wrap_boost_posix_time_ptime_to_string(_src.crdate);
    // XXX IDL nonsense
    _dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_update);
    // XXX IDL nonsense
    _dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_transfer);
    _dst.Name = wrap_Nullable_string_to_string(_src.name);
    _dst.Organization = wrap_Nullable_string_to_string(_src.organization);

    const unsigned number_of_streets =
            !_src.street3.isnull() && !_src.street3.get_value().empty()
                    ? 3
                    : !_src.street2.isnull() && !_src.street2.get_value().empty()
                              ? 2
                              : !_src.street1.isnull() && !_src.street1.get_value().empty()
                                        ? 1
                                        : 0;
    _dst.Streets.length(number_of_streets);
    if (0 < number_of_streets)
    {
        _dst.Streets[0] = wrap_Nullable_string_to_string(_src.street1);
    }
    if (1 < number_of_streets)
    {
        _dst.Streets[1] = wrap_Nullable_string_to_string(_src.street2);
    }
    if (2 < number_of_streets)
    {
        _dst.Streets[2] = wrap_Nullable_string_to_string(_src.street3);
    }

    _dst.City            = wrap_Nullable_string_to_string(_src.city);
    _dst.StateOrProvince = wrap_Nullable_string_to_string(_src.state_or_province);
    _dst.PostalCode      = wrap_Nullable_string_to_string(_src.postal_code);
    _dst.CountryCode     = wrap_Nullable_string_to_string(_src.country_code);
    _dst.Telephone       = wrap_Nullable_string_to_string(_src.telephone);
    _dst.Fax             = wrap_Nullable_string_to_string(_src.fax);
    _dst.Email           = wrap_Nullable_string_to_string(_src.email);
    _dst.NotifyEmail     = wrap_Nullable_string_to_string(_src.notify_email);
    _dst.VAT             = wrap_Nullable_string_to_string(_src.VAT);
    _dst.ident           = wrap_Nullable_string_to_string(_src.ident);
    _dst.identtype       = wrap_identtyp(_src.identtype);
    _dst.AuthInfoPw =
            Fred::Corba::wrap_string_to_corba_string(_src.authinfopw ? _src.authinfopw.value() : std::string());

    if (!_src.disclose.is_initialized())
    {
        _dst.DiscloseFlag           = ccReg::DISCL_EMPTY;
        _dst.DiscloseName           = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseOrganization   = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseAddress        = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseTelephone      = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseFax            = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseEmail          = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseVAT            = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseIdent          = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseNotifyEmail    = wrap_int<CORBA::Boolean>(false);
    }
    else
    {
        _dst.DiscloseFlag         = _src.disclose->does_present_item_mean_to_disclose()
                                           ? ccReg::DISCL_DISPLAY
                                           : ccReg::DISCL_HIDE;
        _dst.DiscloseName         = presents<Epp::Contact::ContactDisclose::Item::name>(*_src.disclose);
        _dst.DiscloseOrganization = presents<Epp::Contact::ContactDisclose::Item::organization>(*_src.disclose);
        _dst.DiscloseAddress      = presents<Epp::Contact::ContactDisclose::Item::address>(*_src.disclose);
        _dst.DiscloseTelephone    = presents<Epp::Contact::ContactDisclose::Item::telephone>(*_src.disclose);
        _dst.DiscloseFax          = presents<Epp::Contact::ContactDisclose::Item::fax>(*_src.disclose);
        _dst.DiscloseEmail        = presents<Epp::Contact::ContactDisclose::Item::email>(*_src.disclose);
        _dst.DiscloseVAT          = presents<Epp::Contact::ContactDisclose::Item::vat>(*_src.disclose);
        _dst.DiscloseIdent        = presents<Epp::Contact::ContactDisclose::Item::ident>(*_src.disclose);
        _dst.DiscloseNotifyEmail  = presents<Epp::Contact::ContactDisclose::Item::notify_email>(*_src.disclose);
    }
}


} // namespace Fred::Corba
} // namespace Fred
