/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @keyset_client_impl.h
 *  keyset client implementation header
 */

#ifndef KEYSET_CLIENT_IMPL_H_
#define KEYSET_CLIENT_IMPL_H_
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cli_admin/handle_adminclientselection_args.h"
#include "log/context.h"
#include "cli_admin/keysetclient.h"


/**
 * \class keyset_check_impl
 * \brief admin client implementation of keyset_check
 */
struct keyset_check_impl
{
  void operator()() const
  {
      Logging::Context ctx("keyset_check_impl");
      Admin::KeysetClient keyset_client (
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false //bool _keyset_list
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientKeySetCheckArgsGrp>()->keyset_check//const optional_string& _keyset_check
              , false//bool _keyset_list_plain
              , optional_string()//const optional_string& _keyset_info
              , optional_string()//const optional_string& _keyset_info2
              , false//bool _keyset_show_opts
              , KeysetListArgs());
      keyset_client.runMethod();
      return ;
  }
};

/**
 * \class keyset_info_impl
 * \brief admin client implementation of keyset_info
 */
struct keyset_info_impl
{
  void operator()() const
  {
      Logging::Context ctx("keyset_info_impl");
      Admin::KeysetClient keyset_client (
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false //bool _keyset_list
              , optional_string() //const optional_string& _keyset_check
              , false//bool _keyset_list_plain
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientKeySetInfoArgsGrp>()->keyset_info//const optional_string& _keyset_info
              , optional_string()//const optional_string& _keyset_info2
              , false//bool _keyset_show_opts
              , KeysetListArgs());
      keyset_client.runMethod();
      return ;
  }
};

/**
 * \class keyset_info2_impl
 * \brief admin client implementation of keyset_info
 */
struct keyset_info2_impl
{
  void operator()() const
  {
      Logging::Context ctx("keyset_info2_impl");
      Admin::KeysetClient keyset_client (
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false //bool _keyset_list
              , optional_string() //const optional_string& _keyset_check
              , false//bool _keyset_list_plain
              , optional_string()//const optional_string& _keyset_info
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientKeySetInfo2ArgsGrp>()->keyset_info2//const optional_string& _keyset_info2
              , false//bool _keyset_show_opts
              , KeysetListArgs());
      keyset_client.runMethod();
      return ;
  }
};

/**
 * \class keyset_list_impl
 * \brief admin client implementation of keyset_list
 */
struct keyset_list_impl
{
  void operator()() const
  {
      Logging::Context ctx("keyset_list_impl");
      Admin::KeysetClient keyset_client (
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false //bool _keyset_list
              , optional_string() //const optional_string& _keyset_check
              , false//bool _keyset_list_plain
              , optional_string()//const optional_string& _keyset_info
              , optional_string()//const optional_string& _keyset_info2
              , false//bool _keyset_show_opts
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientKeySetListArgsGrp>()->params//KeysetListArgs()
              );
      keyset_client.runMethod();
      return ;
  }
};

/**
 * \class keyset_list_plain_impl
 * \brief admin client implementation of keyset_list_plain
 */
struct keyset_list_plain_impl
{
  void operator()() const
  {
      Logging::Context ctx("keyset_list_plain_impl");
      Admin::KeysetClient keyset_client (
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false //bool _keyset_list
              , optional_string() //const optional_string& _keyset_check
              , true//bool _keyset_list_plain
              , optional_string()//const optional_string& _keyset_info
              , optional_string()//const optional_string& _keyset_info2
              , false//bool _keyset_show_opts
              , KeysetListArgs()
              );
      keyset_client.runMethod();
      return ;
  }
};

#endif // KEYSET_CLIENT_IMPL_H_
