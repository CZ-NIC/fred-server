#include "object_impl.h"
#include "dbsql.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::ObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectImpl::ObjectImpl(
  TID _id, ptime _crDate, ptime _trDate, ptime _upDate,
  TID _registrar, const std::string _registrarHandle,
  TID _createRegistrar, const std::string _createRegistrarHandle,
  TID _updateRegistrar, const std::string _updateRegistrarHandle,
  const std::string& _authPw, const std::string _roid
) :
  CommonObjectImpl(_id), crDate(_crDate), trDate(_trDate), upDate(_upDate), 
  registrar(_registrar), registrarHandle(_registrarHandle),
  createRegistrar(_createRegistrar), 
  createRegistrarHandle(_createRegistrarHandle),
  updateRegistrar(_updateRegistrar), 
  updateRegistrarHandle(_updateRegistrarHandle),
  authPw(_authPw), roid(_roid)
{
}

Register::ObjectImpl::ObjectImpl()
  : CommonObjectImpl(0), 
    crDate(not_a_date_time), trDate(not_a_date_time), 
    upDate(not_a_date_time),
    registrar(0), createRegistrar(0), updateRegistrar(0)
{
}

ptime 
Register::ObjectImpl::getCreateDate() const
{
  return crDate;
}

ptime 
Register::ObjectImpl::getTransferDate() const
{
  return trDate;
}

ptime 
Register::ObjectImpl::getUpdateDate() const
{
  return upDate;
}

Register::TID
Register::ObjectImpl::getRegistrarId() const
{
  return registrar;
}

const std::string&
Register::ObjectImpl::getRegistrarHandle() const
{
  return registrarHandle;
}

Register::TID
Register::ObjectImpl::getUpdateRegistrarId() const
{
  return updateRegistrar;
}

const std::string&
Register::ObjectImpl::getCreateRegistrarHandle() const
{
  return createRegistrarHandle;
}

const std::string&
Register::ObjectImpl::getUpdateRegistrarHandle() const
{
  return updateRegistrarHandle;
}

Register::TID
Register::ObjectImpl::getCreateRegistrarId() const
{
  return createRegistrar;
}

const std::string& 
Register::ObjectImpl::getAuthPw() const
{
  return authPw;
}

void 
Register::ObjectImpl::setAuthPw(const std::string& authPw)
{
  this->authPw = authPw;
}

const std::string& 
Register::ObjectImpl::getROID() const
{
  return roid;
}

const Register::Object::StatusSet& 
Register::ObjectImpl::getStatusSet() const
{
  return sset;
}

bool 
Register::ObjectImpl::insertStatus(StatusElement element)
{
  modified = true;
  return sset.insert(element).second;
}

bool 
Register::ObjectImpl::deleteStatus(StatusElement element)
{
  modified = true;
  return sset.erase(element);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::CommonListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectListImpl::ObjectListImpl(DB *_db) :
  CommonListImpl(_db),
  registrarFilter(0), 
  createRegistrarFilter(0), 
  updateRegistrarFilter(0),
  crDateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
  updateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
  trDateIntervalFilter(ptime(neg_infin),ptime(pos_infin))
{
}

void 
Register::ObjectListImpl::setRegistrarFilter(TID registrarId)
{
  registrarFilter = registrarId;
}

void 
Register::ObjectListImpl::setRegistrarHandleFilter(
  const std::string& registrarHandle
)
{
  registrarHandleFilter = registrarHandle;
}

void 
Register::ObjectListImpl::setCrDateIntervalFilter(time_period period)
{
  crDateIntervalFilter = period;
}

void 
Register::ObjectListImpl::setCreateRegistrarFilter(TID registrarId)
{
  createRegistrarFilter = registrarId;
}

void 
Register::ObjectListImpl::setCreateRegistrarHandleFilter(
  const std::string& registrarHandle
)
{
  createRegistrarHandleFilter = registrarHandle;
}

void 
Register::ObjectListImpl::setUpdateIntervalFilter(time_period period)
{
  updateIntervalFilter = period;
}

void 
Register::ObjectListImpl::setUpdateRegistrarFilter(TID registrarId)
{
  updateRegistrarFilter = registrarId;
}

void 
Register::ObjectListImpl::setUpdateRegistrarHandleFilter(
  const std::string& registrarHandle
)
{
  updateRegistrarHandleFilter = registrarHandle;
}

void 
Register::ObjectListImpl::setTrDateIntervalFilter(time_period period)
{
  trDateIntervalFilter = period;
}

void
Register::ObjectListImpl::clearFilter()
{
  CommonListImpl::clearFilter();
  crDateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
  updateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
  trDateIntervalFilter = time_period(ptime(neg_infin),ptime(pos_infin));
  registrarFilter = 0;
  createRegistrarFilter = 0;
  updateRegistrarFilter = 0;
  registrarHandleFilter = "";
  createRegistrarHandleFilter = ""; 
  updateRegistrarHandleFilter = "";
}
