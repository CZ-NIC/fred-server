#include "admin.h"

namespace Admin
{
 class ManagerImpl
 {
   DBSql *db;
  public:
   ManagerImpl(DBSql *_db) : db(_db)
   {} 
   CheckHandle checkHandle(const std::string& handle)
   {
     
   }
 };
 Manager::create(DBSql *db)
 {
   return new ManagerImpl(db);
 }
}
