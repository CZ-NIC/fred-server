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
 *  get registar handles
 */

#ifndef GET_REGISTRAR_HANDLES_H_
#define GET_REGISTRAR_HANDLES_H_

#include <vector>
#include <string>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"

namespace Fred
{
namespace Registrar
{

    /**
    * Gets list of registar handles.
    * Except excluded registar handles with optional locking and optional dedicated db connection.
    */
    class GetRegistrarHandles
    {
        std::vector<std::string> excluded_registrar_handles_;/**< registrar handles excluded from the list returned by exec method */
        bool read_lock_;/**< lock registrar row for read */
    public:
        /**
        * GetRegistrarHandles constructor
        */
        GetRegistrarHandles();

        /**
        * Sets list of registrar handles which has to be omitted in result list
        * @param exclude_registrars sets excluded registrar handles into @ref excluded_registrar_handles_ attribute
        */
        GetRegistrarHandles& set_exclude_registrars(const std::vector<std::string>& _exclude_registrars);

        /**
        * Sets registrar read lock flag, effective only with shared db connection.
        * @param lock sets registrar read lock flag into @ref read_lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        GetRegistrarHandles& set_read_lock(bool read_lock = true);

        /**
        * Executes getting list of registrar handles
        * @param ctx contains reference to database and logging interface
        * @return list of registrar handles except excluded, throws if excluded registrar handle doesn't exist
        */
        std::vector<std::string> exec(OperationContext& ctx);

        /**
        * Executes getting list of registrar handles using it's own db connection.
        * @return list of registrar handles except excluded, throws if excluded registrar handle doesn't exist
        */
        std::vector<std::string> exec();
    };

}//namespace Registrar
}//namespace Fred
#endif //GET_REGISTRAR_HANDLES_H_
