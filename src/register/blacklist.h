#ifndef BLACKLIST_H_
#define BLACKLIST_H_

#include <string>
#include "exceptions.h"

// forward declaration
class DB;

namespace Register
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
      static Blacklist *create(DB *db);
    };
  };
};

#endif
