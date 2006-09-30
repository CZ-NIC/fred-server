#include "object_impl.h"

Register::ObjectImpl::ObjectImpl(
  ptime _crDate, unsigned _registrar, const std::string _registrarHandle
 ) : crDate(_crDate), registrar(_registrar), registrarHandle(_registrarHandle)
{
}

Register::ObjectImpl::ObjectImpl()
  : crDate(not_a_date_time), trDate(not_a_date_time), upDate(not_a_date_time),
    registrar(0), updateRegistrar(0), createRegistrar(0), modified(true)
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
