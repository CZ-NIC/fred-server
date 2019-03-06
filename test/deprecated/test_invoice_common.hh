/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"

#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/corba/EPP.hh"
#include "src/bin/corba/epp/epp_impl.hh"


// thrown in case test has failed (something than CAN go wrong, not an error in test)
class CreateDomainFailed : public std::runtime_error {
public:
    CreateDomainFailed(const std::string &msg) : std::runtime_error(msg) 
    { }
};

void init_corba_container();

void handle_epp_exception(ccReg::EPP::EppError &ex);
void EPP_backend_init(ccReg_EPP_i *epp_i, HandleRifdArgs *rifd_args_ptr);
std::unique_ptr<ccReg_EPP_i> create_epp_backend_object();
CORBA::Long epp_backend_login(ccReg_EPP_i *epp, std::string registrar_handle);
