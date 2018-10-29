/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_REGISTRAR_HH_751FDCF5F35F4B7F85687CE87E36F682
#define CREATE_REGISTRAR_HH_751FDCF5F35F4B7F85687CE87E36F682

#include <boost/optional.hpp>
#include <exception>
#include <string>

namespace Admin {
namespace Registrar {

struct CreateRegistrarException : std::exception
{
    const char* what() const noexcept override;
};

struct RegistrarAlreadyExists : std::exception
{
    const char* what() const noexcept override;
};

unsigned long long create_registrar(const std::string& _handle,
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
