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
#include "result.h"
#include "psql_connection.h"
#include "db_exceptions.h"

namespace Database {

/**
 * Definition for specific objects with PSQL driver
 */
typedef Connection_<PSQLConnection> Connection;
typedef Result_<PSQLResult> Result;
typedef Transaction_<PSQLTransaction> Transaction;
typedef Result::result_type::Row Row;

}

#endif /*DATABASE_H_*/

