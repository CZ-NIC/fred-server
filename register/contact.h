#ifndef CONTACT_H_
#define CONTACT_H_

#include <string>
#include <vector>
#include "object.h"
#include "exceptions.h"

/// forward declared parameter type 
class DB;

namespace Register
{
  namespace Contact 
  {
    class Contact : virtual public Register::Object
    {
     public:
      /// public destructor
      virtual ~Contact() {}
      /// return id of contact
      virtual unsigned getId() const = 0;
      /// return contact handle
      virtual const std::string& getHandle() const = 0;
      /// return contact name
      virtual const std::string& getName() const = 0;
    };
    /// domain list
    class List : virtual public ObjectList
    {
     public:
      virtual ~List() {}
      /// get detail of loaded contact
      virtual Contact *get(unsigned idx) const = 0;
      /// reload list with current filter
      virtual void reload() throw (SQL_ERROR) = 0;
    };
    /// main entry class
    class Manager
    {
     public:
      /// destructor 
      virtual ~Manager() {}
      /// Return list of domains
      virtual List *getList() = 0;
      /// factory method
      static Manager *create(DB *db);
    };
  } // namespace Contact
} // namespace Register

#endif /* CONTACT_H_ */
