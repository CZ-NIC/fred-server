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

#ifndef SRC_CORBA_EPP_EPP_LEGACY_COMPATIBILITY_H_7876341150341530
#define SRC_CORBA_EPP_EPP_LEGACY_COMPATIBILITY_H_7876341150341530

#include "src/epp/session_lang.h"
#include "src/corba/epp/epp_session.h"
#include "util/optional_value.h"

namespace Legacy {

inline Epp::SessionLang::Enum get_lang(EppSessionContainer& epp_sessions, unsigned long long session_id) {
    // TODO kvuli kompatibilite s epp_session.h musi byt cz = 1
    const int result = epp_sessions.get_registrar_lang(session_id);
    return result == 1
        ?   Epp::SessionLang::cz
        :   Epp::SessionLang::en;
}

inline unsigned long long get_registrar_id(EppSessionContainer& epp_sessions, unsigned long long session_id) {

    const unsigned long long result = epp_sessions.get_registrar_id(session_id);

    if(result == 0) {
        throw std::runtime_error("unable to get session registrar id");
    }
    return result;
}

inline Optional<bool> convert_update_discloseflag(char disclose_value) {

    switch(disclose_value) {
        case 't' : return true;
        case 'f' : return false;
        case ' ' : return Optional<bool>();
    }

    throw std::runtime_error("invalid disclose value");
}

inline bool DefaultPolicy() { return true; }

inline bool setvalue_DISCLOSE(bool d, ccReg::Disclose flag) {

    switch (flag) {
        case ccReg::DISCL_DISPLAY:
            if( DefaultPolicy() ) { return true; } else { return d; }

        case ccReg::DISCL_HIDE:
            if( !DefaultPolicy() ) { return true; } else { return !d; }

        case ccReg::DISCL_EMPTY:
            return DefaultPolicy();
    }

    return false;
}

inline bool get_DISCLOSE(bool db) {
    if( DefaultPolicy() ) {
        return !db;
    } else {
        return db;
    }
}

inline char update_DISCLOSE(bool d, ccReg::Disclose flag) {

    if (flag == ccReg::DISCL_EMPTY) {
        return ' ';

    } else {
        if (setvalue_DISCLOSE(d, flag) ) {
            return 't';
        } else {
            return 'f';
        }
    }

}

}

#endif
