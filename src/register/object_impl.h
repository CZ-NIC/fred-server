#ifndef OBJECT_IMPL_H_
#define OBJECT_IMPL_H_

#include "object.h"
#include "common_impl.h"
#include "db/dbs.h"

class DB;

namespace Register
{
  /// Implementation of simple status object
  class StatusImpl : virtual public Status
  {
    TID id;
    ptime timeFrom;
    ptime timeTo;
   public:
    StatusImpl(TID _id, ptime _timeFrom, ptime _timeTo);
    ~StatusImpl();
    virtual TID getStatusId() const;
    virtual ptime getFrom() const;
    virtual ptime getTo() const;
  };
  /// Implementation of common register object properties
  class ObjectImpl : public CommonObjectImpl, virtual public Object
  {
   protected:
    ptime crDate;
    ptime trDate;
    ptime upDate;
    date erDate;
    TID registrar;
    std::string registrarHandle;
    TID createRegistrar;
    std::string createRegistrarHandle;
    TID updateRegistrar;
    std::string updateRegistrarHandle;
    std::string authPw;
    std::string roid;
    typedef std::vector<StatusImpl> StatusList;
    StatusList slist;
   public:
    ObjectImpl();
    ObjectImpl(
      TID _id, ptime _crDate, ptime _trDate, ptime _upDate, date _erDate, 
      TID registrar, const std::string registrarHandle,
      TID updateRegistrar, const std::string updateRegistrarHandle,
      TID createRegistrar, const std::string createRegistrarHandle,
      const std::string& authPw, const std::string roid
    );
    ptime getCreateDate() const;
    ptime getTransferDate() const;
    ptime getUpdateDate() const;
    date getDeleteDate() const;
    TID getRegistrarId() const;
    const std::string& getRegistrarHandle() const;
    TID getUpdateRegistrarId() const;
    const std::string& getUpdateRegistrarHandle() const;
    TID getCreateRegistrarId() const;
    const std::string& getCreateRegistrarHandle() const;
    const std::string& getAuthPw() const;
    void setAuthPw(const std::string& auth);
    const std::string& getROID() const;
    unsigned getStatusCount() const;
    const Status* getStatusByIdx(unsigned idx) const;
    virtual void insertStatus(TID id, ptime timeFrom, ptime timeTo);
  }; // class ObjectImpl
  
  /// Implementation of common register object list properties
  class ObjectListImpl : public CommonListImpl, virtual public ObjectList
  {
   protected:
    TID registrarFilter;
    std::string registrarHandleFilter;
    TID createRegistrarFilter;
    std::string createRegistrarHandleFilter;
    TID updateRegistrarFilter;
    std::string updateRegistrarHandleFilter;
    time_period crDateIntervalFilter;
    time_period updateIntervalFilter;
    time_period trDateIntervalFilter;
    typedef std::vector<StatusFilter> StatusFilterList;
    StatusFilterList sflist;
    bool nonHandleFilterSet; // set if other then handle(fqdn) filtr is set
   public:
    ObjectListImpl(DB *db);
    virtual void clearFilter();
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
    virtual void addStateFilter(TID state, bool stateIsOn);
    virtual void clearStateFilter(TID state);
    void reload(const char *handle = NULL, int type=0) throw (SQL_ERROR);
    void reload2(DBase::Connection* _conn);
  }; // class ObjectListImpl
   
} // namespace register

#endif
