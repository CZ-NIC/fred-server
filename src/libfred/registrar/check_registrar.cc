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

#include <string>

#include <boost/regex.hpp>

#include "src/libfred/registrar/check_registrar.hh"
#include <boost/regex.hpp>

#include "src/libfred/opcontext.hh"
#include "src/libfred/db_settings.hh"


namespace LibFred
{
    CheckRegistrar::CheckRegistrar(const std::string& handle)
    : handle_(handle)
    {}
    bool CheckRegistrar::is_invalid_handle() const
    {
        static const boost::regex REGISTRAR_HANDLE_SYNTAX("[a-zA-Z0-9](-?[a-zA-Z0-9])*");
        if (handle_.length() < 3 || handle_.length() > 16)
        {
            return true;
        }
        return !boost::regex_match(handle_, REGISTRAR_HANDLE_SYNTAX);
    }

    std::string CheckRegistrar::to_string() const
    {
        return Util::format_operation_state("CheckRegistrar",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_)));
    }
}

