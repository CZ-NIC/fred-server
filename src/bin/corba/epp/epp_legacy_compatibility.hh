/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 */

#ifndef EPP_LEGACY_COMPATIBILITY_HH_9F1CAD37EAEB4C84BABD679CC0684A1B
#define EPP_LEGACY_COMPATIBILITY_HH_9F1CAD37EAEB4C84BABD679CC0684A1B

#include "src/backend/epp/session_lang.hh"
#include "src/bin/corba/epp/epp_session.hh"
#include "util/optional_value.hh"

namespace Legacy {

inline Epp::SessionLang::Enum get_lang(EppSessionContainer& epp_sessions, unsigned long long session_id) {
    // TODO kvuli kompatibilite s epp_session.h musi byt cz = 1
    const int result = epp_sessions.get_registrar_lang(session_id);
    return result == 1
        ?   Epp::SessionLang::cs
        :   Epp::SessionLang::en;
}

inline unsigned long long get_registrar_id(EppSessionContainer& epp_sessions, unsigned long long session_id) {

    const unsigned long long result = epp_sessions.get_registrar_id(session_id);

    return result;
}

}

#endif
