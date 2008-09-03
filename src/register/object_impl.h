#ifndef OBJECT_IMPL_H_
#define OBJECT_IMPL_H_

#include "object.h"
#include "common_impl.h"
#include "db/manager.h"

class DB;
namespace Register {

  class SortByHistoryId {
  public:
    bool operator()(CommonObject *_left, CommonObject *_right) const {
      if (_left->getId() == _right->getId()) {
        return dynamic_cast<Object*>(_left)->getHistoryId() <= dynamic_cast<Object*>(_right)->getHistoryId();
      }
      return _left->getId() < _right->getId();
    }
  };

  /// Implementation of simple status object
  class StatusImpl : virtual public Status
  {
    TID id;
    TID status_id;
    ptime timeFrom;
    ptime timeTo;
    TID ohid_from;
    TID ohid_to;
   public:
    StatusImpl(const TID& _id, 
               const TID& _status_id,
               const ptime& _timeFrom, 
               const ptime& _timeTo, 
               const TID& _ohid_from,
               const TID& _ohid_to);
    ~StatusImpl();
    virtual TID getId() const;
    virtual TID getStatusId() const;
    virtual ptime getFrom() const;
    virtual ptime getTo() const;
    Register::TID getHistoryIdFrom() const;
    Register::TID getHistoryIdTo() const;
  };
  /// Implementation of common register object properties
  class ObjectImpl : public CommonObjectImpl, virtual public Object
  {
   protected:
    Database::ID history_id;
    ptime crDate;
    ptime trDate;
    ptime upDate;
    ptime erDate;
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

    Database::ID action_id;
    Database::DateTime action_start_time;

   public:
    ObjectImpl();
    ObjectImpl(
      TID _id, const Database::ID& _history_id, ptime _crDate, ptime _trDate, ptime _upDate, ptime _erDate, 
      TID registrar, const std::string registrarHandle,
      TID updateRegistrar, const std::string updateRegistrarHandle,
      TID createRegistrar, const std::string createRegistrarHandle,
      const std::string& authPw, const std::string roid
    );

    Database::ID getHistoryId() const;
    Database::ID getActionId() const; 
    Database::DateTime getActionStartTime() const;
    void setAction(const Database::ID& _id, const Database::DateTime& _start_time);

    ptime getCreateDate() const;
    ptime getTransferDate() const;
    ptime getUpdateDate() const;
    ptime getDeleteDate() const;
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
    virtual void insertStatus(const StatusImpl& _state);
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

    int ptr_history_idx_;

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
    void reload(Database::Connection* _conn);

    void resetHistoryIDSequence();
    Object* findHistoryIDSequence(const Database::ID& _history_id);
    void deleteDuplicatesId();
  }; // class ObjectListImpl
   
} // namespace register

#endif
