#ifndef BLACKLIST_H_
#define BLACKLIST_H_

#include <string>
#include "exceptions.h"
#include "old_utils/dbsql.h"

namespace Fred
{
  namespace Domain
  {
    /// domain registration blacklist
    class Blacklist {
     public:
      virtual ~Blacklist() {}
      /// check domain against actual blacklist
      virtual bool checkDomain(const std::string& fqdn) const 
        throw (SQL_ERROR) = 0;                   
      /// factory function
      static Blacklist *create(DBSharedPtr db);
    };
  };
};

#endif
