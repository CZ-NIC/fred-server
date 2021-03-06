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
/**
 *  @bank_client_impl.h
 *  bank client implementation header
 */

#ifndef BANK_CLIENT_IMPL_HH_27593C1EF81C44D688679B93F16F68C9
#define BANK_CLIENT_IMPL_HH_27593C1EF81C44D688679B93F16F68C9
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "util/log/context.hh"
#include "src/bin/cli/bankclient.hh"

/**
 * \class bank_add_account_impl
 * \brief admin client implementation of bank_add_account
 */
struct bank_add_account_impl
{
  void operator()() const
  {
      Logging::Context ctx("bank_add_account_impl");
      Admin::BankClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false//BANK_SHOW_OPTS_NAME
              , true//BANK_ADD_ACCOUNT_NAME
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientBankAddAccountArgsGrp>()->params //AddAccountArgs()
              );
      pom.runMethod();
  }
};

#endif
