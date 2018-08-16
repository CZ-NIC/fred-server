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
 *  @invoice_client_impl.h
 *  invoice client implementation header
 */

#ifndef INVOICE_CLIENT_IMPL_HH_6E4EE876C9984B8EBD641AD90656A56C
#define INVOICE_CLIENT_IMPL_HH_6E4EE876C9984B8EBD641AD90656A56C
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "src/util/log/context.hh"
#include "src/bin/cli/invoiceclient.hh"


/**
 * \class invoice_list_impl
 * \brief admin client implementation of invoice_list
 */
struct invoice_list_impl
{
  void operator()() const
  {
      Logging::Context ctx("invoice_list_impl");
      Admin::InvoiceClient invoice_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceListArgsGrp>()->params
              , true //bool _invoice_list
              , false//bool _invoice_list_filters
              , false//bool _invoice_archive
              , false//bool _invoice_credit
              , false//bool _invoice_billing
              , false//bool _invoice_add_prefix
              , false//bool _invoice_create
              , false//bool _invoice_show_opts
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , false//bool _invoice_dont_send
              , InvoiceCreditArgs()
              , InvoiceBillingArgs()
              , InvoicePrefixArgs()
              );
      invoice_client.runMethod();

      return ;
  }
};

/**
 * \class invoice_archive_impl
 * \brief admin client implementation of invoice_archive
 */
struct invoice_archive_impl
{
  void operator()() const
  {
      Logging::Context ctx("invoice_archive_impl");
      Admin::InvoiceClient invoice_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , InvoiceListArgs()
              , false //bool _invoice_list
              , false//bool _invoice_list_filters
              , true//bool _invoice_archive
              , false//bool _invoice_credit
              , false//bool _invoice_billing
              , false//bool _invoice_add_prefix
              , false//bool _invoice_create
              , false//bool _invoice_show_opts
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceArchiveArgsGrp>()->invoice_dont_send//bool _invoice_dont_send
              , InvoiceCreditArgs()
              , InvoiceBillingArgs()
              , InvoicePrefixArgs()
              );
      invoice_client.runMethod();

      return ;
  }
};

/**
 * \class invoice_credit_impl
 * \brief admin client implementation of invoice_credit
 */
struct invoice_credit_impl
{
  void operator()() const
  {
      Logging::Context ctx("invoice_credit_impl");
      Admin::InvoiceClient invoice_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , InvoiceListArgs()
              , false //bool _invoice_list
              , false//bool _invoice_list_filters
              , false//bool _invoice_archive
              , true//bool _invoice_credit
              , false//bool _invoice_billing
              , false//bool _invoice_add_prefix
              , false//bool _invoice_create
              , false//bool _invoice_show_opts
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , false//bool _invoice_dont_send
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceCreditArgsGrp>()->params//InvoiceCreditArgs()
              , InvoiceBillingArgs()
              , InvoicePrefixArgs()
              );
      invoice_client.runMethod();

      return ;
  }
};

/**
 * \class invoice_billing_impl
 * \brief admin client implementation of invoice_billing
 */
struct invoice_billing_impl
{
  void operator()() const
  {
      Logging::Context ctx("invoice_billing_impl");
      Admin::InvoiceClient invoice_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , InvoiceListArgs()
              , false //bool _invoice_list
              , false//bool _invoice_list_filters
              , false//bool _invoice_archive
              , false//bool _invoice_credit
              , true//bool _invoice_billing
              , false//bool _invoice_add_prefix
              , false//bool _invoice_create
              , false//bool _invoice_show_opts
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , false//bool _invoice_dont_send
              , InvoiceCreditArgs()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceBillingArgsGrp>()->params//InvoiceBillingArgs()
              , InvoicePrefixArgs()
              );
      invoice_client.runMethod();

      return ;
  }
};

/**
 * \class invoice_add_prefix_impl
 * \brief admin client implementation of invoice_add_prefix
 */
struct invoice_add_prefix_impl
{
  void operator()() const
  {
      Logging::Context ctx("invoice_add_prefix_impl");
      Admin::InvoiceClient invoice_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , InvoiceListArgs()
              , false //bool _invoice_list
              , false//bool _invoice_list_filters
              , false//bool _invoice_archive
              , false//bool _invoice_credit
              , false//bool _invoice_billing
              , true//bool _invoice_add_prefix
              , false//bool _invoice_create
              , false//bool _invoice_show_opts
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , false//bool _invoice_dont_send
              , InvoiceCreditArgs()
              , InvoiceBillingArgs()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientInvoiceAddPrefixArgsGrp>()->params//InvoicePrefixArgs()
              );
      invoice_client.runMethod();

      return ;
  }
};

/**
 * \class create_invoice_prefixes_impl
 * \brief admin client implementation of create_invoice_prefixes_impl
 */
struct create_invoice_prefixes_impl
{
  void operator()() const
  {
      Logging::Context ctx("create_invoice_prefixes_impl");
      Admin::create_invoice_prefixes(CfgArgGroups::instance()
      ->get_handler_ptr_by_type
          <HandleAdminClientCreateInvoicePrefixesArgsGrp>()
      ->params);

      return ;
  }
};

/**
 * \class add_invoice_number_prefix_impl
 * \brief admin client implementation of add_invoice_number_prefix_impl
 */
struct add_invoice_number_prefix_impl
{
  void operator()() const
  {
      Logging::Context ctx("add_invoice_number_prefix_impl");

      Admin::add_invoice_number_prefix(CfgArgGroups::instance()
          ->get_handler_ptr_by_type
              <HandleAdminClientAddInvoiceNumberPrefixArgsGrp>()
          ->params);

      return ;
  }
};


/**
 * \class create_invoiceimpl
 * \brief admin client implementation of create_invoice
 */
struct create_invoice_impl
{
  void operator()() const
  {
      Logging::Context ctx("create_invoice_impl");
      Admin::InvoiceClient invoice_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , InvoiceListArgs()
              , false //bool _invoice_list
              , false//bool _invoice_list_filters
              , false//bool _invoice_archive
              , false//bool _invoice_credit
              , false//bool _invoice_billing
              , false//bool _invoice_add_prefix
              , true//bool _invoice_create
              , false//bool _invoice_show_opts
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , false//bool _invoice_dont_send
              , InvoiceCreditArgs()
              , InvoiceBillingArgs()
              , InvoicePrefixArgs()
              );
      invoice_client.runMethod();

      return ;
  }
};

#endif
