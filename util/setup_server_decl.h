/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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
 *  @file setup_server_decl.h
 *  Declaration of server setup.
 */

#ifndef SETUP_SERVER_DECL_H_
#define SETUP_SERVER_DECL_H_

#include "cfg/config_handler_decl.h"
#include "corba_wrapper_decl.h"

void setup_logging(CfgArgs * cfg_instance_ptr);
void run_server(CfgArgs * cfg_instance_ptr , CorbaContainer* corba_instance_ptr );
void corba_init();

#endif //SETUP_SERVER_DECL_H_
