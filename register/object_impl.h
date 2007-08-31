#ifndef OBJECT_IMPL_H_
#define OBJECT_IMPL_H_

#include "object.h"

class DB;

namespace Register
{
  /// Implementation of common register object properties
  class ObjectImpl : virtual public Object
  {
   protected:
    TID id;
    ptime crDate;
    ptime trDate;
    ptime upDate;
    TID registrar;
    std::string registrarHandle;
    TID createRegistrar;
    std::string createRegistrarHandle;
    TID updateRegistrar;
    std::string updateRegistrarHandle;
    std::string authPw;
    std::string roid;
    StatusSet sset;
    bool modified;
   public:
    ObjectImpl();
    ObjectImpl(
      TID _id, ptime _crDate, ptime _trDate, ptime _upDate,
      TID registrar, const std::string registrarHandle,
      TID updateRegistrar, const std::string updateRegistrarHandle,
      TID createRegistrar, const std::string createRegistrarHandle,
      const std::string& authPw, const std::string roid
    );
    TID getId() const;
    ptime getCreateDate() const;
    ptime getTransferDate() const;
    ptime getUpdateDate() const;
    TID getRegistrarId() const;
    const std::string& getRegistrarHandle() const;
    TID getUpdateRegistrarId() const;
    const std::string& getUpdateRegistrarHandle() const;
    TID getCreateRegistrarId() const;
    const std::string& getCreateRegistrarHandle() const;
    const std::string& getAuthPw() const;
    void setAuthPw(const std::string& auth);
    const std::string& getROID() const;
    const StatusSet& getStatusSet() const;
    bool insertStatus(StatusElement element);
    bool deleteStatus(StatusElement element);
  }; // class ObjectImpl
  
  /// Implementation of common register object list properties
  class ObjectListImpl : virtual public ObjectList
  {
   protected:
    TID idFilter;
    TID registrarFilter;
    std::string registrarHandleFilter;
    TID createRegistrarFilter;
    std::string createRegistrarHandleFilter;
    TID updateRegistrarFilter;
    std::string updateRegistrarHandleFilter;
    time_period crDateIntervalFilter;
    time_period updateIntervalFilter;
    time_period trDateIntervalFilter;
    unsigned long long realCount;
    unsigned limitCount;
    bool wcheck; ///< do wildcard expansion in filter handles
    DB *db;
    virtual void makeQuery(
      bool count, bool limit, std::stringstream& sql
    ) const = 0;
   public:
    ObjectListImpl(DB *db);
    virtual void setIdFilter(TID id);
    virtual void setRegistrarFilter(TID registrarId);
    virtual void setRegistrarHandleFilter(
      const std::string& registrarHandle
    );
    virtual void setCrDateIntervalFilter(time_period period);
    virtual void setCreateRegistrarFilter(TID registrarId);
    virtual void setCreateRegistrarHandleFilter(
      const std::string& registrarHandle
    );
    virtual void setUpdateIntervalFilter(time_period period);
    virtual void setUpdateRegistrarFilter(TID registrarId);
    virtual void setUpdateRegistrarHandleFilter(
      const std::string& registrarHandle
    );
    virtual void setTrDateIntervalFilter(time_period period);
    virtual void clear();
    virtual void setLimit(unsigned count);
    virtual unsigned long long getRealCount() const;
    virtual void fillTempTable(bool limit) const throw (SQL_ERROR);
    virtual void makeRealCount() throw (SQL_ERROR);    
    virtual void setWildcardExpansion(bool _wcheck);
    
  }; // class ObjectListImpl
   
} // namespace register

#endif
