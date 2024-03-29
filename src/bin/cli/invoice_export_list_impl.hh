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

#ifndef INVOICE_EXPORT_LIST_IMPL_HH_0390F82D29944D649E148E49880B7952
#define INVOICE_EXPORT_LIST_IMPL_HH_0390F82D29944D649E148E49880B7952

#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_fileman_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/bin/cli/invoice_export.hh"
#include "util/log/context.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

void invoice_export_list_impl()
{
    Logging::Context ctx("invoice_export_list_impl");

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

    const auto taxdate_from = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceExportListArgsGrp>()->taxdate_from.is_value_set()
        ? boost::optional<boost::gregorian::date>{boost::gregorian::from_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceExportListArgsGrp>()->taxdate_from)}
        : boost::none;
    const auto taxdate_to = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceExportListArgsGrp>()->taxdate_to.is_value_set()
        ? boost::optional<boost::gregorian::date>{boost::gregorian::from_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceExportListArgsGrp>()->taxdate_to)}
        : boost::none;

    Admin::invoice_export_list(
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleFilemanArgsGrp>()->get_args(),
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceExportListArgsGrp>()->invoice_id,
            taxdate_from,
            taxdate_to,
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceExportListArgsGrp>()->limit);
};

#endif
