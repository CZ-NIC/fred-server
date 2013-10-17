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
 *  registar impl
 */

#ifndef REGISTRAR_IMPL_H_
#define REGISTRAR_IMPL_H_

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
namespace Fred
{
namespace Registrar
{

    /**
    * Gets registar database id by registrar handle and locks registrar for read.
    * @param EXCEPTION is type of exception used for reporting when registrar is not found, deducible from type of @ref ex_ptr parameter
    * @param EXCEPTION_SETTER is EXCEPTION member function pointer used to report unknown registrar_handle
    * @param ctx contains reference to database and logging interface
    * @param registrar_handle registrar handle to look for
    * @param ex_ptr is  pointer to given exception instance to be set (don't throw), if ex_ptr is 0, new exception instance is created, set and thrown
    * @param ex_setter is EXCEPTION member function pointer used to report unknown registrar_handle
    * @return database id of registrar
    * , or throw @ref EXCEPTION if registrar was not found and external exception instance was not provided
    * , or set unknown registrar_handle into given external exception instance and return 0
    * , or throw InternalError or some other exception in case of failure.
    */
    template <class EXCEPTION, typename EXCEPTION_SETTER>
    unsigned long long get_registrar_id_by_handle(OperationContext& ctx, const std::string& registrar_handle
            , EXCEPTION* ex_ptr, EXCEPTION_SETTER ex_setter)
    {//check registrar
        Database::Result registrar_res = ctx.get_conn().exec_params(
            "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
            , Database::query_param_list(registrar_handle));
        if(registrar_res.size() == 0)//registrar not found
        {
            if(ex_ptr == 0)//make new exception instance, set data and throw
            {
                BOOST_THROW_EXCEPTION((EXCEPTION().*ex_setter)(registrar_handle));
            }
            else//set unknown registrar handle to given exception instance (don't throw) and return 0
            {
                (ex_ptr->*ex_setter)(registrar_handle);
                return 0;
            }
        }
        if (registrar_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
        }

        return static_cast<unsigned long long>(registrar_res[0][0]);//return id
    }



}//namespace Registrar
}//namespace Fred
#endif //REGISTRAR_IMPL_H_
