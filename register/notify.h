#ifndef _NOTIFY_H_
#define _NOTIFY_H_

#include "mailer.h"
#include "contact.h"
#include "nsset.h"
#include "domain.h"
#include "exceptions.h"

class DB;
namespace Register
{
  namespace Notify
  {
    class Manager
    {
     public:
      /// notify contacts about state changes  
      virtual void notifyStateChanges(
    	const std::string& exceptList,
    	unsigned limit,
    	std::ostream *debugOutput
      ) throw (SQL_ERROR) = 0;
      /// factory method
      static Manager *create(
        DB *db, 
        Mailer::Manager *mm,
        Contact::Manager *cm,
        NSSet::Manager *nm,
        Domain::Manager *dm
      );
    };
  }
}

#endif
