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
    class Host
    {
     public:
      /// public destructor
      virtual ~Host() {}
      /// return id of nsset
      virtual const std::string getName() const = 0;
      /// return count of address
      virtual unsigned getAddrCount() const = 0;
      /// return address by index
      virtual std::string getAddrByIdx(unsigned idx) const = 0;
    };
    class NSSet : virtual public Register::Object
    {
     public:
      /// public destructor
      virtual ~NSSet() {}
      /// return nsset handle
      virtual const std::string& getHandle() const = 0;
      /// return count of admin contacts
      virtual unsigned getAdminCount() const = 0;
      /// return handle of admin contact by index
      virtual std::string getAdminByIdx(unsigned idx) const = 0;
      /// return count of managed hosts
      virtual unsigned getHostCount() const = 0;
      /// return host by index
      virtual const Host *getHostByIdx(unsigned idx) const = 0;
    };
    /// nssets list
    class List : virtual public ObjectList
    {
     public:
      virtual ~List() {}
      /// get detail of loaded nsset
      virtual NSSet *get(unsigned idx) const = 0;
      /// set filter for handle
      virtual void setHandleFilter(const std::string& handle) = 0;
      /// set filter for nameserver hostname
      virtual void setHostNameFilter(const std::string& name) = 0;
      /// set filter for nameserver ip address
      virtual void setHostIPFilter(const std::string& ip) = 0;
      /// set filter for tech admin
      virtual void setAdminFilter(const std::string& handle) = 0;
      /// reload list with current filter
      virtual void reload() throw (SQL_ERROR) = 0;
      /// clear filter data
      virtual void clearFilter() = 0;
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
