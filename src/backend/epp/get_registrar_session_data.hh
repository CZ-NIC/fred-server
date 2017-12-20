/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef GET_REGISTRAR_SESSION_DATA_H_B7B802E9C549405F85FE4DE10A393EAD
#define GET_REGISTRAR_SESSION_DATA_H_B7B802E9C549405F85FE4DE10A393EAD

#include "src/bin/corba/epp/epp_session.hh"
#include "src/bin/corba/epp/epp_legacy_compatibility.hh"
#include "src/backend/epp/registrar_session_data.hh"

namespace Epp {

inline RegistrarSessionData get_registrar_session_data(EppSessionContainer& _epp_sessions, unsigned long long _session_id)
{
    return RegistrarSessionData(
            Legacy::get_registrar_id(_epp_sessions, _session_id),
            Legacy::get_lang(_epp_sessions, _session_id));
}

}

#endif
