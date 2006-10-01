#ifndef OBJECT_H_
#define OBJECT_H_

#include <boost/date_time/posix_time/ptime.hpp>

#include <string>
#include <set>

using namespace boost::posix_time;

namespace Register
{
  /// Common ancestor for all types managed in register
  class Object
  {
   public:
    virtual ~Object() {}
    /// State elements that can be set to object
    enum StatusElement
    {
      CLIENT_TRANSFER_PROHIBITED,
      SERVER_TRANSFER_PROHIBITED,
      CLIENT_DELETE_PROHIBITED,
      SERVER_DELETE_PROHIBITED,
      CLIENT_UPDATE_PROHIBITED,
      SERVER_UPDATE_PROHIBITED
    };
    /// Set of states
    typedef std::set<StatusElement> StatusSet;
    /// Return time of object registration
    virtual ptime getCreateDate() const  = 0;
    /// Return time of last transfer
    virtual ptime getTransferDate() const = 0;
    /// Return time of last update
    virtual ptime getUpdateDate() const = 0;
    /// Return handle of dedicated registrar
    virtual const std::string& getRegistrarHandle() const = 0;
    /// Return id of dedicated registrar
    virtual unsigned getRegistrarId() const  = 0;
    /// Return id of registrar who made last update
    virtual unsigned getUpdateRegistrarId() const = 0;
    /// Return id of registrar who created object
    virtual unsigned getCreateRegistrarId() const = 0;
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
};

#endif
