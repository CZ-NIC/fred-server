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

/**
 *  @file
 */

#ifndef EPP_LEGACY_COMPATIBILITY_H_2E719054B59248C196A1A2D17DF40DF9
#define EPP_LEGACY_COMPATIBILITY_H_2E719054B59248C196A1A2D17DF40DF9

#include "src/epp/impl/session_lang.h"
#include "src/corba/epp/epp_session.h"
#include "util/optional_value.h"

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
