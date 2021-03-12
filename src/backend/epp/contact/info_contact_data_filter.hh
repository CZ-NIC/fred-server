/*
 * Copyright (C) 2021  CZ.NIC, z. s. p. o.
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

#ifndef INFO_CONTACT_DATA_FILTER_HH_53E0BB635FD48C9637BBC7A03CD00CDD//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define INFO_CONTACT_DATA_FILTER_HH_53E0BB635FD48C9637BBC7A03CD00CDD

#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"

#include "src/backend/epp/session_data.hh"

#include <boost/optional.hpp>

#include <string>

namespace Epp {
namespace Contact {

class InfoContactDataFilter
{
public:
    struct AuthenticationFailure { };
    virtual ~InfoContactDataFilter() { }
    virtual LibFred::InfoContactData& operator()(
            LibFred::OperationContext& ctx,
            const boost::optional<std::string>& contact_authinfopw,
            const SessionData& session_data,
            LibFred::InfoContactData& contact_data) const = 0;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//INFO_CONTACT_DATA_FILTER_HH_53E0BB635FD48C9637BBC7A03CD00CDD
