#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>

/// forward declared parameter type
class DB;

namespace Register
{
  /// classification for domain handle 
  enum CheckHandleClass {
    CH_INVALID, ///< not a valid handle
    CH_ENUM, ///< handle is enum number
    CH_BAD_ENUM, ///< handle is enum number not managed in this register 
    CH_DOMAIN_PART, ///< handle is subdomain or single string
    CH_BAD_ZONE, ///< zone isn't managed in this register
    CH_DOMAIN, ///< handle is domain
    CH_NSSET, ///< handle is nsset
    CH_CONTACT ///< handle is contact  
  };
  /// return type for checkHandle
  struct CheckHandle {
    std::string newHandle; ///< transformed handle if appropriate
    CheckHandleClass handleClass; ///< result of handle classification
  };
  /// main entry class
  class Manager
  {
   public:
    virtual ~Manager() = 0;
    /// classify input handle according rules 
    virtual void checkHandle(const std::string& handle, CheckHandle& ch) = 0;
    static Manager *create(DB *db);    
  };
};

#endif /*REGISTER_H_*/
