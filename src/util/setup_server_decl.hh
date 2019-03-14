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
 *  @file setup_server_decl.h
 *  Declaration of server setup.
 */

#ifndef SETUP_SERVER_DECL_HH_9810837C9F6943C09DD10B8E5526D0A6
#define SETUP_SERVER_DECL_HH_9810837C9F6943C09DD10B8E5526D0A6

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/corba_wrapper_decl.hh"

void setup_logging(CfgArgs* cfg_instance_ptr);
void run_server(CfgArgs* cfg_instance_ptr , CorbaContainer* corba_instance_ptr);
void corba_init();

#endif
