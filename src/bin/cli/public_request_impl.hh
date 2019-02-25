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
#ifndef PUBLIC_REQUEST_IMPL_HH_5710CC5D97C944A89C5C30F8750D136A
#define PUBLIC_REQUEST_IMPL_HH_5710CC5D97C944A89C5C30F8750D136A

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "src/bin/cli/public_request_method.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "util/log/context.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/corba/file_manager_client.hh"
#include "libfred/mailer.hh"
#include "src/deprecated/libfred/file_transferer.hh"

#include <memory>

struct process_public_requests_impl
{
    void operator()() const
    {
        Logging::Context ctx("process_public_requests_impl");

        FakedArgs orb_fa = CfgArgGroups::instance()->fa;

        orb_fa.add_argv(std::string("-ORBnativeCharCodeSet"));
        orb_fa.add_argv(std::string("UTF-8"));

        HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
                   get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

        CorbaContainer::set_instance(orb_fa.get_argc(),
                                     orb_fa.get_argv(),
                                     ns_args_ptr->get_nameservice_host(),
                                     ns_args_ptr->get_nameservice_port(),
                                     ns_args_ptr->get_nameservice_context());

        Admin::PublicRequestProcedure public_request(
                CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientProcessPublicRequestsArgsGrp>()->process_public_requests_params,
                static_cast<std::shared_ptr<LibFred::Mailer::Manager>>(
                        std::make_shared<MailerManager>(CorbaContainer::get_instance()->getNS())),
                static_cast<std::shared_ptr<LibFred::File::Transferer>>(
                        std::make_shared<FileManagerClient>(CorbaContainer::get_instance()->getNS()))
            );
        public_request.exec();
    }
};

#endif
