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
 *  registrar check
 */

#ifndef CHECK_REGISTRAR_H
#define CHECK_REGISTRAR_H

#include <string>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/printable.h"

namespace Fred
{
    /**
    * Checking of registrar properties.
    */
    class CheckRegistrar : public Util::Printable
    {
        const std::string handle_;/**< registrar identifier */
    public:

        /**
        * Check registrar ctor.
        * @param handle a registrar identifier.
        */
        CheckRegistrar(const std::string& handle);

        /**
        * Check registrar handle syntax.
        * @return true if invalid, false if ok
        */
        bool is_invalid_handle() const;

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

}//namespace Fred

#endif
