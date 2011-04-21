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
 *  @bank_client_impl.h
 *  bank client implementation header
 */

#ifndef BANK_CLIENT_IMPL_H_
#define BANK_CLIENT_IMPL_H_
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cli_admin/handle_adminclientselection_args.h"
#include "log/context.h"
#include "cli_admin/bankclient.h"

/**
 * \class payment_list_impl
 * \brief admin client implementation of payment_list
 */
struct payment_list_impl
{
  void operator()() const
  {
      Logging::Context ctx("payment_list_impl");
      Admin::BankClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false//BANK_SHOW_OPTS_NAME
              , true//BANK_PAYMENT_LIST_NAME
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientBankPaymentListArgsGrp>()->bank_payment_type //BANK_PAYMENT_TYPE_NAME
              , false //BANK_IMPORT_XML_NAME
              , ImportXMLArgs()
              , false//BANK_ADD_ACCOUNT_NAME
              , AddAccountArgs()
              );
      pom.runMethod();
  }
};

/**
 * \class bank_import_xml_impl
 * \brief admin client implementation of bank_import_xml
 */
struct bank_import_xml_impl
{
  void operator()() const
  {
      Logging::Context ctx("bank_import_xml_impl");
      Admin::BankClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false//BANK_SHOW_OPTS_NAME
              , false//BANK_PAYMENT_LIST_NAME
              , optional_ulong()//CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientBankPaymentListArgsGrp>()->bank_payment_type //BANK_PAYMENT_TYPE_NAME
              , true //BANK_IMPORT_XML_NAME
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientBankImportXMLArgsGrp>()->params //ImportXMLArgs()
              , false//BANK_ADD_ACCOUNT_NAME
              , AddAccountArgs()
              );
      pom.runMethod();
  }
};


/**
 * \class bank_add_account_impl
 * \brief admin client implementation of bank_add_account
 */
struct bank_add_account_impl
{
  void operator()() const
  {
      Logging::Context ctx("bank_add_account_impl");
      Admin::BankClient pom(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , false//BANK_SHOW_OPTS_NAME
              , false//BANK_PAYMENT_LIST_NAME
              , optional_ulong()//BANK_PAYMENT_TYPE_NAME
              , false //BANK_IMPORT_XML_NAME
              , ImportXMLArgs()
              , true//BANK_ADD_ACCOUNT_NAME
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientBankAddAccountArgsGrp>()->params //AddAccountArgs()
              );
      pom.runMethod();
  }
};

#endif // BANK_CLIENT_IMPL_H_
