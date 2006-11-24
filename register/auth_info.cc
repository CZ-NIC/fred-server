#include "auth_info.h"
#include "dbsql.h"

namespace Register
{
  namespace AuthInfoRequest
  {
    class ManagerImpl : virtual public Manager
    {
      DB *db;
      Mailer::Manager *mm;
     public:
      ManagerImpl(DB *_db, Mailer::Manager *_mm) : db(_db), mm(_mm)
      {}
      unsigned long createRequest(
        unsigned long objectId,
        RequestType requestType,
        unsigned long eppActionId,
        const std::string& requestReason,
        const std::string& emailToAnswer
      ) throw (BAD_EMAIL, OBJECT_NOT_FOUND, ACTION_NOT_FOUND, SQL_ERROR)
      {
        processRequest(0);
        return 0;
      }
      void processRequest(unsigned id)
      {
        Mailer::Parameters params;
        params["wwwpage"] = "http://www.nic.cz/";
        params["reqdate"] = "27.11.2006";
        params["reqid"] = "007";
        params["handle"] = "domena.cz";
        params["authinfo"] = "tajne-heslo";
        Mailer::Handles handles;
        handles.push_back("HANDLE");
        mm->sendEmail(
          "jaromir.talir@nic.cz","jaromir.talir@nic.cz","subject",
          "sendauthinfo_pif",params,handles
        );
      } 
    }; // ManagerImpl
    Manager *Manager::create(DB *db, Mailer::Manager *mm)
    {
      return new ManagerImpl(db,mm);
    }
  }; // AuthInfoRequest
}; // Register
