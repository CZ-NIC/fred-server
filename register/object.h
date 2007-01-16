#ifndef OBJECT_H_
#define OBJECT_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <set>

#include "types.h"

using namespace boost::posix_time;

namespace Register
{
  /// Pair of id and name of some object
  struct NameIdPair {
    std::string name;
    TID id;
  };  
  /// Status description
  struct StatusDesc
  {
    unsigned id;
    std::string desc;
  };
  /// Request for inclusion status in filter
  struct StatusFlagFilter {
    /// id of flag 
    unsigned flagId;
    /// wheter it has to be On (true) or Off (false)
    bool flagIsOn;
  };
  /// List of filters for status. Not include status is ignored
  typedef std::vector<StatusFlagFilter> StatusFlagListFilter; 
  /// Common ancestor for all types managed in register
  class Object
  {
   public:
    virtual ~Object() {}
    /// State elements that can be set to object are managed by their id 
    typedef unsigned StatusElement;
    /// Set of states
    typedef std::set<StatusElement> StatusSet;
    /// return id of object
    virtual TID getId() const = 0;
    /// Return time of object registration
    virtual ptime getCreateDate() const  = 0;
    /// Return time of last transfer
    virtual ptime getTransferDate() const = 0;
    /// Return time of last update
    virtual ptime getUpdateDate() const = 0;
    /// Return handle of dedicated registrar
    virtual const std::string& getRegistrarHandle() const = 0;
    /// Return id of dedicated registrar
    virtual TID getRegistrarId() const  = 0;
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
    /// Return set of current states
    virtual const StatusSet& getStatusSet() const = 0;
    /// Insert state into status set
    virtual bool insertStatus(StatusElement element) = 0;
    /// Remove state from status set
    virtual bool deleteStatus(StatusElement element) = 0;
  };
  
  class ObjectList {
   protected:
    /// protected destructor
    virtual ~ObjectList() {}
   public:
    /// return count of objects in list
    virtual unsigned getCount() const = 0;
    /// get detail of loaded objects
    virtual Object *get(unsigned idx) const = 0;
    /// set filter for id
    virtual void setIdFilter(TID id) = 0;
    /// set filter for registrar
    virtual void setRegistrarFilter(TID registrarId) = 0;
    /// set filter for registrar handle
    virtual void setRegistrarHandleFilter(
      const std::string& registrarHandle
    ) = 0;
    /// set filter for period of crDate
    virtual void setCrDateIntervalFilter(time_period period) = 0;
    /// set filter for create registrar
    virtual void setCreateRegistrarFilter(TID registrarId) = 0;
    /// set filter for create registrar handle
    virtual void setCreateRegistrarHandleFilter(
      const std::string& registrarHandle
    ) = 0;
    /// set filter for period of upDate
    virtual void setUpdateIntervalFilter(time_period period) = 0;
    /// set filter for update registrar
    virtual void setUpdateRegistrarFilter(TID registrarId) = 0;
    /// set filter for update registrar handle
    virtual void setUpdateRegistrarHandleFilter(
      const std::string& registrarHandle
    ) = 0;
    /// set filter for period of trDate
    virtual void setTrDateIntervalFilter(time_period period) = 0;
    /// clear filter
    virtual void clear() = 0;
  };
  
};

#endif
