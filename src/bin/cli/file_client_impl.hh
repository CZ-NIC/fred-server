/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file_client_impl.h
 *  file client implementation header
 */

#ifndef FILE_CLIENT_IMPL_HH_5DDC000846C64CBEA63025C75F65FA8A
#define FILE_CLIENT_IMPL_HH_5DDC000846C64CBEA63025C75F65FA8A
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"

#include "src/bin/cli/fileclient.hh"

/**
 * \class file_list_impl
 * \brief admin client implementation of file_list
 */
struct file_list_impl
{
  void operator()() const
  {
      Admin::FileClient  file_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , true //bool file_list
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientFileListArgsGrp>()->params//FileListArgs()
              );
      file_client.runMethod();
  }
};

#endif
