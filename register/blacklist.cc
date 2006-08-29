#include <sstream>
#include "blacklist.h"
#include "dbsql.h"

namespace Register
{
  namespace Domain
  {
    class BlacklistImpl : virtual public Blacklist {
      DB *db;
     public:
      BlacklistImpl(DB *_db) : db(_db)
      {}
      bool checkDomain(const std::string& fqdn) const 
        throw (SQL_ERROR)
      {
        bool ret = false;
        std::ostringstream sql;
        sql << "SELECT id FROM domain_blacklist b "
            << "WHERE '" << fqdn << "' ~ b.regexp "
            << "AND NOW()>b.valid_from "
            << "AND (b.valid_to ISNULL OR NOW()<b.valid_to) ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        if (db->GetSelectRows() > 0) ret = true;
        db->FreeSelect();
        return ret;
      }
    };
    Blacklist* Blacklist::create(DB *db)
    {
      return new BlacklistImpl(db);
    }
  };
};
