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
 *  @registrar_client_impl.h
 *  registrar client implementation header
 */

#ifndef REGISTRAR_CLIENT_IMPL_HH_7B95C8AD54434E83B42D3ABC025B2FE5
#define REGISTRAR_CLIENT_IMPL_HH_7B95C8AD54434E83B42D3ABC025B2FE5
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "util/log/context.hh"
#include "src/bin/cli/registrarclient.hh"

/**
 * \class zone_add_impl
 * \brief admin client implementation of zone_add
 */
struct zone_add_impl
{
  void operator()() const
  {
      Logging::Context ctx("zone_add_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , true //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientZoneAddArgsGrp>()->params //ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

//HandleAdminClientRegistrarAddArgsGrp
/**
 * \class registrar_add_impl
 * \brief admin client implementation of registrar_add
 */
struct registrar_add_impl
{
  void operator()() const
  {
      Logging::Context ctx("registrar_add_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , true //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientRegistrarAddArgsGrp>()->params //RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class registrar_add_zone_impl
 * \brief admin client implementation of registrar_add_zone
 */
struct registrar_add_zone_impl
{
  void operator()() const
  {
      Logging::Context ctx("registrar_add_zone_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , true //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientRegistrarAddZoneArgsGrp>()->params //RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class registrar_create_certification_impl
 * \brief admin client implementation of registrar_create_certification
 */
struct registrar_create_certification_impl
{
  void operator()() const
  {
      Logging::Context ctx("registrar_create_certification_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , true //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientRegistrarCreateCertificationArgsGrp>()->params //RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class registrar_create_group_impl
 * \brief admin client implementation of registrar_create_group
 */
struct registrar_create_group_impl
{
  void operator()() const
  {
      Logging::Context ctx("registrar_create_group_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , true //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientRegistrarCreateGroupArgsGrp>()->params //RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class registrar_into_group_impl
 * \brief admin client implementation of registrar_into_group
 */
struct registrar_into_group_impl
{
  void operator()() const
  {
      Logging::Context ctx("registrar_into_group_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , true //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientRegistrarIntoGroupArgsGrp>()->params //RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class registrar_list_impl
 * \brief admin client implementation of registrar_list
 */
struct registrar_list_impl
{
  void operator()() const
  {
      Logging::Context ctx("registrar_list_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , true //conf.hasOpt(REGISTRAR_LIST_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientRegistrarListArgsGrp>()->params //RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class zone_ns_add_impl
 * \brief admin client implementation of zone_ns_add
 */
struct zone_ns_add_impl
{
  void operator()() const
  {
      Logging::Context ctx("zone_ns_add_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , true //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientZoneNsAddArgsGrp>()->params //ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class registrar_acl_add_impl
 * \brief admin client implementation of registrar_acl_add
 */
struct registrar_acl_add_impl
{
  void operator()() const
  {
      Logging::Context ctx("registrar_acl_add_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , true //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientRegistrarAclAddArgsGrp>()->params //RegistrarAclAddArgs()
          , false //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};

/**
 * \class price_add_impl
 * \brief admin client implementation of price_add
 */
struct price_add_impl
{
  void operator()() const
  {
      Logging::Context ctx("price_add_impl");
      Admin::RegistrarClient registrar_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)
          , ZoneAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)
          , RegistrarAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)
          , RegistrarAddZoneArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME)
          , RegistrarCreateCertificationArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_CREATE_GROUP_NAME)
          , RegistrarCreateGroupArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_INTO_GROUP_NAME)
          , RegistrarIntoGroupArgs()
          , false //conf.hasOpt(REGISTRAR_LIST_NAME)
          , RegistrarListArgs()
          , false //conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)
          , false //conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)
          , ZoneNsAddArgs()
          , false //conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)
          , RegistrarAclAddArgs()
          , true //conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientPriceAddArgsGrp>()->params //PriceAddArgs()
      );
      registrar_client.runMethod();
      return ;
  }
};


#endif
