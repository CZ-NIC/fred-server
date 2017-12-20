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

#include <vector>
#include <string>
#include <sstream>

#include "src/libfred/opexception.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/util.hh"

#include "src/libfred/registrar/get_registrar_handles.hh"

namespace LibFred
{
namespace Registrar
{

    GetRegistrarHandles::GetRegistrarHandles()
    : excluded_registrar_handles_()
    , read_lock_(false)
    {}

    GetRegistrarHandles& GetRegistrarHandles::set_exclude_registrars(const std::vector<std::string>& _exclude_registrars)
    {
        excluded_registrar_handles_ = _exclude_registrars;
        return *this;
    }

    GetRegistrarHandles& GetRegistrarHandles::set_read_lock(bool read_lock)
    {
        read_lock_ = read_lock;
        return *this;
    }

    std::vector<std::string> GetRegistrarHandles::exec(OperationContext& ctx)
    {
        //check if excluded registrar handles exists
        {
            Database::QueryParams params;
            params.reserve(excluded_registrar_handles_.size());

            Util::HeadSeparator array_separator ("",",");
            std::ostringstream sql;
            sql << "SELECT handle FROM unnest(array[";

            for(std::vector<std::string>::const_iterator ci = excluded_registrar_handles_.begin()
                        ; ci != excluded_registrar_handles_.end(); ++ci)
            {
                params.push_back(*ci);
                sql << array_separator.get() << "$" <<params.size() << "::text ";
            }
            sql <<"]::text[]) as excluded_registrar(handle)"
            " WHERE NOT EXISTS (SELECT handle FROM registrar r WHERE r.handle = excluded_registrar.handle)";

            Database::Result not_existing_excluded_registrar_handles_res = ctx.get_conn().exec_params(sql.str(),params);

            if(not_existing_excluded_registrar_handles_res.size() > 0)
            {
                std::string not_existing_excluded_registrar_handles("excluded registrar handle doesn't exist:");
                for(Database::Result::size_type i = 0; i < not_existing_excluded_registrar_handles_res.size(); ++i)
                {
                    not_existing_excluded_registrar_handles += " ";
                    not_existing_excluded_registrar_handles += static_cast<std::string>(not_existing_excluded_registrar_handles_res[i][0]);
                }
                throw std::runtime_error(not_existing_excluded_registrar_handles);
            }
        }//check if excluded registrar handles exists

        std::vector<std::string> registrar_handles;

        Database::QueryParams params;
        params.reserve(excluded_registrar_handles_.size());

        Util::HeadSeparator where_and ("WHERE","AND");
        std::ostringstream sql;
        sql << "SELECT handle FROM registrar ";

        for(std::vector<std::string>::const_iterator ci = excluded_registrar_handles_.begin()
            ; ci != excluded_registrar_handles_.end(); ++ci)
        {
            params.push_back(*ci);
            sql << where_and.get() << " handle != UPPER($" << params.size() << "::text ) ";
        }
        sql << " ORDER BY HANDLE ";

        if(read_lock_) sql << " FOR SHARE";

        Database::Result registrar_handles_res = ctx.get_conn().exec_params(sql.str(),params);
        registrar_handles.reserve(registrar_handles_res.size());
        for(Database::Result::size_type i = 0; i < registrar_handles_res.size(); ++i)
        {
            registrar_handles.push_back(static_cast<std::string>(registrar_handles_res[i][0]));
        }
        return registrar_handles;
    }

    std::vector<std::string> GetRegistrarHandles::exec()
    {
        OperationContextCreator ctx;
        return exec(ctx);

    }


} // namespace Registrar
} // namespace LibFred
