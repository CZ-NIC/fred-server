#ifndef BLACKLIST_H_
#define BLACKLIST_H_

#include <string>
#include "src/libfred/exceptions.hh"
#include "src/deprecated/util/dbsql.hh"

namespace LibFred
{
  namespace Domain
  {
    /// domain registration blacklist
    class Blacklist {
     public:
      virtual ~Blacklist() {}
      /// check domain against actual blacklist
      virtual bool checkDomain(const std::string& fqdn) const 
        = 0;
      /// factory function
      static Blacklist *create(DBSharedPtr db);
    };
  };
};

#endif
