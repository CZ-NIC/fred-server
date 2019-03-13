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
#ifndef REGISTRAR_SESSION_DATA_HH_297112DD79744E299FBDD4D7481C5C17
#define REGISTRAR_SESSION_DATA_HH_297112DD79744E299FBDD4D7481C5C17

#include "src/backend/epp/session_lang.hh"

namespace Epp {

struct RegistrarSessionData
{
    unsigned long long registrar_id;
    SessionLang::Enum language;


    RegistrarSessionData(
            unsigned long long _registrar_id,
            SessionLang::Enum _language)
        : registrar_id(_registrar_id),
          language(_language)
    {
    }


};

} // namespace Epp

#endif
