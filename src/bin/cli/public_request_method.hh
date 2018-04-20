/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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


#ifndef PUBLIC_REQUEST_METHOD_HH_51FA81CF886A4AADB94A936FCA6123D0
#define PUBLIC_REQUEST_METHOD_HH_51FA81CF886A4AADB94A936FCA6123D0

#include <boost/program_options.hpp>
#include <iostream>

#include "src/deprecated/util/dbsql.hh"
#include "src/libfred/db_settings.hh"

#include "src/bin/cli/public_request_params.hh"

namespace Admin {

class PublicRequestProcedure
{
    public:
        PublicRequestProcedure(const ProcessPublicRequestsArgs& _args)
            : args(_args)
        {
        }

        void exec();
    private:
        ProcessPublicRequestsArgs args;
};

} // namespace Admin;


#endif
