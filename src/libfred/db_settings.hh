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

#include "src/util/db/database.hh"

namespace Database {

  typedef Factory::Simple<PSQLConnection> ConnectionFactory;
  typedef TSSManager_<ConnectionFactory>          Manager;

  typedef Manager::connection_type                Connection;
  typedef Manager::transaction_type               Transaction;
  typedef Manager::result_type                    Result;
  typedef Manager::sequence_type                  Sequence;
  typedef Manager::row_type                       Row;
  
  typedef Factory::Simple<PSQLConnection>         StandaloneConnectionFactory;
  typedef Manager_<StandaloneConnectionFactory>       StandaloneManager;

  typedef StandaloneManager::connection_type                StandaloneConnection;
  typedef StandaloneManager::transaction_type               StandaloneTransaction;
  typedef StandaloneManager::result_type                    StandaloneResult;
  typedef StandaloneManager::sequence_type                  StandaloneSequence;
  typedef StandaloneManager::row_type                       StandaloneRow;

}


#endif

