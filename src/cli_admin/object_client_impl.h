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
 *  @object_client_impl.h
 *  object client implementation header
 */

#ifndef OBJECT_CLIENT_IMPL_H_
#define OBJECT_CLIENT_IMPL_H_
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cli_admin/handle_adminclientselection_args.h"
#include "log/context.h"
#include "cli_admin/objectclient.h"

/**
 * \class object_new_state_request_impl
 * \brief admin client implementation of object_new_state_request
 */

struct object_new_state_request_impl
{
  void operator()() const
  {
      Logging::Context ctx("object_new_state_request_impl");
      Admin::ObjectClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_domain_count_limit()
              , true//const bool _object_new_state_request
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientObjectNewStateRequestArgsGrp>()->params//ObjectNewStateRequestArgs()//const ObjectNewStateRequestArgs& _object_new_state_request_params
              , false//const bool _object_new_state_request_name
              , ObjectNewStateRequestNameArgs()//const ObjectNewStateRequestNameArgs& _object_new_state_request_name_params
              , false//const bool _object_update_states
              , ObjectUpdateStatesArgs()//const ObjectUpdateStatesArgs& _object_update_states_params
              , false//const bool _object_regular_procedure
              , ObjectRegularProcedureArgs()//const ObjectRegularProcedureArgs& _object_regular_procedure_params
              , false//const bool _object_delete_candidates
              , DeleteObjectsArgs()//const DeleteObjectsArgs& _delete_objects_params
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_disable_epp_notifier_cltrid_prefix()
              );
       pom.runMethod();
  }
};


/**
 * \class object_new_state_request_name_impl
 * \brief admin client implementation of object_new_state_request_name
 */

struct object_new_state_request_name_impl
{
  void operator()() const
  {
      Logging::Context ctx("object_new_state_request_name_impl");
      Admin::ObjectClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_domain_count_limit()
              , false//const bool _object_new_state_request
              , ObjectNewStateRequestArgs()//const ObjectNewStateRequestArgs& _object_new_state_request_params
              , true//const bool _object_new_state_request_name
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientObjectNewStateRequestNameArgsGrp>()->params//ObjectNewStateRequestNameArgs()//const ObjectNewStateRequestNameArgs& _object_new_state_request_name_params
              , false//const bool _object_update_states
              , ObjectUpdateStatesArgs()//const ObjectUpdateStatesArgs& _object_update_states_params
              , false//const bool _object_regular_procedure
              , ObjectRegularProcedureArgs()//const ObjectRegularProcedureArgs& _object_regular_procedure_params
              , false//const bool _object_delete_candidates
              , DeleteObjectsArgs()//const DeleteObjectsArgs& _delete_objects_params
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_disable_epp_notifier_cltrid_prefix()
              );
       pom.runMethod();
  }
};

/**
 * \class object_update_states_impl
 * \brief admin client implementation of object_update_states
 */

struct object_update_states_impl
{
  void operator()() const
  {
      Logging::Context ctx("object_update_states_impl");
      Admin::ObjectClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_domain_count_limit()
              , false//const bool _object_new_state_request
              , ObjectNewStateRequestArgs()//const ObjectNewStateRequestArgs& _object_new_state_request_params
              , false//const bool _object_new_state_request_name
              , ObjectNewStateRequestNameArgs()//const ObjectNewStateRequestNameArgs& _object_new_state_request_name_params
              , true//const bool _object_update_states
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientObjectUpdateStatesArgsGrp>()->params //ObjectUpdateStatesArgs()//const ObjectUpdateStatesArgs& _object_update_states_params
              , false//const bool _object_regular_procedure
              , ObjectRegularProcedureArgs()//const ObjectRegularProcedureArgs& _object_regular_procedure_params
              , false//const bool _object_delete_candidates
              , DeleteObjectsArgs()//const DeleteObjectsArgs& _delete_objects_params
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_disable_epp_notifier_cltrid_prefix()
              );
       pom.runMethod();
  }
};

/**
 * \class object_regular_procedure_impl
 * \brief admin client implementation of object_regular_procedure
 */

struct object_regular_procedure_impl
{
  void operator()() const
  {
      Logging::Context ctx("object_regular_procedure_impl");
      Admin::ObjectClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_domain_count_limit()
              , false//const bool _object_new_state_request
              , ObjectNewStateRequestArgs()//const ObjectNewStateRequestArgs& _object_new_state_request_params
              , false//const bool _object_new_state_request_name
              , ObjectNewStateRequestNameArgs()//const ObjectNewStateRequestNameArgs& _object_new_state_request_name_params
              , false//const bool _object_update_states
              , ObjectUpdateStatesArgs()//const ObjectUpdateStatesArgs& _object_update_states_params
              , true//const bool _object_regular_procedure
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientObjectRegularProcedureArgsGrp>()->regular_procedure_params //ObjectRegularProcedureArgs()//const ObjectRegularProcedureArgs& _object_regular_procedure_params
              , false//const bool _object_delete_candidates
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientObjectRegularProcedureArgsGrp>()->delete_objects_params//DeleteObjectsArgs()//const DeleteObjectsArgs& _delete_objects_params
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_disable_epp_notifier_cltrid_prefix()
              );
       pom.runMethod();
  }
};

/**
 * \class object_delete_candidates_impl
 * \brief admin client implementation of object_delete_candidates
 */

struct object_delete_candidates_impl
{
  void operator()() const
  {
      Logging::Context ctx("object_delete_candidates_impl");
      Admin::ObjectClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_domain_count_limit()
              , false//const bool _object_new_state_request
              , ObjectNewStateRequestArgs()//const ObjectNewStateRequestArgs& _object_new_state_request_params
              , false//const bool _object_new_state_request_name
              , ObjectNewStateRequestNameArgs()//const ObjectNewStateRequestNameArgs& _object_new_state_request_name_params
              , false//const bool _object_update_states
              , ObjectUpdateStatesArgs()//const ObjectUpdateStatesArgs& _object_update_states_params
              , false//const bool _object_regular_procedure
              , ObjectRegularProcedureArgs()
              , true//const bool _object_delete_candidates
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientObjectDeleteCandidatesArgsGrp>()->delete_objects_params//DeleteObjectsArgs()//const DeleteObjectsArgs& _delete_objects_params
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_disable_epp_notifier_cltrid_prefix()
              );
       pom.runMethod();
  }
};

#endif // OBJECT_CLIENT_IMPL_H_
