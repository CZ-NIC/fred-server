#ifndef NSSET_H_
#define NSSET_H_

#include <string>
#include <vector>
#include "object.h"
#include "exceptions.h"

/// forward declared parameter type 
class DB;

namespace Register
{
  namespace NSSet
  {
    class NSSet : virtual public Register::Object
    {
     public:
      /// public destructor
      virtual ~NSSet() {}
      /// return id of nsset
      virtual unsigned getId() const = 0;
      /// return nsset handle
      virtual const std::string& getHandle() const = 0;
    };
    /// domain list
    class List
    {
     public:
      virtual ~List() {}
      /// return count of nssets in list
      virtual unsigned getCount() const = 0;
      /// get detail of loaded nsset
      virtual NSSet *get(unsigned idx) const = 0;
      /// set filter for registrar
      virtual void setRegistrarFilter(unsigned registrarId) = 0;
      /// reload list with current filter
      virtual void reload() throw (SQL_ERROR) = 0;
    };
    /// main entry class
    class Manager
    {
     public:
      /// destructor 
      virtual ~Manager() {}
      /// return list of nssets
      virtual List *getList() = 0;
      /// factory method
      static Manager *create(DB *db);
    };
  } // namespace NSSet
} // namespace Register

#endif /* NSSET_H_ */
