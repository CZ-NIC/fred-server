/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#ifndef INVOICE_EXPORT_IMPL_HH_CDADC880BA034564AC8CAB737366B8F9
#define INVOICE_EXPORT_IMPL_HH_CDADC880BA034564AC8CAB737366B8F9

#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_fileman_args.hh"
#include "src/util/cfg/handle_messenger_args.hh"
#include "src/util/cfg/handle_secretary_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/bin/cli/invoice_export.hh"
#include "util/log/context.hh"

void invoice_export_impl()
{
    Logging::Context ctx("invoice_export_impl");

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

    Admin::invoice_export(
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleMessengerArgsGrp>()->get_args(),
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleFilemanArgsGrp>()->get_args(),
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleSecretaryArgsGrp>()->get_args(),
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceExportArgsGrp>()->invoice_dont_send);
};

#endif
