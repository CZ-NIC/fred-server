/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  enum testsuite names
 *  There can be another testsuites as well but content of this enum is mandatory and should be always present in database.
 */

#ifndef CONTACT_VERIFICATION_TESTSUITE_HANDLE_2133867786_
#define CONTACT_VERIFICATION_TESTSUITE_HANDLE_2133867786_

namespace Fred
{
    /**
     * Available testsuites defined by their handles.
     * Should be in sync with enum_contact_testsuite.handle in db.
     */
    namespace TestsuiteHandle
    {
        const std::string AUTOMATIC     = "automatic";
        const std::string MANUAL        = "manual";
        const std::string THANK_YOU     = "thank_you";
    }
}
#endif // #include guard end

