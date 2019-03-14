/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef PUBLIC_REQUEST_METHOD_HH_51FA81CF886A4AADB94A936FCA6123D0
#define PUBLIC_REQUEST_METHOD_HH_51FA81CF886A4AADB94A936FCA6123D0

#include "src/deprecated/util/dbsql.hh"
#include "libfred/db_settings.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/corba/file_manager_client.hh"
#include "libfred/mailer.hh"
#include "src/deprecated/libfred/file_transferer.hh"
#include "src/bin/cli/public_request_params.hh"

#include <boost/program_options.hpp>
#include <iostream>
#include <memory>

namespace Admin {

class PublicRequestProcedure
{
public:
    PublicRequestProcedure(
            const ProcessPublicRequestsArgs& _args,
            std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager,
            std::shared_ptr<LibFred::File::Transferer> _file_manager_client)
        : args_(_args),
          mailer_manager_(std::move(_mailer_manager)),
          file_manager_client_(std::move(_file_manager_client))
        {
        }

    void exec();
private:
    ProcessPublicRequestsArgs args_;
    std::shared_ptr<LibFred::Mailer::Manager> mailer_manager_;
    std::shared_ptr<LibFred::File::Transferer> file_manager_client_;
};

} // namespace Admin;


#endif
