#ifndef NOTIFY_H_D320FB5E3AEC489188346088EA8FD72E
#define NOTIFY_H_D320FB5E3AEC489188346088EA8FD72E

#include "src/fredlib/mailer.h"
#include "src/fredlib/contact.h"
#include "src/fredlib/nsset.h"
#include "src/fredlib/domain.h"
#include "src/fredlib/keyset.h"
#include "src/fredlib/documents.h"
#include "src/fredlib/exceptions.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/file_transferer.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/old_utils/dbsql.h"


namespace Fred
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
      virtual void generateLetters(unsigned item_count_limit) = 0;
      /// factory method
      static Manager *create(
        DBSharedPtr db,
        Mailer::Manager *mm,
        Contact::Manager *cm,
        Nsset::Manager *nm,
        Keyset::Manager *km,
        Domain::Manager *dm,
        Document::Manager *docm,
        Registrar::Manager *rm,
        Messages::ManagerPtr msgm
      );
    };
  }
}

#endif
