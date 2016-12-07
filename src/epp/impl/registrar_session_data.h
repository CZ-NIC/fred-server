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

/**
 *  @file
 */

#ifndef CORBA_EPP_REGISTRAR_SESSION_DATA_54154540219
#define CORBA_EPP_REGISTRAR_SESSION_DATA_54154540219

#include "src/epp/impl/session_lang.h"

namespace Epp {

struct RegistrarSessionData {
    unsigned long long registrar_id;
    SessionLang::Enum language;

    RegistrarSessionData(
        unsigned long long _registrar_id,
        SessionLang::Enum _language
    ) :
        registrar_id(_registrar_id),
        language(_language)
    { }
};

}

#endif
