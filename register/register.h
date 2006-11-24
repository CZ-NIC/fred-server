#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>
#include "domain.h"
#include "registrar.h"
#include "contact.h"
#include "nsset.h"
#include "auth_info.h"

/// forward declared parameter type
class DB;

namespace Register
{
  /// classification for register object handle 
  enum CheckHandleClass 
  {
    CH_ENUM_BAD_ZONE, ///< handle is enum number not managed in this register 
    CH_ENUM, ///< handle is enum number
    CH_DOMAIN_PART, ///< handle is single string (appended by default domain)
    CH_DOMAIN_BAD_ZONE, ///< hadnle is domain not managed in this register
    CH_DOMAIN_LONG, ///< handle is domain longer then allowed (truncated)
    CH_DOMAIN, ///< handle is domain
    CH_NSSET, ///< handle is nsset
    CH_CONTACT, ///< handle is contact  
    CH_INVALID ///< not a valid handle
  };
  /// return type for checkHandle
  struct CheckHandle 
  {
    std::string newHandle; ///< transformed handle if appropriate
    CheckHandleClass handleClass; ///< result of handle classification
  };
  /// structure with country description
  struct CountryDesc
  {
    std::string cc; ///< country code
    std::string name; ///< country name
  };
  /// main register entry class
  class Manager
  {
   public:
    /// destructor
    virtual ~Manager() {}
    /// classify input handle according register rules 
    virtual void checkHandle(
      const std::string& handle, CheckHandle& ch
    ) const = 0;
    /// return domain manager
    virtual Domain::Manager *getDomainManager() = 0;
    /// return registrar manager
    virtual Registrar::Manager *getRegistrarManager() = 0;
    /// return contact manager
    virtual Contact::Manager *getContactManager() = 0;
    /// return nsset manager
    virtual NSSet::Manager *getNSSetManager() = 0;
    /// return size of country description list
    virtual unsigned getCountryDescSize() const = 0;
    /// return country description by index in list
    virtual const CountryDesc& getCountryDescByIdx(unsigned idx) const 
     throw (NOT_FOUND) = 0;
    /// factory method
    static Manager *create(DB *db);    
  };
};

#endif /*REGISTER_H_*/
