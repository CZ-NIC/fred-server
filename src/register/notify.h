#ifndef _NOTIFY_H_
#define _NOTIFY_H_

#include "mailer.h"
#include "contact.h"
#include "nsset.h"
#include "domain.h"
#include "documents.h"
#include "exceptions.h"
#include "registrar.h"

class DB;
namespace Register
{
  namespace Notify
  {
    class Manager
    {
     public:
      virtual ~Manager() {}
      /// notify contacts about state changes  
      virtual void notifyStateChanges(
    	const std::string& exceptList,
    	unsigned limit,
    	std::ostream *debugOutput,
        bool useHistory
      ) throw (SQL_ERROR) = 0;
      virtual void generateLetters() throw (SQL_ERROR) = 0;
      /// factory method
      static Manager *create(
        DB *db, 
        Mailer::Manager *mm,
        Contact::Manager *cm,
        NSSet::Manager *nm,
        Domain::Manager *dm,
        Document::Manager *docm,
        Registrar::Manager *rm
      );
    };
  }
}

#endif
