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

