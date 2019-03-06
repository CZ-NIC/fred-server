/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef OBJECT_IMPL_HH_BE628EFBBBDE4C5BB1082D3C1048D07D
#define OBJECT_IMPL_HH_BE628EFBBBDE4C5BB1082D3C1048D07D

#include "src/deprecated/libfred/object.hh"
#include "src/deprecated/libfred/common_impl.hh"
#include "libfred/db_settings.hh"

class DB;
namespace LibFred {

  class SortByHistoryId {
  public:
    bool operator()(CommonObject *_left, CommonObject *_right) const {
      if (_left->getId() == _right->getId()) {
        Object *l_casted = dynamic_cast<Object*>(_left);
        Object *r_casted = dynamic_cast<Object*>(_right);

        if (l_casted == 0 || r_casted == 0) {
          /* this should never happen */
          throw std::bad_cast();
        }

        return l_casted->getHistoryId() <= r_casted->getHistoryId();
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
    LibFred::TID getHistoryIdFrom() const;
    LibFred::TID getHistoryIdTo() const;
  };
  /// Implementation of common registry object properties
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

    Database::ID request_id;
    Database::DateTime request_start_time;

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
    Database::ID getRequestId() const; 
    Database::DateTime getRequestStartTime() const;
    void setRequestId(const Database::ID& _id, const Database::DateTime& _start_time);

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
  
  /// Implementation of common registry object list properties
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
    ObjectListImpl(DBSharedPtr db);
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
    void reload(const char *handle = NULL, int type=0);
    void reload(bool _history = false);

    void resetHistoryIDSequence();
    Object* findHistoryIDSequence(const Database::ID& _history_id);
    void deleteDuplicatesId();
  }; // class ObjectListImpl
   
} // namespace LibFred

#endif
