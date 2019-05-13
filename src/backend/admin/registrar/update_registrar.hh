/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef UPDATE_REGISTRAR_HH_4B763A0DDC124C54BEB826847EB50795
#define UPDATE_REGISTRAR_HH_4B763A0DDC124C54BEB826847EB50795

#include <boost/optional.hpp>
#include <exception>
#include <string>

namespace Admin {
namespace Registrar {

struct UpdateRegistrarException : std::exception
{
    const char* what() const noexcept override;
};

struct UpdateRegistrarNonexistent : std::exception
{
    const char* what() const noexcept override;
};

struct UpdateRegistrarInvalidVarSymb : std::exception
{
    const char* what() const noexcept override;
};

struct UpdateRegistrarInvalidHandle: std::exception
{
    const char* what() const noexcept override;
};

struct UpdateRegistrarNoUpdateData : std::exception
{
    const char* what() const noexcept override;
};

struct UpdateRegistrarInvalidCountryCode : std::exception
{
    const char* what() const noexcept override;
};

void update_registrar(unsigned long long _id,
                const boost::optional<std::string>& _handle,
                const boost::optional<std::string>& _name,
                const boost::optional<std::string>& _organization,
                const boost::optional<std::string>& _street1,
                const boost::optional<std::string>& _street2,
                const boost::optional<std::string>& _street3,
                const boost::optional<std::string>& _city,
                const boost::optional<std::string>& _state_or_province,
                const boost::optional<std::string>& _postal_code,
                const boost::optional<std::string>& _country,
                const boost::optional<std::string>& _telephone,
                const boost::optional<std::string>& _fax,
                const boost::optional<std::string>& _email,
                const boost::optional<std::string>& _url,
                boost::optional<bool> _system,
                const boost::optional<std::string>& _ico,
                const boost::optional<std::string>& _dic,
                const boost::optional<std::string>& _variable_symbol,
                const boost::optional<std::string>& _payment_memo_regex,
                boost::optional<bool> _vat_payer);

} // namespace Admin::Registrar
} // namespace Admin

#endif
