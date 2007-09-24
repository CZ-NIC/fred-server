#include "notify.h"
#include "dbsql.h"
#include <sstream>

namespace Register
{
  namespace Notify
  {
    class ManagerImpl : virtual public Manager
    {
      DB *db;
      Mailer::Manager *mm;
     public:
      ManagerImpl(DB *_db, Mailer::Manager *_mm)
	: db(_db), mm(_mm)
      {}
      void notifyStateChanges() throw (SQL_ERROR)
      {
        std::stringstream sql;
        sql << "SELECT nt.state_id, nt.type, nt.mtype, nt.obj_id FROM "
            << "(SELECT s.id AS state_id, nm.id AS type, "
	    << "nm.mail_type_id AS mtype, obr.id AS obj_id "
            << "FROM object_state s, object_registry obr, notify_statechange_map nm "
            << "WHERE s.object_id=obr.id AND obr.type=nm.obj_type "
	    << "AND s.state_id=nm.state_id) AS nt "
	    << "LEFT JOIN notify_statechange ns ON ("
	    << "nt.state_id=ns.state_id AND nt.type=ns.type)";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
	}
      }
    };
    Manager *Manager::create(DB *_db, Mailer::Manager *_mm)
    {
      return new ManagerImpl(_db,_mm);
    }

  }
}
