#ifndef BLACKLIST_HH_50696502AC5F4D9082557C36C1ACB7F7
#define BLACKLIST_HH_50696502AC5F4D9082557C36C1ACB7F7

#include <string>
#include "src/deprecated/libfred/exceptions.hh"
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
