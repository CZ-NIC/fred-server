#ifndef REGISTRAR_H_
#define REGISTRAR_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>

#include "exceptions.h" 

using namespace boost::posix_time;

/// forward declared parameter type 
class DB;

namespace Register {

  namespace Registrar {
    
    /// Access control attributes of registrar
    class ACL
    {
     protected:
      /// Protected destructor, object is managed by object Registrar
      virtual ~ACL() {}
     public:
      /// Return MD5 of client SSL certificate for EPP communication
      virtual const std::string& getCertificateMD5() const = 0;
      /// Set MD5 of client SSL certificate for EPP communication
      virtual void setCertificateMD5(
        const std::string& newCertificateMD5
      ) = 0;
      /// Return password for EPP login command
      virtual const std::string& getPassword() const = 0;
      /// Set password for EPP login command
      virtual void setPassword(const std::string& newPassword) = 0;
    };

    /// Registrar detail access
    class Registrar 
    {
     public:
      /// Public destructor, user is responsible for object delete
      virtual ~Registrar() {}
      /// Return registrar id
      virtual unsigned getId() const = 0;
      /// Return registrar handle (EPP session login name)
      virtual const std::string& getHandle() const = 0;
      /// Set registrar handle (EPP session login name)
      virtual void setHandle(const std::string& newHandle) = 0;
      /// Get registrar name 
      virtual const std::string& getName() const = 0;
      /// Set registrar name 
      virtual void setName(const std::string& newName) = 0;
      /// Get registrar URL       
      virtual const std::string& getURL() const = 0;
      /// Set registrar URL
      virtual void setURL(const std::string& newURL) = 0;
      /// Return ACL list size
      virtual unsigned getACLSize() const = 0;
      /// Return ACL list member by index
      virtual ACL* getACL(unsigned idx) const = 0;
      /// Save changes to database
      virtual void save() throw (SQL_ERROR) = 0;
    };
    
    /// List of registrar object
    class RegistrarList
    {
     protected:
      /// Protected destructor, object is manager by Manager
      virtual ~RegistrarList() {}
     public:
      /// Filter in handle
      virtual void setHandleFilter(const std::string& handle) = 0;
      /// Filter in name
      virtual void setNameFilter(const std::string& name) = 0;
      /// Reload actual list of registrars
      virtual void reload() throw (SQL_ERROR) = 0;
      /// Return size of list
      virtual unsigned size() const = 0;
      /// Get registrar detail object by list index
      virtual const Registrar* get(unsigned idx) const = 0;
      /// Create new registrar in list
      virtual Registrar* create() = 0;
    };
    
    /// Action made by registrar through EPP
    class EPPAction 
    {
     protected:
      /// Protected destructor, object is managed by EPPActionList
      virtual ~EPPAction() {}
     public:
      /// Return id of session action is part of
      virtual unsigned getSessionId() const = 0;
      /// Return type of actoion
      virtual unsigned getType() const = 0;
      /// Return type name of action
      virtual const std::string& getTypeName() const = 0;
      /// Return time of session start
      virtual const ptime getStartTime() const = 0;
      /// Return server provided transaction id
      virtual const std::string& getServerTransactionId() const = 0;
      /// Return client provided transaction id
      virtual const std::string& getClientTransactionId() const = 0;
      /// Return xml of EPP message
      virtual const std::string& getEPPMessage() const = 0;
      /// Return result of action
      virtual unsigned getResult() const = 0;
      virtual const std::string& getRegistrarHandle() const = 0;
    };
    
    /// List of EPPAction objects
    class EPPActionList
    {
     public:
      /// Public destructor, user is responsible for destruction
      virtual ~EPPActionList() {}
      /// Set filter for session action is part of
      virtual void setSessionFilter(unsigned sessionId) = 0;
      /// Set filter for registrar who performed action
      virtual void setRegistrarFilter(unsigned registrarId) = 0;
      /// Set filter for registrar who performed action
      virtual void setRegistrarHandleFilter(
        const std::string& registrarHandle
      ) = 0;
      /// Set filter for time interval of performing acion 
      virtual void setTimePeriodFilter(const time_period& period) = 0;
      /// Set filter for action type
      virtual void setTypeFilter(unsigned typeId) = 0;
      /// Set filter for return code
      virtual void setReturnCodeFilter(unsigned returnCodeId) = 0;
      /// Set filter for handle in XML
      virtual void setHandleFilter(const std::string& handle) = 0;
      /// Set filter for text type of action
      virtual void setTextTypeFilter(const std::string& textType) = 0;
      /// Set filter for clTRID
      virtual void setClTRIDFilter(const std::string& clTRID) = 0;
      /// set filter for svTRID
      virtual void setSvTRIDFilter(const std::string& svTRID) = 0;      
      /// Reload list according actual filter settings
      virtual void reload() = 0;
      /// Return size of list
      virtual const unsigned size() const = 0;
      /// Return deatil of action by index in list
      virtual const EPPAction* get(unsigned idx) const = 0;
    };
    
    /// Detail about EPP session
    class EPPSession
    {
     protected:
      /// protected destructor, EPPSession is managed by EPPSessionList
      virtual ~EPPSession() {}
     public: 
      /// Return id of this session
      virtual unsigned getId() const = 0;
      /// Return time of login action
      virtual const std::string& getLoginTime() const = 0;
      /// Return private list of actions of this session
      virtual const EPPActionList *getSessionActions() const = 0;
    };
    
    /// Main entry point for Registrar namespace
    class Manager 
    {
     public:
      /// Public destructor, user is responsible for delete
      virtual ~Manager() {}
      /// Return list of registrars
      virtual RegistrarList *getList() = 0;
      /// Return list of EPP actions
      virtual EPPActionList *getEPPActionList() = 0;
      /// Factory method
      static Manager *create(DB *db);
    };
    
  };

};

#endif /*REGISTRAR_H_*/
