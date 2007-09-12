#ifndef REGISTRAR_H_
#define REGISTRAR_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>

#include "types.h"
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
      virtual TID getId() const = 0;
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
      /// Get registrar organization      
      virtual const std::string& getOrganization() const = 0;
      /// Set registrar organization
      virtual void setOrganization(const std::string& _organization) = 0;
      /// Get registrar street part 1
      virtual const std::string& getStreet1() const = 0;
      /// Set registrar street part 1
      virtual void setStreet1(const std::string& _street1) = 0;
      /// Get registrar street part 2
      virtual const std::string& getStreet2() const = 0;
      /// Set registrar street part 2
      virtual void setStreet2(const std::string& _street2) = 0;
      /// Get registrar street part 3
      virtual const std::string& getStreet3() const = 0;
      /// Set registrar street part 3
      virtual void setStreet3(const std::string& _street3) = 0;
      /// Get registrar city
      virtual const std::string& getCity() const = 0;
      /// Set registrar city
      virtual void setCity(const std::string& _city) = 0;
      /// Get registrar state or province 
      virtual const std::string& getProvince() const = 0;
      /// Set registrar state or province
      virtual void setProvince(const std::string& _province) = 0;
      /// Get registrar postal code
      virtual const std::string& getPostalCode() const = 0;
      /// Set registrar postal code
      virtual void setPostalCode(const std::string& _postalCode) = 0;
      /// Get registrar country code
      virtual const std::string& getCountry() const = 0;
      /// Set registrar country code
      virtual void setCountry(const std::string& _country) = 0;
      /// Get registrar telephone number
      virtual const std::string& getTelephone() const = 0;
      /// Set registrar telephone number
      virtual void setTelephone(const std::string& _telephone) = 0;
      /// Get registrar fax number
      virtual const std::string& getFax() const = 0;
      /// Set registrar fax number
      virtual void setFax(const std::string& _fax) = 0;
      /// Get registrar email
      virtual const std::string& getEmail() const = 0;
      /// Set registrar email
      virtual void setEmail(const std::string& _email) = 0;
      /// Get actual credit 
      virtual unsigned long getCredit() const = 0;
      /// Create new ACL record
      virtual ACL* newACL() = 0;
      /// Return ACL list size
      virtual unsigned getACLSize() const = 0;
      /// Return ACL list member by index
      virtual ACL* getACL(unsigned idx) const = 0;
      /// Delete ACL or do nothing
      virtual void deleteACL(unsigned idx) = 0;
      /// Clear ACL list
      virtual void clearACLList() = 0;
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
      /// Filter in id
      virtual void setIdFilter(TID id) = 0;
      /// Filter in handle
      virtual void setHandleFilter(const std::string& handle) = 0;
      /// Filter in name
      virtual void setNameFilter(const std::string& name) = 0;
      /// Filter for zone
      virtual void setZoneFilter(const std::string& zone) = 0;
      /// Reload actual list of registrars
      virtual void reload() throw (SQL_ERROR) = 0;
      /// Return size of list
      virtual unsigned size() const = 0;
      /// Get registrar detail object by list index
      virtual const Registrar* get(unsigned idx) const = 0;
      /// Get registrar detail object by list index for update
      virtual Registrar* get(unsigned idx) = 0;
      /// Create new registrar in list
      virtual Registrar* create() = 0;
      /// clear filter data
      virtual void clearFilter() = 0;      
    };
    
    /// Type for specification of EPP action result in filter for actions
    enum EPPActionResultFilter {
      EARF_OK, ///< EPP action result begins with 1
      EARF_FAIL, ///< EPP action result is greater than 1 
      EARF_ALL ///< All EPP Actions
    };
    
    /// Action made by registrar through EPP
    class EPPAction 
    {
     protected:
      /// Protected destructor, object is managed by EPPActionList
      virtual ~EPPAction() {}
     public:
      virtual TID getId() const = 0; 
      /// Return id of session action is part of
      virtual TID getSessionId() const = 0;
      /// Return type of actoion
      virtual unsigned getType() const = 0;
      /// Return type name of action
      virtual const std::string& getTypeName() const = 0;
      /// Return handle of main object in action
      virtual const std::string& getHandle() const = 0;
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
      /// Return result message
      virtual std::string getResultStatus() const = 0;
      /// Return handle of registrar who made this action
      virtual const std::string& getRegistrarHandle() const = 0;
    };
    
    /// List of EPPAction objects
    class EPPActionList
    {
     public:
      /// Public destructor, user is responsible for destruction
      virtual ~EPPActionList() {}
      /// Set filter for action with id
      virtual void setIdFilter(TID id) = 0;
      /// Set filter for session action is part of
      virtual void setSessionFilter(TID sessionId) = 0;
      /// Set filter for registrar who performed action
      virtual void setRegistrarFilter(TID registrarId) = 0;
      /// Set filter for registrar who performed action
      virtual void setRegistrarHandleFilter(
        const std::string& registrarHandle
      ) = 0;
      /// Set filter for time interval of performing acion 
      virtual void setTimePeriodFilter(const time_period& period) = 0;
      /// Set filter for action type
      virtual void setTypeFilter(unsigned typeId) = 0;
      /// Set filter for concrete return code
      virtual void setReturnCodeFilter(unsigned returnCodeId) = 0;
      /// Set filter for simple result classification
      virtual void setResultFilter(EPPActionResultFilter result) = 0;
      /// Set filter for handle in XML
      virtual void setHandleFilter(const std::string& handle) = 0;
      /// Filter in xml
      virtual void setXMLFilter(const std::string& xml) = 0;
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
      /// clear filter data
      virtual void clearFilter() = 0;      
    };
    
    /// Detail about EPP session
    class EPPSession
    {
     protected:
      /// protected destructor, EPPSession is managed by EPPSessionList
      virtual ~EPPSession() {}
     public: 
      /// Return id of this session
      virtual TID getId() const = 0;
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
      /// Return count of EPP action types
      virtual unsigned getEPPActionTypeCount() = 0;
      /// Return EPP action type by index
      virtual const std::string& getEPPActionTypeByIdx(
        unsigned idx
      ) const throw (NOT_FOUND) = 0;
      virtual bool checkHandle(const std::string) const 
        throw (SQL_ERROR) = 0;
      /// Factory method
      static Manager *create(DB *db);
    };
    
  };

};

#endif /*REGISTRAR_H_*/
