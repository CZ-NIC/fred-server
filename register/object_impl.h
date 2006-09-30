#ifndef OBJECT_IMPL_H_
#define OBJECT_IMPL_H_

#include "object.h"

namespace Register
{
  class ObjectImpl : virtual public Object
  {
   protected:
    ptime crDate;
    ptime trDate;
    ptime upDate;
    unsigned registrar;
    std::string registrarHandle;
    unsigned updateRegistrar;
    unsigned createRegistrar;
    std::string authPw;
    std::string roid;
    StatusSet sset;
    bool modified;
   public:
    ObjectImpl();
    ObjectImpl(
      ptime _crDate, unsigned registrar, const std::string registrarHandle
    );
    ptime getCreateDate() const;
    ptime getTransferDate() const;
    ptime getUpdateDate() const;
    unsigned getRegistrarId() const;
    const std::string& getRegistrarHandle() const;
    unsigned getUpdateRegistrarId() const;
    unsigned getCreateRegistrarId() const;
    const std::string& getAuthPw() const;
    void setAuthPw(const std::string& auth);
    const std::string& getROID() const;
    const StatusSet& getStatusSet() const;
    bool insertStatus(StatusElement element);
    bool deleteStatus(StatusElement element);
  }; // class ObjectImpl
} // namespace register

#endif
