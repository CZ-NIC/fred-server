/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_KEYSET_HH_6E1697BC812E4AEA9613DC71E2814CEF
#define CREATE_KEYSET_HH_6E1697BC812E4AEA9613DC71E2814CEF

#include "src/backend/epp/keyset/create_keyset_config_data.hh"
#include "src/backend/epp/keyset/create_keyset_input_data.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Keyset {

struct CreateKeysetResult
{
    unsigned long long id;
    unsigned long long create_history_id;
    boost::posix_time::ptime crdate;
};

CreateKeysetResult create_keyset(
        LibFred::OperationContext& _ctx,
        const CreateKeysetInputData& _create_keyset_input_data,
        const CreateKeysetConfigData& _create_keyset_config_data,
        const SessionData& _session_data);

} // namespace Epp::Keyset
} // namespace Epp

#endif
