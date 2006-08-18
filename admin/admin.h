#ifndef ADMIN_H_
#define ADMIN_H_

namespace Admin
{
  /// classification for domain handle 
  enum CheckHandleClass {
    CH_INVALID, ///< not a valid handle
    CH_ENUM, ///< handle is enum number 
    CH_SUBDOMAIN, ///< handle is subdomain
    CH_BAD_ZONE, /// < zone isn't managed in this register
    CH_DOMAIN, /// handle is domain
    CH_NSSET, /// handle is nsset
    CH_CONTACT /// handle is contact  
  };
  /// return type for checkHandle
  struct CheckHandle {
    std::string newHandle; /// transformed handle if appropriate
    CheckHandleClass handleClass; /// result of handle classification
  };
  class DBsql;
  /// main entry class
  class Manager
  {
    virtual ~Manager() = 0;
   public:
    /// classify input handle according rules 
    virtual CheckHandle checkHandle(const std::string& handle) = 0;
    static Manager *create(DBsql *db);    
  };
}

#endif /*ADMIN_H_*/
