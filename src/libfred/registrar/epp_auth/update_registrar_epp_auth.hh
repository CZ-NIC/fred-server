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

#ifndef UPDATE_REGISTRAR_EPP_AUTH_HH_4D17531385D14877B071FDC2D35425A0
#define UPDATE_REGISTRAR_EPP_AUTH_HH_4D17531385D14877B071FDC2D35425A0

#include "src/libfred/opcontext.hh"

#include <boost/optional.hpp>
#include <string>

namespace LibFred {
namespace Registrar {
namespace EppAuth {

class UpdateRegistrarEppAuth
{
public:
    explicit UpdateRegistrarEppAuth(unsigned long long _id);

    UpdateRegistrarEppAuth& set_certificate_fingerprint(
            const boost::optional<std::string>& _certificate_fingerprint);

    UpdateRegistrarEppAuth& set_plain_password(const boost::optional<std::string>& _plain_password);

    void exec(OperationContext& _ctx) const;

private:
    unsigned long long id_;
    boost::optional<std::string> certificate_fingerprint_;
    boost::optional<std::string> plain_password_;
};

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
