#include "object_impl.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     Register::ObjectImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectImpl::ObjectImpl(
  ptime _crDate, ptime _trDate, ptime _upDate,
  unsigned _registrar, const std::string _registrarHandle,
  unsigned _createRegistrar, const std::string _createRegistrarHandle,
  unsigned _updateRegistrar, const std::string _updateRegistrarHandle,
  const std::string& _authPw, const std::string _roid
) :
 crDate(_crDate), trDate(_trDate), upDate(_upDate), 
 registrar(_registrar), registrarHandle(_registrarHandle),
 createRegistrar(_createRegistrar), 
 createRegistrarHandle(_createRegistrarHandle),
 updateRegistrar(_updateRegistrar), 
 updateRegistrarHandle(_updateRegistrarHandle),
 authPw(_authPw), roid(_roid)
{
}

Register::ObjectImpl::ObjectImpl()
  : crDate(not_a_date_time), trDate(not_a_date_time), upDate(not_a_date_time),
    registrar(0), createRegistrar(0), updateRegistrar(0), modified(true)
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

unsigned 
Register::ObjectImpl::getRegistrarId() const
{
  return registrar;
}

const std::string&
Register::ObjectImpl::getRegistrarHandle() const
{
  return registrarHandle;
}

unsigned 
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

unsigned 
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
//     Register::ObjectListImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Register::ObjectListImpl::ObjectListImpl() :
 registrarFilter(0), 
 createRegistrarFilter(0), 
 updateRegistrarFilter(0),
 crDateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
 updateIntervalFilter(ptime(neg_infin),ptime(pos_infin)),
 trDateIntervalFilter(ptime(neg_infin),ptime(pos_infin))
{
}

void 
Register::ObjectListImpl::setRegistrarFilter(unsigned registrarId)
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
Register::ObjectListImpl::setCreateRegistrarFilter(unsigned registrarId)
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
Register::ObjectListImpl::setUpdateRegistrarFilter(unsigned registrarId)
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
Register::ObjectListImpl::clear()
{
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
