#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>
#include "domain.h"

/// forward declared parameter type
class DB;

namespace Register
{
  /// classification for register object handle 
  enum CheckHandleClass {
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
  struct CheckHandle {
    std::string newHandle; ///< transformed handle if appropriate
    CheckHandleClass handleClass; ///< result of handle classification
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
    /// factory method
    static Manager *create(DB *db);    
  };
};

#endif /*REGISTER_H_*/
