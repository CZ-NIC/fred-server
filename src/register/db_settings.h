/*  
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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
 *  @file db_settings.h
 *  Usage setting for database:
 *
 *  There should be defined user database settings:
 *    Manager and used Connection factory, and following types
 *    defined as internal and depends on used connection manager type
 *    (Manager + Connection factory) - Connection, Transaction, 
 *    Result, Sequence and Row types.
 */

#ifndef DB_SETTINGS_H_
#define DB_SETTINGS_H_

#include "db/database.h"

namespace Database {

  typedef Factory::Simple<PSQLConnection> ConnectionFactory;
  typedef TSSManager_<ConnectionFactory>          Manager;

  typedef Manager::connection_type                Connection;
  typedef Manager::transaction_type               Transaction;
  typedef Manager::result_type                    Result;
  typedef Manager::sequence_type                  Sequence;
  typedef Manager::row_type                       Row;
  
  // typedef Factory::Simple<PSQLConnection>         SimpleConnectionFactory;
  // typedef Manager_<SimpleConnectionFactory>       SimpleManager;
// 
  // typedef SimpleManager::connection_type                SimpleConnection;
  // typedef SimpleManager::transaction_type               SimpleTransaction;
  // typedef SimpleManager::result_type                    SimpleResult;
  // typedef SimpleManager::sequence_type                  SimpleSequence;
  // typedef SimpleManager::row_type                       SimpleRow;

}


#endif

