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

#ifndef SRC_EPP_SESSION_LANG_505343440
#define SRC_EPP_SESSION_LANG_505343440

#include "src/epp/impl/exception.h"

#include <string>

namespace Epp {

struct SessionLang
{
    enum Enum
    {
        en,
        cs
    };

    /**
     * @throws Epp::InvalidSessionLang
     */
    static std::string to_db_handle(Epp::SessionLang::Enum _lang)
    {
        switch (_lang)
        {
            case Epp::SessionLang::en: return "en";
            case Epp::SessionLang::cs: return "cs";
        }
        throw Epp::InvalidSessionLang();
    }
};

} // namespace Epp

#endif
