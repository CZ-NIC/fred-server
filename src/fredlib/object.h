#ifndef OBJECT_H_
#define OBJECT_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/formatters.hpp>

#include <string>
#include <set>
#include <map>

#include "common_object.h"
#include "db_settings.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Fred {

/// Pair of id and name of some object
struct NameIdPair {
  std::string name;
  TID id;
};

/// status attributes
class StatusDesc {
protected:
  /// protected destructor
  virtual ~StatusDesc() {
  }

public:
  /// exception thrown in case of invalid language specification
  struct BAD_LANG : public std::runtime_error {
      BAD_LANG() : std::runtime_error("bad lang"){}
  };
  /// return id of status
  virtual TID getId() const = 0;
  /// return code name of status
  virtual const std::string& getName() const = 0;
  /// return flag if status is exported to public
  virtual bool getExternal() const = 0;
  /// return description in selected language
  virtual const std::string& getDesc(const std::string& lang) const
      throw (BAD_LANG) = 0;
  /// return flag if status is supported by given type
  virtual bool isForType(short type) const = 0;
};


/// Request for inclusion status in filter
struct StatusFilter {
  /// id of flag 
  TID stateId;
  /// wheter it has to be On (true) or Off (false)
  bool stateIsOn;
};


/// State elements that can be set to object are managed by their id 
class Status {
protected:
  virtual ~Status() {
  }

public:
  virtual TID getId() const = 0;
  /// Return id of status 
  virtual TID getStatusId() const = 0;
  /// Return timestamp when object entered this state
  virtual ptime getFrom() const = 0;
  /// Return timestamp when object leaved this state
  virtual ptime getTo() const = 0;
  /// Return object history id from state was active
  virtual Fred::TID getHistoryIdFrom() const = 0;
  /// Return object history id to state was active
  virtual Fred::TID getHistoryIdTo() const = 0;
};


/// Common ancestor for all types managed in registry
class Object : virtual public CommonObject {
public:
  virtual ~Object() {
  }

  virtual Database::ID getHistoryId() const = 0;
  virtual Database::ID getRequestId() const = 0; 
  virtual Database::DateTime getRequestStartTime() const = 0;
  virtual void setRequestId(const Database::ID& _id, const Database::DateTime& _start_time) = 0;

  /// Return time of object registration
  virtual ptime getCreateDate() const = 0;
  /// Return time of last transfer
  virtual ptime getTransferDate() const = 0;
  /// Return time of last update
  virtual ptime getUpdateDate() const = 0;
  /// Return time of object delete - database field erdate
  virtual ptime getDeleteDate() const = 0;
  /// Return handle of dedicated registrar
  virtual const std::string& getRegistrarHandle() const = 0;
  /// Return id of dedicated registrar
  virtual TID getRegistrarId() const = 0;
  /// Return handle of registrar who made last update
  virtual const std::string& getUpdateRegistrarHandle() const = 0;
  /// Return id of registrar who made last update
  virtual TID getUpdateRegistrarId() const = 0;
  /// Return handle of registrar who created object
  virtual const std::string& getCreateRegistrarHandle() const = 0;
  /// Return id of registrar who created object
  virtual TID getCreateRegistrarId() const = 0;
  /// Return authorization token
  virtual const std::string& getAuthPw() const = 0;
  /// Set authorization token
  virtual void setAuthPw(const std::string& auth) = 0;
  /// Return repository object id
  virtual const std::string& getROID() const = 0;
  /// Return count of status objects in list
  virtual unsigned getStatusCount() const = 0;
  /// Return state by it's index in list
  virtual const Status* getStatusByIdx(unsigned idx) const = 0;
};

class ObjectList : virtual public CommonList {
public:
  /// public destructor
  virtual ~ObjectList() {
  }
  /// set filter for registrar
  virtual void setRegistrarFilter(TID registrarId) = 0;
  /// set filter for registrar handle
  virtual void setRegistrarHandleFilter(const std::string& registrarHandle) = 0;
  /// set filter for period of crDate
  virtual void setCrDateIntervalFilter(time_period period) = 0;
  /// set filter for create registrar
  virtual void setCreateRegistrarFilter(TID registrarId) = 0;
  /// set filter for create registrar handle
  virtual void
      setCreateRegistrarHandleFilter(const std::string& registrarHandle) = 0;
  /// set filter for period of upDate
  virtual void setUpdateIntervalFilter(time_period period) = 0;
  /// set filter for update registrar
  virtual void setUpdateRegistrarFilter(TID registrarId) = 0;
  /// set filter for update registrar handle
  virtual void
      setUpdateRegistrarHandleFilter(const std::string& registrarHandle) = 0;
  /// set filter for period of trDate
  virtual void setTrDateIntervalFilter(time_period period) = 0;
  /// add filter for one of states
  virtual void addStateFilter(TID state, bool stateIsOn) = 0;
  /// clear filter for one of states
  virtual void clearStateFilter(TID state) = 0;
  /// delete duplicate records with same ID
  virtual void deleteDuplicatesId() = 0;
};

}
;

#endif
