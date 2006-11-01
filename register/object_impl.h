#ifndef OBJECT_IMPL_H_
#define OBJECT_IMPL_H_

#include "object.h"

// this should go to sql builder object
#define SQL_DATE_FILTER(x,colname,member) \
  if (!member.begin().is_special()) \
     x << "AND " << colname << ">='"  \
       <<  to_iso_extended_string(member.begin().date()) \
       << "' "; \
  if (!member.end().is_special()) \
     x << "AND " << colname << "<='"  \
       <<  to_iso_extended_string(member.end().date()) \
       << " 23:59:59' "
#define SQL_HANDLE_FILTER(x,colname,member) \
  if (!member.empty()) \
     x << "AND " << colname << "='" << member << "' "
#define SQL_ID_FILTER(x,colname,member) \
  if (member) \
     x << "AND " << colname << "=" << member << " "



namespace Register
{
  /// Implementation of common register object properties
  class ObjectImpl : virtual public Object
  {
   protected:
    ptime crDate;
    ptime trDate;
    ptime upDate;
    unsigned registrar;
    std::string registrarHandle;
    unsigned createRegistrar;
    std::string createRegistrarHandle;
    unsigned updateRegistrar;
    std::string updateRegistrarHandle;
    std::string authPw;
    std::string roid;
    StatusSet sset;
    bool modified;
   public:
    ObjectImpl();
    ObjectImpl(
      ptime _crDate, ptime _trDate, ptime _upDate,
      unsigned registrar, const std::string registrarHandle,
      unsigned updateRegistrar, const std::string updateRegistrarHandle,
      unsigned createRegistrar, const std::string createRegistrarHandle,
      const std::string& authPw, const std::string roid
    );
    ptime getCreateDate() const;
    ptime getTransferDate() const;
    ptime getUpdateDate() const;
    unsigned getRegistrarId() const;
    const std::string& getRegistrarHandle() const;
    unsigned getUpdateRegistrarId() const;
    const std::string& getUpdateRegistrarHandle() const;
    unsigned getCreateRegistrarId() const;
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
    unsigned registrarFilter;
    std::string registrarHandleFilter;
    unsigned createRegistrarFilter;
    std::string createRegistrarHandleFilter;
    unsigned updateRegistrarFilter;
    std::string updateRegistrarHandleFilter;
    time_period crDateIntervalFilter;
    time_period updateIntervalFilter;
    time_period trDateIntervalFilter;
   public:
    ObjectListImpl();
    virtual void setRegistrarFilter(unsigned registrarId);
    virtual void setRegistrarHandleFilter(
      const std::string& registrarHandle
    );
    virtual void setCrDateIntervalFilter(time_period period);
    virtual void setCreateRegistrarFilter(unsigned registrarId);
    virtual void setCreateRegistrarHandleFilter(
      const std::string& registrarHandle
    );
    virtual void setUpdateIntervalFilter(time_period period);
    virtual void setUpdateRegistrarFilter(unsigned registrarId);
    virtual void setUpdateRegistrarHandleFilter(
      const std::string& registrarHandle
    );
    virtual void setTrDateIntervalFilter(time_period period);
    virtual void clear(); 
  }; // class ObjectListImpl
   
} // namespace register

#endif
