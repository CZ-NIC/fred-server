/*
 * Copyright (C) 2019  CZ.NIC, z. s. p. o.
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
 *  @file db_settings.hh
 *  Usage setting for database:
 *
 *  There should be defined user database settings:
 *    Manager and used Connection factory, and following types
 *    defined as internal and depends on used connection manager type
 *    (Manager + Connection factory) - Connection, Transaction, 
 *    Result, Sequence and Row types.
 */

#ifndef DB_SETTINGS_HH_496DFB2A05584145B21F5808014C6545x
#define DB_SETTINGS_HH_496DFB2A05584145B21F5808014C6545x

#include "util/db/database.hh"
#include "src/util/db/manager_tss.hh"

namespace Database {

typedef Factory::Simple<PSQLConnection> ConnectionFactory;
typedef TSSManager_<ConnectionFactory> Manager;

typedef Manager::connection_type Connection;
typedef Manager::transaction_type Transaction;
typedef Manager::sequence_type Sequence;
  
}//namespace Database

#endif//DB_SETTINGS_HH_496DFB2A05584145B21F5808014C6545x
