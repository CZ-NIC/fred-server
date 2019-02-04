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
 *  @domain_client_impl.h
 *  domain client implementation header
 */

#ifndef DOMAIN_CLIENT_IMPL_HH_5A27F567122B48C7B842182F1B73AD06
#define DOMAIN_CLIENT_IMPL_HH_5A27F567122B48C7B842182F1B73AD06
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_createexpireddomain_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "util/log/context.hh"
#include "src/bin/cli/domainclient.hh"
#include "src/bin/corba/logger_client_impl.hh"
#include "src/backend/admin/domain/create_expired_domain.hh"

/**
 * \class domain_list_impl
 * \brief admin client implementation of domain_list
 */
struct domain_list_impl
{
  void operator()() const
  {
      Logging::Context ctx("domain_list_impl");
      Admin::DomainClient domain_client(
          CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
          , true //bool domain_list
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientDomainListArgsGrp>()->params
          );
      domain_client.runMethod();
      return ;
  }
};

/**
 * \class create_expired_domain_impl
 * \brief admin client implementation of create_expired_domain
 */
struct create_expired_domain_impl
{
    void operator()() const
    {
        Logging::Context ctx("create_expired_domain_impl");

        FakedArgs orb_fa = CfgArgGroups::instance()->fa;
        HandleCorbaNameServiceArgsGrp* ns_args_ptr = CfgArgGroups::instance()->
                   get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

        CorbaContainer::set_instance(
                orb_fa.get_argc(),
                orb_fa.get_argv(),
                ns_args_ptr->get_nameservice_host(),
                ns_args_ptr->get_nameservice_port(),
                ns_args_ptr->get_nameservice_context()
                );

        auto logger_client = std::make_unique<LibFred::Logger::LoggerCorbaClientImpl>();

        if (!logger_client) {
            throw std::runtime_error("unable to get request logger reference");
        }

        const auto params = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientCreateExpiredDomainArgsGrp>()->params;
        const auto registrar_handle = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCreateExpiredDomainArgsGrp>()->get_registrar_handle();
        Admin::Domain::create_expired_domain(*logger_client, params.fqdn, params.registrant, params.cltrid, params.delete_existing, registrar_handle);
    }
};

#endif
