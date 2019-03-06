/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef MANAGER_OLD_DB_HH_566862C4B8BB45898EB728CDA6A0A2A0
#define MANAGER_OLD_DB_HH_566862C4B8BB45898EB728CDA6A0A2A0

#include "src/deprecated/util/dbsql.hh"
#include "libfred/db_settings.hh"

namespace Database {

/**
 * UGLY HACK for allowing use of old DB connection object
 * through new Database::Manager class
 *
 * this is only temporarily used for notify EPP update changes
 * and should NOT be used in the future!
 */
class OldDBManager : public Manager {
public:
  typedef Manager::connection_driver     connection_driver;
  typedef Manager::connection_type       connection_type;
  typedef Manager::transaction_type      transaction_type;
  typedef Manager::sequence_type         sequence_type;
  typedef Manager::row_type              row_type;


  OldDBManager(DB *_db) : Manager(0), db_(_db) { }


  virtual ~OldDBManager() { }


  virtual connection_type* acquire() {
    return new connection_type(new connection_driver(db_->__getPGconn()));
  }

private:
  DB *db_;
};


}


#endif /*MANAGER_OLD_DB_H_*/

