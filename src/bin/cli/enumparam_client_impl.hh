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
 *  @enumparam_client_impl.h
 *  enumparam client implementation header
 */

#ifndef ENUMPARAM_CLIENT_IMPL_HH_EF0E8982D1BA40E490F06D40E6C8BAF4
#define ENUMPARAM_CLIENT_IMPL_HH_EF0E8982D1BA40E490F06D40E6C8BAF4
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "util/log/context.hh"
#include "src/bin/cli/enumparamclient.hh"

/**
 * \class enum_parameter_change_impl
 * \brief admin client implementation of enum_parameter_change
 */
struct enum_parameter_change_impl
{
  void operator()() const
  {
      Logging::Context ctx("enum_parameter_change_impl");
      Admin::EnumParamClient enum_param_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , true
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientEnumParameterChangeArgsGrp>()->params
              );
      enum_param_client.runMethod();
  }
};

#endif
