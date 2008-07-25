#ifndef NSSET_H_
#define NSSET_H_

#include <string>
#include <vector>
#include "object.h"
#include "exceptions.h"
#include "zone.h"
#include "db/manager.h"
#include "model/model_filters.h"

/// forward declared parameter type 
class DB;

namespace Register {
namespace NSSet {

/// member identification (i.e. for sorting)
enum MemberType {
  MT_HANDLE, ///< nsset identificator
  MT_CRDATE, ///< create date
  MT_ERDATE, ///< delete date
  MT_REGISTRAR_HANDLE, ///< registrar handle
};


class Host {
public:
  /// public destructor
  virtual ~Host() {
  }
  /// return hostname of nameserver
  virtual const std::string getName() const = 0;
  /// return hostname of nameserver translated from IDN encoded form
  virtual const std::string getNameIDN() const = 0;
  /// return count of address
  virtual unsigned getAddrCount() const = 0;
  /// return address by index
  virtual std::string getAddrByIdx(unsigned idx) const = 0;
};


class NSSet : virtual public Register::Object {
public:
  /// public destructor
  virtual ~NSSet() {
  }
  /// return nsset handle
  virtual const std::string& getHandle() const = 0;
  /// return technical check level
  virtual const unsigned getCheckLevel() const = 0;
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
class List : virtual public ObjectList {
public:
  virtual ~List() {
  }
  /// get detail of loaded nsset
  virtual NSSet *getNSSet(unsigned idx) const = 0;
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
  /// testing new reload function
  virtual void reload(Database::Filters::Union &uf, Database::Manager* dbm) = 0;
  /// clear filter data
  virtual void clearFilter() = 0;
  /// sort by column
  virtual void sort(MemberType _member, bool _asc) = 0;
};


/// main entry class
class Manager {
public:
  /// destructor 
  virtual ~Manager() {
  }
  /// return list of nssets
  virtual List *createList() = 0;
  /// type for check 
  enum CheckAvailType {
    /// handle cannot be contact
    CA_INVALID_HANDLE,
    /// handle is already registred
    CA_REGISTRED,
    /// handle is in protected period
    CA_PROTECTED,
    /// handle is free for registration
    CA_FREE
  };
  /// check proper format of handle
  virtual bool checkHandleFormat(const std::string& handle) const = 0;
  /// check possibilities for registration
  virtual CheckAvailType
      checkAvail(const std::string& handle,
                 NameIdPair& conflict,
                 bool lock = false) const throw (SQL_ERROR) = 0;
  /// check FQDN of host, should be hidden and not exported in manager API
  /** \return 0=OK, 1=invalid name, 2=glue and invalid zone */
      virtual unsigned checkHostname(const std::string& hostname, 
      															 bool glue, 
      															 bool allowIDN = false) const = 0;
  /// factory method
  static Manager *create(DB *db, Zone::Manager *zman, bool restrictedHandle);
};

} // namespace NSSet
} // namespace Register

#endif /* NSSET_H_ */
