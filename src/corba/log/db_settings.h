#ifndef DB_SETTINGS_H_
#define DB_SETTINGS_H_

#include "db/database.h"

namespace Database {

  typedef Factory::ConnectionPool<PSQLConnection> ConnectionFactory;
  typedef TSSManager_<ConnectionFactory>          Manager;

/*
 // cannot call Manager::acquire() as a static function
  typedef Factory::Simple<PSQLConnection>         ConnectionFactory;
  typedef Manager_<ConnectionFactory>             Manager;
*/

  typedef Manager::connection_type                Connection;
  typedef Manager::transaction_type               Transaction;
  typedef Manager::result_type                    Result;
  typedef Manager::sequence_type                  Sequence;
  typedef Manager::row_type                       Row;

}


#endif

