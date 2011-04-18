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
 *  @contact_client_impl.h
 *  contact client implementation header
 */

#ifndef CONTACT_CLIENT_IMPL_H_
#define CONTACT_CLIENT_IMPL_H_
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "handle_adminclientselection_args.h"
#include "log/context.h"
#include "contactclient.h"


/**
 * \class contact_info_impl
 * \brief admin client implementation of contact_info
 */
struct contact_info_impl
{
  void operator()() const
  {
      Logging::Context ctx("contact_info_impl");
      Admin::ContactClient contact_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false //contact_list
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientContactInfoArgsGrp >()->contact_info //contact_info
              , false //contact_show_opts bool _contact_show_opts
              , ContactListArgs()
              );
      contact_client.runMethod();
      return ;
  }
};

/**
 * \class contact_list_impl
 * \brief admin client implementation of contact_list
 */
struct contact_list_impl
{
  void operator()() const
  {
      Logging::Context ctx("contact_list_impl");
      Admin::ContactClient contact_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , true //contact_list
              , optional_string() //contact_info
              , false //contact_show_opts bool _contact_show_opts
              , ContactListArgs()
              );
      contact_client.runMethod();
      return ;
  }
};

#endif // CONTACT_CLIENT_IMPL_H_
