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
 *  @file database.h
 *  All includes for database library.
 */


#ifndef DATABASE_H_
#define DATABASE_H_

#include "connection.h"
#include "connection_factory.h"
#include "psql_connection.h"
#include "manager.h"
#include "manager_tss.h"


namespace Database {


/* default manager will be connection pooler */
typedef Factory::ConnectionPool<PSQLConnection>    ConnectionPool;
typedef TSSManager_<ConnectionPool>                TSSManager;

typedef Factory::Simple<PSQLConnection>            ConnectionFactory;
typedef Manager_<ConnectionFactory>                Manager;


/**
 * Definition for specific database objects with driver specified
 * in manager above
 */
// typedef TSSManager::connection_type     Connection;
// typedef TSSManager::transaction_type    Transaction;
// typedef TSSManager::result_type         Result;
// typedef TSSManager::row_type            Row;

typedef Manager::connection_type        Connection;
typedef Manager::transaction_type       Transaction;
typedef Manager::result_type            Result;
typedef Manager::row_type               Row;


}

#include "sequence.h"
#include "query.h"

#endif /*DATABASE_H_*/

